"""E2E Test Harness: High-fidelity model of GTA VI XCL Mac Native integration subsystems.

Provides opaque-box test environments for R1-R6:
- Console descriptor heaps, write-watch tracking, and dirty slot synchronization (R1)
- Bounded 1-view raw SRV tables and out-of-bounds compute clamp simulation (R2)
- Command list graphics root argument rebinding and texture sampling (R3)
- ID3D12Device9::CreateShaderCacheSession and fallback COM session (R4)
- High-throughput streaming BCPack decompressor with compile-time LUTs and zero-alloc memory queues (R5)
- Mac host gating and verification parity probes (R6)
"""

import os
import struct
import time
from typing import Dict, List, Optional, Tuple, Any

# ==============================================================================
# COM & D3D12 Constants & GUIDs
# ==============================================================================
S_OK: int = 0
S_FALSE: int = 1
E_INVALIDARG: int = -2147024809   # 0x80070057
E_NOINTERFACE: int = -2147467262  # 0x80004002
E_FAIL: int = -2147467259         # 0x80004005
E_NOTIMPL: int = -2147467263       # 0x80004001
DXGI_ERROR_NOT_FOUND: int = -2005270526       # 0x887A0002
DXGI_ERROR_DEVICE_REMOVED: int = -2005270523  # 0x887A0005
STATUS_BREAKPOINT: int = -2147483645          # 0x80000003

IID_IUNKNOWN: str = "00000000-0000-0000-c000-000000000046"
IID_ID3D12DEVICE: str = "189819f1-1db6-4b57-be54-1821339b85f7"
IID_ID3D12DEVICE8: str = "80eab3c6-ee3c-4835-a9e2-bb10e3d93216"
IID_ID3D12DEVICE9: str = "4c80e962-f032-4f60-bc9e-ebc2cfa1d83c"
IID_ID3D12SHADER_CACHE_SESSION: str = "28e2495d-0f64-4ae4-a6ec-129255dc49a8"
IID_ID3D12SHADER_CACHE_SESSION1: str = "1c5789ff-9203-490f-b971-984acf1eb5fe"

SLOT_BYTES: int = 32
SAMPLER_SLOT_BYTES: int = 16
CLEAR_COLOR_SLATE: Tuple[int, int, int, int] = (0x20, 0x20, 0x2C, 0xFF)
TEXTURE_COLOR_SAMPLE: Tuple[int, int, int, int] = (0x00, 0xFF, 0x00, 0xFF)

# ==============================================================================
# Compile-Time LUTs for BCPack (R5)
# ==============================================================================
SPREAD4: List[int] = [
    0x00, 0x01, 0x04, 0x05, 0x10, 0x11, 0x14, 0x15,
    0x40, 0x41, 0x44, 0x45, 0x50, 0x51, 0x54, 0x55,
]

SPREAD8: List[int] = [
    sum(((b >> k & 1) << (2 * k)) for k in range(8))
    for b in range(256)
]

UNSPREAD8: List[int] = [
    sum(((b >> (2 * k) & 1) << k) for k in range(4))
    for b in range(256)
]

def _calc_nibble(b: int) -> Tuple[int, int]:
    half_even = UNSPREAD8[b]
    half_odd = UNSPREAD8[b >> 1]
    def z(u: int) -> int:
        return (u >> 1) ^ (-(u & 1))
    return (z(half_even), z(half_odd))

NIBBLES: List[Tuple[int, int]] = [_calc_nibble(b) for b in range(256)]

def fast_interleave4(a: int, b: int) -> int:
    """Interleave two 4-bit nibbles using SPREAD4."""
    return SPREAD4[a & 0xF] | (SPREAD4[b & 0xF] << 1)

def fast_interleave8(a: int, b: int) -> int:
    """Interleave two 8-bit bytes into 16-bit using SPREAD8."""
    return SPREAD8[a & 0xFF] | (SPREAD8[b & 0xFF] << 1)

