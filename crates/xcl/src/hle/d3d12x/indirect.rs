//! Repack root-changing indirect draws and dispatches on the GPU. The native signature
//! consumes an ABI io UAV address followed by the draw or dispatch arguments for each item.
use std::collections::HashMap;
use std::ffi::c_void;
use std::mem::ManuallyDrop;
use std::sync::{Arc, Mutex};

use windows::core::Interface;
use windows::Win32::Graphics::Direct3D12::*;

use super::{console_list::{Layout, Param}, real};
use crate::gpu::recomp::{self, KERNEL_SGPRS};

const IO_BYTES: u32 = 4 * KERNEL_SGPRS as u32;
const THREADS: u32 = 64;
const CONFIG_WORDS: u32 = 13;

#[derive(Clone, Copy)]
enum RootArg {
    Constants { index: usize, offset: u32, words: u32 },
    Descriptor { index: usize },
}

pub(super) struct Signature {
    stride: u32,
    root: Vec<(u32, RootArg)>,
    action: D3D12_INDIRECT_ARGUMENT_TYPE,
    action_offset: u32,
    action_bytes: u32,
}

// An entry without a layout still identifies a root-changing signature. Its original
// signature can only be run with the title's root signature.
static SIGNATURES: Mutex<Option<HashMap<usize, Option<Arc<Signature>>>>> = Mutex::new(None);

pub(super) fn register(sig: usize, stride: u32, args: &[[u32; 4]]) {
    SIGNATURES.lock().unwrap().get_or_insert_with(HashMap::new)
        .insert(sig, Signature::parse(stride, args).map(Arc::new));
}

pub(super) fn lookup(sig: usize) -> Option<Option<Arc<Signature>>> {
    SIGNATURES.lock().unwrap().as_ref()?.get(&sig).cloned()
}

pub(super) fn forget(sig: usize) {
    if let Some(sigs) = SIGNATURES.lock().unwrap().as_mut() {
        sigs.remove(&sig);
    }
}

impl Signature {
    fn parse(stride: u32, args: &[[u32; 4]]) -> Option<Self> {
        let ([action, ..], root_args) = args.split_last()?;
        let action = D3D12_INDIRECT_ARGUMENT_TYPE(*action as i32);
        let action_bytes = match action {
            D3D12_INDIRECT_ARGUMENT_TYPE_DRAW => 16,
            D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED => 20,
            D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH => 12,
            _ => return None,
        };
        let mut root = Vec::with_capacity(root_args.len());
        let mut at = 0u32;
        for &[kind, index, offset, words] in root_args {
            let index = index as usize;
            let (arg, bytes) = match D3D12_INDIRECT_ARGUMENT_TYPE(kind as i32) {
                D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT if words != 0 => (RootArg::Constants { index, offset, words }, words.checked_mul(4)?),
                D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW
                | D3D12_INDIRECT_ARGUMENT_TYPE_SHADER_RESOURCE_VIEW
                | D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW => (RootArg::Descriptor { index }, 8),
                _ => return None,
            };
            root.push((at, arg));
            at = at.checked_add(bytes)?;
        }
        if root.is_empty() || stride % 4 != 0 || at.checked_add(action_bytes)? > stride {
            return None;
        }
        Some(Self { stride, root, action, action_offset: at, action_bytes })
    }

    fn patches(&self, layout: &Layout, inline: u32, gfx: bool) -> Option<Vec<Patch>> {
        let mut result = Vec::new();
        for &(source, arg) in &self.root {
            let (at, bytes) = match arg {
                RootArg::Constants { index, offset, words } => {
                    let Param::Constants { at, count } = *layout.params.get(index)? else { return None };
                    if offset.checked_add(words)? > count {
                        return None;
                    }
                    (at.checked_add(offset.checked_mul(4)?)?, words.checked_mul(4)?)
                }
                RootArg::Descriptor { index } => {
                    let Param::Descriptor { at } = *layout.params.get(index)? else { return None };
                    (at, 8)
                }
            };
            let end = at.checked_add(bytes)?;
            if end > layout.size || (at < inline && end > inline) {
                return None;
            }
            let dest = if at < inline { at + if gfx { 32 } else { 16 } } else { IO_BYTES + (at - inline) };
            for off in (0..bytes).step_by(4) {
                result.push(Patch { source: source + off, dest: dest + off, shadow: (at + off) as usize });
            }
        }
        Some(result)
    }
}

#[derive(Debug, PartialEq, Eq)]
struct Patch {
    source: u32,
    dest: u32,
    shadow: usize,
}

struct Repacker {
    root: ID3D12RootSignature,
    pso: ID3D12PipelineState,
}

type SignatureKey = (usize, i32);
type RepackerKey = (usize, bool);
static REPACKERS: Mutex<Option<HashMap<RepackerKey, Arc<Repacker>>>> = Mutex::new(None);
static PACKED_SIGNATURES: Mutex<Option<HashMap<SignatureKey, ID3D12CommandSignature>>> = Mutex::new(None);
static PENDING: Mutex<Option<HashMap<usize, Vec<ID3D12Resource>>>> = Mutex::new(None);

pub(super) fn retain_resource(allocator: usize, resource: ID3D12Resource) {
    PENDING.lock().unwrap().get_or_insert_with(HashMap::new).entry(allocator).or_default().push(resource);
}

pub(super) fn allocator_reset(allocator: usize) {
    let buffers = PENDING.lock().unwrap().as_mut().and_then(|m| m.remove(&allocator));
    drop(buffers);
}

unsafe fn repacker(dev: &ID3D12Device, upload: bool) -> Result<Arc<Repacker>, String> {
    let mut g = REPACKERS.lock().unwrap();
    let map = g.get_or_insert_with(HashMap::new);
    let key = (dev.as_raw() as usize, upload);
    if let Some(p) = map.get(&key) {
        return Ok(p.clone());
    }
    let src = include_str!("indirect_repack.hlsl");
    let extra: &[&str] = if upload { &["-DINPUT_UPLOAD=1"] } else { &[] };
    let dxil = recomp::dxc::compile(src, "main", "cs_6_0", extra)?;
    let descriptor = |kind, reg| D3D12_ROOT_PARAMETER {
        ParameterType: kind,
        Anonymous: D3D12_ROOT_PARAMETER_0 { Descriptor: D3D12_ROOT_DESCRIPTOR { ShaderRegister: reg, RegisterSpace: 0 } },
        ShaderVisibility: D3D12_SHADER_VISIBILITY_ALL,
    };
    let params = [
        if upload { descriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, 0) } else { descriptor(D3D12_ROOT_PARAMETER_TYPE_UAV, 0) },
        descriptor(D3D12_ROOT_PARAMETER_TYPE_UAV, 1),
        descriptor(D3D12_ROOT_PARAMETER_TYPE_UAV, 2),
        D3D12_ROOT_PARAMETER {
            ParameterType: D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS,
            Anonymous: D3D12_ROOT_PARAMETER_0 { Constants: D3D12_ROOT_CONSTANTS { ShaderRegister: 0, RegisterSpace: 0, Num32BitValues: CONFIG_WORDS } },
            ShaderVisibility: D3D12_SHADER_VISIBILITY_ALL,
        },
    ];
    let desc = D3D12_ROOT_SIGNATURE_DESC { NumParameters: params.len() as u32, pParameters: params.as_ptr(), ..Default::default() };
    let mut blob = None;
    D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &mut blob, None).map_err(|e| format!("serialize the indirect repacker root signature: {e}"))?;
    let blob = blob.ok_or("no indirect repacker root signature blob")?;
    let root: ID3D12RootSignature = dev.CreateRootSignature(0, std::slice::from_raw_parts(blob.GetBufferPointer().cast(), blob.GetBufferSize()))
        .map_err(|e| format!("create the indirect repacker root signature: {e}"))?;
    let pso = compute_pso(dev, &root, &dxil).map_err(|e| format!("create the indirect repacker PSO: {e}"))?;
    let p = Arc::new(Repacker { root, pso });
    map.insert(key, p.clone());
    Ok(p)
}