def reference_fold_interleave(a: int, b: int) -> int:
    """Reference fold implementation from VIctory for parity verification."""
    def spread(v: int) -> int:
        res = 0
        for k in range(4):
            res |= ((v >> k) & 1) << (2 * k)
        return res
    return spread(a) | (spread(b) << 1)

def reference_fold_nibbles(b: int, odd_first: bool = False) -> List[int]:
    """Reference fold implementation of nibbles for parity verification."""
    def half(v: int) -> int:
        res = 0
        for k in range(4):
            res |= ((v >> (2 * k)) & 1) << k
        return res
    def z(u: int) -> int:
        return (u >> 1) ^ (-(u & 1))
    even = z(half(b))
    odd = z(half(b >> 1))
    return [odd, even] if odd_first else [even, odd]


# ==============================================================================
# Subsystem Model: Console Descriptor Heap & Mirroring (R1)
# ==============================================================================
class DescriptorSlot:
    def __init__(self, raw_bytes: bytes = bytes(32)):
        assert len(raw_bytes) == 32
        self.raw_bytes = bytearray(raw_bytes)

    def is_valid_view(self) -> bool:
        """Inspects slot header words: words[3] >> 28 >= 8 or words[0] != 0."""
        w0 = struct.unpack_from("<I", self.raw_bytes, 0)[0]
        w3 = struct.unpack_from("<I", self.raw_bytes, 12)[0]
        return ((w3 >> 28) >= 8) or (w0 != 0)

    def get_v_sharp_base(self) -> int:
        """Extracts V# base address = dw0 | (dw1[15:0] << 32)."""
        d0 = struct.unpack_from("<I", self.raw_bytes, 0)[0]
        d1 = struct.unpack_from("<I", self.raw_bytes, 4)[0]
        return d0 | ((d1 & 0xFFFF) << 32)


class ConsoleHeap:
    def __init__(self, base_addr: int, num_slots: int, sampler: bool = False):
        self.base_addr = base_addr
        self.num_slots = num_slots
        self.sampler = sampler
        self.slot_size = SAMPLER_SLOT_BYTES if sampler else SLOT_BYTES
        self.total_bytes = num_slots * self.slot_size
        self.memory = bytearray(self.total_bytes)
        self.mirrored_pc_slots: Dict[int, bytes] = {}

    def write_slot(self, slot_idx: int, slot_data: bytes):
        if not (0 <= slot_idx < self.num_slots):
            raise IndexError("Slot index out of range")
        offset = slot_idx * self.slot_size
        self.memory[offset:offset + self.slot_size] = slot_data

    def read_slot(self, slot_idx: int) -> bytes:
        offset = slot_idx * self.slot_size
        return bytes(self.memory[offset:offset + self.slot_size])

    def get_range(self) -> Tuple[int, int]:
        return (self.base_addr, self.total_bytes)


class WriteWatchSystem:
    def __init__(self, page_size: int = 4096):
        self.page_size = page_size
        self.heaps: List[ConsoleHeap] = []
        self.textures_ranges: List[Tuple[int, int]] = []
        self.sparse_ranges: List[Tuple[int, int]] = []
        self.dirty_pages: List[int] = []

    def register_heap(self, heap: ConsoleHeap):
        self.heaps.append(heap)

    def cached_watch_ranges(self) -> List[Tuple[int, int]]:
        """Returns registered address ranges to be tracked by GetWriteWatch."""
        ranges: List[Tuple[int, int]] = []
        ranges.extend(self.textures_ranges)
        ranges.extend(self.sparse_ranges)
        # R1 fix: must include console heap ranges
        for h in self.heaps:
            ranges.append(h.get_range())
        return ranges

    def mark_page_dirty(self, addr: int):
        page_addr = addr & ~(self.page_size - 1)
        if page_addr not in self.dirty_pages:
            self.dirty_pages.append(page_addr)

    def take_written(self) -> List[int]:
        res = list(self.dirty_pages)
        self.dirty_pages.clear()
        return res

    def sync_dirty_heap_slots(self, dirty_pages: List[int]):
        """Inspects dirty pages and mirrors valid descriptor slots into PC Global heap."""
        for h in self.heaps:
            if h.sampler:
                continue
            h_start = h.base_addr
            h_end = h_start + h.total_bytes
            for page in dirty_pages:
                p_end = page + self.page_size
                if p_end <= h_start or page >= h_end:
                    continue
                clamp_start = max(page, h_start)
                clamp_end = min(p_end, h_end)
                k_start = (clamp_start - h_start) // SLOT_BYTES
                k_end = (clamp_end - h_start + SLOT_BYTES - 1) // SLOT_BYTES
                for k in range(k_start, min(k_end, h.num_slots)):
                    slot_bytes = h.read_slot(k)
                    slot = DescriptorSlot(slot_bytes)
                    if slot.is_valid_view():
                        # Republish into mirrored PC heap
                        h.mirrored_pc_slots[k] = slot_bytes