unsafe fn compute_pso(dev: &ID3D12Device, root: &ID3D12RootSignature, dxil: &[u8]) -> windows::core::Result<ID3D12PipelineState> {
    let mut desc = D3D12_COMPUTE_PIPELINE_STATE_DESC {
        pRootSignature: ManuallyDrop::new(Some(root.clone())),
        CS: D3D12_SHADER_BYTECODE { pShaderBytecode: dxil.as_ptr().cast(), BytecodeLength: dxil.len() },
        ..Default::default()
    };
    let pso = dev.CreateComputePipelineState(&desc);
    ManuallyDrop::drop(&mut desc.pRootSignature);
    pso
}

unsafe fn packed_signature(dev: &ID3D12Device, root: &ID3D12RootSignature, sig: &Signature) -> Result<ID3D12CommandSignature, String> {
    let mut g = PACKED_SIGNATURES.lock().unwrap();
    let map = g.get_or_insert_with(HashMap::new);
    let key = (root.as_raw() as usize, sig.action.0);
    if let Some(signature) = map.get(&key) {
        return Ok(signature.clone());
    }
    let params = [
        D3D12_INDIRECT_ARGUMENT_DESC {
            Type: D3D12_INDIRECT_ARGUMENT_TYPE_UNORDERED_ACCESS_VIEW,
            Anonymous: D3D12_INDIRECT_ARGUMENT_DESC_0 { UnorderedAccessView: D3D12_INDIRECT_ARGUMENT_DESC_0_4 { RootParameterIndex: 0 } },
        },
        D3D12_INDIRECT_ARGUMENT_DESC { Type: sig.action, ..Default::default() },
    ];
    let desc = D3D12_COMMAND_SIGNATURE_DESC { ByteStride: 8 + sig.action_bytes, NumArgumentDescs: 2, pArgumentDescs: params.as_ptr(), NodeMask: 0 };
    let mut out = None;
    dev.CreateCommandSignature(&desc, root, &mut out).map_err(|e| format!("create the packed indirect signature: {e}"))?;
    let out: ID3D12CommandSignature = out.ok_or("no packed indirect signature")?;
    map.insert(key, out.clone());
    Ok(out)
}

/// Read a title's GPU-produced buffer through its UMA window, or read a native upload
/// buffer as an SRV (already in GENERIC_READ). The console offset excludes placement delta.
pub(super) unsafe fn source(resource: usize, offset: u64, bytes: u64) -> Option<(bool, u64, u32)> {
    let end = offset.checked_add(bytes)?;
    if bytes == 0 || offset % 4 != 0 {
        return None;
    }
    if let Some((p, len)) = real::console_buffer(resource) {
        if end > len as u64 {
            return None;
        }
        let (window, at) = real::uma_window_buffer(p.checked_add(usize::try_from(offset).ok()?)?, usize::try_from(bytes).ok()?)?;
        u32::try_from(at.checked_add(bytes - 1)?).ok()?;
        let window = window as *mut c_void;
        let window = ID3D12Resource::from_raw_borrowed(&window)?;
        let va = window.GetGPUVirtualAddress();
        return (va != 0).then_some((false, va, at as u32));
    }
    let raw = resource as *mut c_void;
    let r = ID3D12Resource::from_raw_borrowed(&raw)?;
    let mut props = D3D12_HEAP_PROPERTIES::default();
    r.GetHeapProperties(Some(&mut props), None).ok()?;
    let d = r.GetDesc();
    if d.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER
        || d.Flags.contains(D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)
        || !matches!(props.Type, D3D12_HEAP_TYPE_UPLOAD | D3D12_HEAP_TYPE_GPU_UPLOAD)
        || end > d.Width
    {
        return None;
    }
    // Root SRV addresses must be 16-byte aligned. Rebase to keep byte offsets in 32 bits.
    u32::try_from((offset & 15).checked_add(bytes - 1)?).ok()?;
    let va = r.GetGPUVirtualAddress();
    (va != 0).then_some((true, va.checked_add(offset & !15)?, (offset & 15) as u32))
}