# ==============================================================================
# Subsystem Model: Bounded Root SRVs & Safe Host Views (R2)
# ==============================================================================
class BoundedRawBufferSRV:
    def __init__(self, resource_bytes: int, is_upload: bool = True):
        self.resource_bytes = resource_bytes
        self.is_upload = is_upload
        # Clamp NumElements strictly to host copy / 4
        self.num_elements = resource_bytes // 4
        self.raw_flag = True
        self.format = "DXGI_FORMAT_R32_TYPELESS"
        self.view_dimension = "D3D12_SRV_DIMENSION_BUFFER"

    def load_word(self, buffer_data: bytes, byte_offset: int) -> Tuple[int, bool]:
        """ByteAddressBuffer.Load emulation with hardware clamping.

        Returns (word_value, gpu_fault_occurred).
        When bounded: out-of-bounds reads return 0 without faulting.
        When naked/unbounded: out-of-bounds reads past physical page trigger DXGI_ERROR_DEVICE_REMOVED (0x887A0005).
        """
        bound_limit = self.num_elements * 4
        if byte_offset + 4 > bound_limit:
            # Hardware clamp ensures 0 returned
            return (0, False)
        if byte_offset + 4 <= len(buffer_data):
            val = struct.unpack_from("<I", buffer_data, byte_offset)[0]
            return (val, False)
        return (0, False)


class NakedRootSRV:
    """Pre-fix unconstrained Root SRV triggering fatal GPU fault on OOB access."""
    def __init__(self, resource_bytes: int):
        self.resource_bytes = resource_bytes

    def load_word(self, buffer_data: bytes, byte_offset: int) -> Tuple[int, bool]:
        if byte_offset + 4 > len(buffer_data):
            # Console overshoots buffer by up to 32 MB -> triggers Metal page fault on Mac
            return (0, True) # Fault!
        val = struct.unpack_from("<I", buffer_data, byte_offset)[0]
        return (val, False)


# ==============================================================================
# Subsystem Model: Root Argument Tracking & Sampling for Cutscenes (R3)
# ==============================================================================
class RootParameterKind:
    CONSTANTS = 1
    DESCRIPTOR_TABLE = 2
    ROOT_DESCRIPTOR = 3


class GraphicsRootSignature:
    def __init__(self, params: Optional[List[Dict[str, Any]]] = None):
        self.params = params or []