unsafe fn transition(list: &ID3D12GraphicsCommandList, resource: &ID3D12Resource, before: D3D12_RESOURCE_STATES, after: D3D12_RESOURCE_STATES) {
    let barrier = D3D12_RESOURCE_BARRIER {
        Type: D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        Anonymous: D3D12_RESOURCE_BARRIER_0 { Transition: ManuallyDrop::new(D3D12_RESOURCE_TRANSITION_BARRIER {
            pResource: ManuallyDrop::new(Some(resource.clone())),
            Subresource: D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            StateBefore: before,
            StateAfter: after,
        }) },
        ..Default::default()
    };
    list.ResourceBarrier(std::slice::from_ref(&barrier));
    let mut t = ManuallyDrop::into_inner(barrier.Anonymous.Transition);
    ManuallyDrop::drop(&mut t.pResource);
}

pub(super) struct Recorded {
    pub(super) signature: ID3D12CommandSignature,
    pub(super) arguments: ID3D12Resource,
    patches: Vec<Patch>,
}

fn indirect_rest() -> bool {
    std::env::var("VICTORY_INDIRECT_REST")
        .or_else(|_| std::env::var("XCL_INDIRECT_REST"))
        .is_ok_and(|v| v != "0" && !v.eq_ignore_ascii_case("false") && (v == "1" || v.eq_ignore_ascii_case("true")))
}

impl Recorded {
    pub(super) unsafe fn finish_gpu(&self, list: &ID3D12GraphicsCommandList) {
        // A list can execute more than once. Leave the scratch resource in its creation state.
        transition(list, &self.arguments, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    }

    pub(super) fn retain_and_clear(self, allocator: usize, shadow: &mut [u8]) {
        retain_resource(allocator, self.arguments);
        if indirect_rest() {
            for p in self.patches {
                if let Some(chunk) = shadow.get_mut(p.shadow..p.shadow + 4) {
                    chunk.fill(0);
                }
            }
        }
    }
}

pub(super) struct Repack<'a> {
    pub(super) signature: &'a Signature,
    pub(super) resource: usize,
    pub(super) offset: u64,
    pub(super) max: u32,
    pub(super) gfx: bool,
}