class CommandListState:
    def __init__(self):
        self.args_gfx: bytearray = bytearray(256)
        self.layout_gfx: Optional[GraphicsRootSignature] = None
        self.rs_gfx: Optional[GraphicsRootSignature] = None
        self.hardware_bound_args: Dict[int, Any] = {}
        self.rendered_pixel_color: Tuple[int, int, int, int] = CLEAR_COLOR_SLATE

    def set_graphics_root_signature(self, rs: GraphicsRootSignature):
        self.rs_gfx = rs
        self.layout_gfx = rs
        # Hardware resets all bound root parameters per D3D12 spec!
        self.hardware_bound_args.clear()

    def set_graphics_root_32bit_constant(self, root_index: int, value: int, offset: int = 0):
        at = root_index * 16 + offset * 4
        struct.pack_into("<I", self.args_gfx, at, value)

    def set_graphics_root_descriptor_table(self, root_index: int, console_gpu_addr: int):
        at = root_index * 16
        struct.pack_into("<Q", self.args_gfx, at, console_gpu_addr)

    def resolve_and_bind_graphics_root_arguments(self, heap: ConsoleHeap):
        """Rebinds root constants and descriptor tables after root signature resets."""
        if not self.layout_gfx:
            return
        for i, param in enumerate(self.layout_gfx.params):
            kind = param.get("type")
            at = i * 16
            if kind == RootParameterKind.CONSTANTS:
                val = struct.unpack_from("<I", self.args_gfx, at)[0]
                self.hardware_bound_args[i] = ("CONSTANTS", val)
            elif kind == RootParameterKind.DESCRIPTOR_TABLE:
                table_addr = struct.unpack_from("<Q", self.args_gfx, at)[0]
                if table_addr != 0:
                    slot_idx = (table_addr - heap.base_addr) // SLOT_BYTES
                    if slot_idx in heap.mirrored_pc_slots:
                        # Successfully bound PC descriptor table handle
                        self.hardware_bound_args[i] = ("TABLE", slot_idx, heap.mirrored_pc_slots[slot_idx])
                    else:
                        self.hardware_bound_args[i] = ("UNBOUND_NULL", None)
            elif kind == RootParameterKind.ROOT_DESCRIPTOR:
                va = struct.unpack_from("<Q", self.args_gfx, at)[0]
                self.hardware_bound_args[i] = ("UAV", va)

    def draw_fullscreen_triangle(self):
        """Simulates cutscene intro full-screen triangle rasterization."""
        # Find if a texture descriptor table is bound
        has_texture = False
        for binding in self.hardware_bound_args.values():
            if isinstance(binding, tuple) and binding[0] == "TABLE":
                has_texture = True
                break
        if has_texture:
            self.rendered_pixel_color = TEXTURE_COLOR_SAMPLE
        else:
            # Unbound / zero descriptor results in untouched slate clear color
            self.rendered_pixel_color = CLEAR_COLOR_SLATE


# ==============================================================================
# Subsystem Model: D3DMetal Shader Cache Session Guard (R4)
# ==============================================================================
class ShaderCacheSessionDesc:
    def __init__(self, guid_str: str = "00000000-0000-0000-0000-000000000000",
                 mode: int = 0, flags: int = 0,
                 max_mem_bytes: int = 0, max_mem_entries: int = 0,
                 max_file_bytes: int = 0, version: int = 1):
        self.guid_str = guid_str
        self.mode = mode
        self.flags = flags
        self.max_mem_bytes = max_mem_bytes
        self.max_mem_entries = max_mem_entries
        self.max_file_bytes = max_file_bytes
        self.version = version

    def pack(self) -> bytes:
        """Packs into standard 48-byte D3D12_SHADER_CACHE_SESSION_DESC."""
        guid_bytes = bytes.fromhex(self.guid_str.replace("-", ""))
        return struct.pack("<16sIIIIIIQ", guid_bytes, self.mode, self.flags,
                           self.max_mem_bytes, self.max_mem_entries, self.max_file_bytes,
                           0, self.version)


class FallbackShaderCacheSession:
    def __init__(self, desc: Optional[ShaderCacheSessionDesc] = None):
        self.ref_count = 1
        self.desc = desc or ShaderCacheSessionDesc()
        self.store: Dict[bytes, bytes] = {}

    def query_interface(self, riid: str) -> Tuple[int, Optional['FallbackShaderCacheSession']]:
        if riid in (IID_IUNKNOWN, IID_ID3D12SHADER_CACHE_SESSION, IID_ID3D12SHADER_CACHE_SESSION1):
            self.add_ref()
            return (S_OK, self)
        return (E_NOINTERFACE, None)

    def add_ref(self) -> int:
        self.ref_count += 1
        return self.ref_count

    def release(self) -> int:
        self.ref_count -= 1
        return self.ref_count

    def find_value(self, key: bytes) -> Tuple[int, Optional[bytes]]:
        """Returns DXGI_ERROR_NOT_FOUND (clean cache miss) allowing normal shader translation."""
        if key in self.store:
            return (S_OK, self.store[key])
        return (DXGI_ERROR_NOT_FOUND, None)

    def store_value(self, key: bytes, value: bytes) -> int:
        self.store[key] = value
        return S_OK

    def get_desc(self) -> ShaderCacheSessionDesc:
        return self.desc


class D3D12DeviceMock:
    def __init__(self, is_mac_host: bool = True):
        self.is_mac_host = is_mac_host
        self.ref_count = 1

    def query_interface(self, riid: str) -> Tuple[int, Any]:
        if riid in (IID_IUNKNOWN, IID_ID3D12DEVICE, IID_ID3D12DEVICE8, IID_ID3D12DEVICE9):
            self.ref_count += 1
            return (S_OK, self)
        return (E_NOINTERFACE, None)

    def create_shader_cache_session(self, desc: Optional[ShaderCacheSessionDesc]) -> Tuple[int, Optional[FallbackShaderCacheSession]]:
        """Slot 73 interceptor."""
        if desc is None:
            return (E_INVALIDARG, None)
        if self.is_mac_host:
            # Under Wine/D3DMetal: return clean fallback session
            session = FallbackShaderCacheSession(desc)
            return (S_OK, session)
        else:
            # Native Windows: native driver session
            session = FallbackShaderCacheSession(desc)
            return (S_OK, session)

    def shader_cache_control(self, kinds: int, control: int) -> int:
        """Slot 74 interceptor."""
        return S_OK


# ==============================================================================
# Subsystem Model: High-Throughput Streaming BCPack Decompression (R5)
# ==============================================================================
class BCPackStreamDecompressor:
    """Streaming decompressor using stack records, const-generic lanes, and LUTs."""
    def __init__(self):
        self.heap_allocation_count: int = 0

    def decode_stream_simulated(self, mode: int, source_bytes: bytes, dst_buffer: bytearray) -> int:
        """Zero-allocation streaming decoding of BCPack payload."""
        if mode == 2:
            return E_FAIL # BC2 explicitly unsupported
        if mode not in (1, 3, 4, 5, 6, 7):
            return E_FAIL
        if len(source_bytes) == 0:
            return E_FAIL
        block_bytes = 8 if mode in (1, 4) else 16
        expected_blocks = len(dst_buffer) // block_bytes
        if expected_blocks == 0:
            return E_INVALIDARG

        # Simulated stack-allocated record buffer: exactly 32 bytes
        stack_record = bytearray(32)
        # Simulated stack history: fixed array of 256 entries
        stack_history = [0] * 256

        # Step through stream block by block without per-block heap allocation
        src_pos = 0
        for block_idx in range(expected_blocks):
            if src_pos >= len(source_bytes):
                break
            # Use LUTs to interleave symbols into stack_record
            sym = source_bytes[src_pos % len(source_bytes)]
            interleaved = fast_interleave4(sym & 0xF, (sym >> 4) & 0xF)
            stack_record[0] = interleaved
            stack_history[block_idx % 256] = interleaved
            src_pos += 1

            # Direct copy to destination without intermediate buffers
            dst_off = block_idx * block_bytes
            for b in range(block_bytes):
                dst_buffer[dst_off + b] = (interleaved + b) & 0xFF

        return S_OK


# ==============================================================================
# Subsystem Model: Mac Host Gating & Verification Parity (R6)
# ==============================================================================
class PlatformUtil:
    @staticmethod
    def mac_host() -> bool:
        override = os.environ.get("XCL_MAC_HOST", "").strip()
        if override == "1":
            return True
        if override == "0":
            return False
        import platform
        return platform.system() == "Darwin"

    @staticmethod
    def rosetta(cpu_brand: str = "") -> bool:
        if "VirtualApple" in cpu_brand or "Apple" in cpu_brand:
            return True
        return False

    @staticmethod
    def check_pointer_alignment(ptr: int) -> bool:
        """align_of::<usize>() - 1 check in is_adopted."""
        align_mask = 7 # 8-byte pointer alignment for 64-bit usize
        return (ptr & align_mask) == 0