impl Repack<'_> {
    pub(super) unsafe fn record(
        &self, list: &ID3D12GraphicsCommandList, root: &ID3D12RootSignature, layout: &Layout,
        inline: usize, args: &[u8], io: &[u32], alloc: impl FnOnce(usize) -> Option<usize>,
    ) -> Result<Recorded, String> {
        let sig = self.signature;
        if self.gfx == (sig.action == D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH) {
            return Err("indirect action does not match the translated pipeline".into());
        }
        let groups = self.max.div_ceil(THREADS);
        if groups == 0 || groups > D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION {
            return Err("indirect command count is outside the repacker's dispatch range".into());
        }
        let patches = sig.patches(layout, inline as u32, self.gfx).ok_or("indirect root arguments do not match the bound layout")?;
        let (upload, input, input_offset) = source(self.resource, self.offset, u64::from(self.max) * u64::from(sig.stride))
            .ok_or("indirect arguments are outside a GPU-readable UMA or upload buffer")?;
        let spill = &args[inline..];
        let template_bytes = IO_BYTES + spill.len() as u32;
        let scratch_stride = (template_bytes + 15) & !15;
        let template_at = scratch_stride.checked_mul(self.max).ok_or("indirect root data is too large")?;
        let patch_at = template_at.checked_add(template_bytes).ok_or("indirect root data is too large")?;
        let size = patch_at.checked_add(8 * patches.len() as u32).ok_or("indirect root data is too large")?;
        let scratch = alloc(size as usize).ok_or("unified memory is full")?;
        let scratch_va = real::uma_va_of(scratch).ok_or("indirect scratch is outside GPU-visible UMA")?;
        let raw = scratch as *mut u8;
        std::ptr::copy_nonoverlapping(io.as_ptr().cast(), raw.add(template_at as usize), IO_BYTES as usize);
        std::ptr::copy_nonoverlapping(spill.as_ptr(), raw.add(template_at as usize + IO_BYTES as usize), spill.len());
        for (i, patch) in patches.iter().enumerate() {
            raw.add(patch_at as usize + 8 * i).cast::<[u32; 2]>().write([patch.source, patch.dest]);
        }
        let mut dev = None;
        list.GetDevice(&mut dev).map_err(|e| format!("get the indirect list device: {e}"))?;
        let dev = dev.ok_or("no indirect list device")?;
        let repack = repacker(&dev, upload)?;
        let signature = packed_signature(&dev, root, sig)?;
        let mut arguments = None;
        dev.CreateCommittedResource(
            &D3D12_HEAP_PROPERTIES { Type: D3D12_HEAP_TYPE_DEFAULT, ..Default::default() }, D3D12_HEAP_FLAG_NONE,
            &D3D12_RESOURCE_DESC {
                Dimension: D3D12_RESOURCE_DIMENSION_BUFFER, Width: u64::from(self.max) * u64::from(8 + sig.action_bytes),
                Height: 1, DepthOrArraySize: 1, MipLevels: 1,
                SampleDesc: windows::Win32::Graphics::Dxgi::Common::DXGI_SAMPLE_DESC { Count: 1, Quality: 0 },
                Layout: D3D12_TEXTURE_LAYOUT_ROW_MAJOR, Flags: D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, ..Default::default()
            },
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS, None, &mut arguments,
        ).map_err(|e| format!("allocate packed indirect arguments: {e}"))?;
        let arguments: ID3D12Resource = arguments.ok_or("no packed indirect argument buffer")?;
        let config: [u32; CONFIG_WORDS as usize] = [
            self.max, input_offset, sig.stride, sig.action_offset, sig.action_bytes, scratch_stride,
            template_bytes, patches.len() as u32, scratch as u32, (scratch >> 32) as u32,
            scratch_va as u32, (scratch_va >> 32) as u32, IO_BYTES,
        ];
        repack.dispatch(list, upload, input, scratch_va, &arguments, &config, groups);
        Ok(Recorded { signature, arguments, patches })
    }
}

impl Repacker {
    unsafe fn dispatch(
        &self, list: &ID3D12GraphicsCommandList, upload: bool, input: u64, scratch_va: u64,
        arguments: &ID3D12Resource, config: &[u32; CONFIG_WORDS as usize], groups: u32,
    ) {
        let uav = D3D12_RESOURCE_BARRIER { Type: D3D12_RESOURCE_BARRIER_TYPE_UAV, ..Default::default() };
        list.ResourceBarrier(&[uav.clone()]);
        list.SetPipelineState(&self.pso);
        list.SetComputeRootSignature(&self.root);
        if upload {
            list.SetComputeRootShaderResourceView(0, input);
        } else {
            list.SetComputeRootUnorderedAccessView(0, input);
        }
        list.SetComputeRootUnorderedAccessView(1, scratch_va);
        list.SetComputeRootUnorderedAccessView(2, arguments.GetGPUVirtualAddress());
        list.SetComputeRoot32BitConstants(3, config.len() as u32, config.as_ptr().cast(), 0);
        list.Dispatch(groups, 1, 1);
        list.ResourceBarrier(&[uav]);
        transition(list, arguments, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn root_offsets_follow_inline_registers_and_the_spilled_table() {
        let key = 0x217_1A00;
        let desc = [[5, 0, 1, 2], [8, 1, 0, 0], [1, 0, 0, 0]];
        register(key, 40, &desc);
        let sig = lookup(key).unwrap().unwrap();
        assert_eq!((sig.stride, sig.action_offset, sig.action_bytes), (40, 16, 20));
        let layout = Layout { params: vec![Param::Constants { at: 0, count: 3 }, Param::Descriptor { at: 12 }], size: 20, inline: 12, inline_gfx: 12, inline_ms: 12 };
        let patches = sig.patches(&layout, 12, true).unwrap();
        assert_eq!(
            patches.iter().map(|p| (p.source, p.dest, p.shadow)).collect::<Vec<_>>(),
            vec![(0, 36, 4), (4, 40, 8), (8, IO_BYTES, 12), (12, IO_BYTES + 4, 16)],
        );
        assert_eq!(sig.patches(&layout, 12, false).unwrap()[0].dest, 20);
        assert!(sig.patches(&layout, 8, true).is_none());
        assert!(Signature::parse(32, &desc).is_none());
        assert!(Signature::parse(40, &[[5, 0, 0, 1], [3, 0, 0, 0], [1, 0, 0, 0]]).is_none());
        forget(key);
        assert!(lookup(key).is_none());
    }

    #[test]
    fn gpu_repack_preserves_per_dispatch_values_and_the_count_buffer() {
        use windows::Win32::Foundation::{CloseHandle, WAIT_OBJECT_0};
        use windows::Win32::Graphics::Direct3D::D3D_FEATURE_LEVEL_12_0;
        use windows::Win32::System::Threading::{CreateEventW, WaitForSingleObject};

        unsafe {
            let mut dev: Option<ID3D12Device> = None;
            if D3D12CreateDevice(None, D3D_FEATURE_LEVEL_12_0, &mut dev).is_err() {
                crate::gpu::recomp::runner::skip_test("no GPU");
                return;
            }
            if recomp::dxc::create_instance().is_none() {
                crate::gpu::recomp::runner::skip_test("no DXC");
                return;
            }
            let dev = dev.unwrap();
            let queue: ID3D12CommandQueue = dev.CreateCommandQueue(&D3D12_COMMAND_QUEUE_DESC::default()).unwrap();
            let allocator: ID3D12CommandAllocator = dev.CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT).unwrap();
            let list: ID3D12GraphicsCommandList = dev.CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, &allocator, None).unwrap();
            let buffer = |bytes, heap, state, flags| -> ID3D12Resource {
                let desc = D3D12_RESOURCE_DESC {
                    Dimension: D3D12_RESOURCE_DIMENSION_BUFFER, Width: bytes, Height: 1, DepthOrArraySize: 1, MipLevels: 1,
                    SampleDesc: windows::Win32::Graphics::Dxgi::Common::DXGI_SAMPLE_DESC { Count: 1, Quality: 0 },
                    Layout: D3D12_TEXTURE_LAYOUT_ROW_MAJOR, Flags: flags, ..Default::default()
                };
                let mut out = None;
                dev.CreateCommittedResource(&D3D12_HEAP_PROPERTIES { Type: heap, ..Default::default() },
                    D3D12_HEAP_FLAG_NONE, &desc, state, None, &mut out).unwrap();
                out.unwrap()
            };
            let gpu_buffer = |size, state| buffer(size, D3D12_HEAP_TYPE_DEFAULT, state, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
            let (count, stride, record_bytes, max) = (2u32, 20u32, 20u32, 3u32);
            let template_at = max * IO_BYTES;
            let scratch_bytes = template_at + IO_BYTES + 16;
            let src = gpu_buffer(u64::from(max * stride), D3D12_RESOURCE_STATE_COPY_DEST);
            let scratch = gpu_buffer(u64::from(scratch_bytes), D3D12_RESOURCE_STATE_COPY_DEST);
            let packed = gpu_buffer(u64::from(max * record_bytes), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            let results = gpu_buffer(16, D3D12_RESOURCE_STATE_COPY_DEST);
            let readback = buffer(16, D3D12_HEAP_TYPE_READBACK, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_FLAG_NONE);
            let upload = buffer(u64::from(scratch_bytes) + 128, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_FLAG_NONE);
            let mut data = vec![0u32; (scratch_bytes as usize + 128) / 4];
            data[..15].copy_from_slice(&[0, 4, 1, 1, 1, 1, 9, 1, 1, 1, 2, 50, 1, 1, 1]);
            data[16] = count;
            let t = (128 + template_at as usize) / 4;
            data[t + 4] = 7;
            data[t + KERNEL_SGPRS..t + KERNEL_SGPRS + 4].copy_from_slice(&[0, 20, 4, 24]);

            list.CopyBufferRegion(&src, 0, &upload, 0, u64::from(max * stride));
            list.CopyBufferRegion(&scratch, 0, &upload, 128, u64::from(scratch_bytes));
            list.CopyBufferRegion(&results, 0, &upload, 80, 16);
            for resource in [&src, &scratch, &results] {
                transition(&list, resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
            }

            let prep = repacker(&dev, false).unwrap();
            let sig = Signature::parse(stride, &[[5, 0, 1, 2], [2, 0, 0, 0]]).unwrap();
            let native_sig = packed_signature(&dev, &prep.root, &sig).unwrap();
            let work = recomp::dxc::compile(
                "RWByteAddressBuffer io : register(u0); RWByteAddressBuffer result : register(u1);
                 [numthreads(1,1,1)] void main() { result.Store(io.Load(20) * 4, io.Load(24) + io.Load(16)); }",
                "main", "cs_6_0", &[],
            ).unwrap();
            let work = compute_pso(&dev, &prep.root, &work).unwrap();
            let va = scratch.GetGPUVirtualAddress();
            let config = [max, 0, stride, 8, 12, IO_BYTES, IO_BYTES, 2, 0, 0, va as u32, (va >> 32) as u32, IO_BYTES];
            prep.dispatch(&list, false, src.GetGPUVirtualAddress(), va, &packed, &config, 1);
            list.SetPipelineState(&work);
            list.SetComputeRootSignature(&prep.root);
            list.SetComputeRootUnorderedAccessView(1, results.GetGPUVirtualAddress());
            list.ExecuteIndirect(&native_sig, max, &packed, 0, &upload, 64);
            transition(&list, &results, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
            list.CopyBufferRegion(&readback, 0, &results, 0, 16);
            list.Close().unwrap();
            // Contents written after recording. The repacker can only see them at GPU execution.
            let mut mapped: *mut c_void = std::ptr::null_mut();
            upload.Map(0, None, Some(&mut mapped)).unwrap();
            std::ptr::copy_nonoverlapping(data.as_ptr(), mapped.cast(), data.len());
            upload.Unmap(0, None);
            let fence: ID3D12Fence = dev.CreateFence(0, D3D12_FENCE_FLAG_NONE).unwrap();
            let event = CreateEventW(None, false, false, None).unwrap();
            queue.ExecuteCommandLists(&[Some(list.cast().unwrap())]);
            queue.Signal(&fence, 1).unwrap();
            fence.SetEventOnCompletion(1, event).unwrap();
            let waited = WaitForSingleObject(event, 10_000);
            CloseHandle(event).unwrap();
            assert_eq!(waited, WAIT_OBJECT_0);
            readback.Map(0, None, Some(&mut mapped)).unwrap();
            let actual = mapped.cast::<[u32; 4]>().read();
            readback.Unmap(0, None);
            assert_eq!(actual, [11, 16, 0, 0]);
        }
    }
}
