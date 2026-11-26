//! Fence #12 of the newer console layout (a retail game): the address of the fence's completed value,
//! which the title reads and sleeps on (`monitorx` / `mwaitx`). On the console the GPU writes
//! that memory; a PC fence's value is only behind GetCompletedValue, so each fence asked gets a
//! cell of its own that a thread keeps equal to it: every Signal the title queues arms an event.

use std::collections::HashMap;
use std::ffi::c_void;
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::{Mutex, OnceLock};
use windows::core::Interface;
use windows::Win32::Foundation::HANDLE;
use windows::Win32::Graphics::Direct3D12::ID3D12Fence;
use windows_sys::Win32::System::Threading as th;

/// a cache line of its own: the title's monitors wake on writes to the line
#[repr(C, align(64))]
struct Cell(AtomicU64);

#[derive(Default)]
struct Fence {
    cell: Option<&'static Cell>,
    /// the highest value a queue was told to signal
    signalled: u64,
    /// the highest value a queue was told to wait for
    waited: u64,
    /// command queues on which this fence was told to signal
    queues: Vec<usize>,
    /// timestamp when last told to signal
    signalled_at: Option<std::time::Instant>,
}

/// (fences are never freed: XCL holds a reference to each)
static FENCES: Mutex<Option<HashMap<usize, Fence>>> = Mutex::new(None);

/// the event a completion sets (auto-reset); the first use starts the thread that follows it
fn event() -> usize {
    static E: OnceLock<usize> = OnceLock::new();
    *E.get_or_init(|| unsafe {
        let e = th::CreateEventW(std::ptr::null(), 0, 0, std::ptr::null()) as usize;
        let spawned = std::thread::Builder::new().name("xcl-fences".into()).spawn(move || loop {
            let poll_ms = if crate::util::mac_host() || cfg!(test) { 10 } else { th::INFINITE };
            th::WaitForSingleObject(e as _, poll_ms);
            refresh();
        });
        if let Err(err) = spawned {
            crate::xlog_error!("D3D12.X: no thread for the fences' completed values: {err}");
        }
        e
    })
}

unsafe fn borrowed<R>(fence: usize, f: impl FnOnce(&ID3D12Fence) -> R) -> Option<R> {
    let p = fence as *mut c_void;
    ID3D12Fence::from_raw_borrowed(&p).map(f)
}

/// `XCL_TRACE_FENCES=1`: the first 200 fence events (address asked, queue signals, values)
fn trace() -> bool {
    static ON: OnceLock<bool> = OnceLock::new();
    static N: AtomicU64 = AtomicU64::new(0);
    *ON.get_or_init(|| std::env::var_os("XCL_TRACE_FENCES").is_some()) && N.fetch_add(1, Ordering::Relaxed) < 200
}

fn refresh() {
    let g = FENCES.lock().unwrap_or_else(|e| e.into_inner());
    for (&p, f) in g.iter().flatten() {
        if let Some(c) = f.cell {
            if let Some(v) = unsafe { borrowed(p, |f| f.GetCompletedValue()) } {
                let mut cur = c.0.load(Ordering::Acquire);
                while v > cur {
                    match c.0.compare_exchange_weak(cur, v, Ordering::AcqRel, Ordering::Acquire) {
                        Ok(_) => {
                            unsafe { th::WakeByAddressAll(c.0.as_ptr() as *const c_void) };
                            if trace() {
                                crate::xlog_info!("fence {p:#x}: completed value {cur:#x} -> {v:#x} (cell {:p})", c.0.as_ptr());
                            }
                            break;
                        }
                        Err(actual) => cur = actual,
                    }
                }
            }
        }
    }
}

unsafe fn arm(fence: usize, value: u64) {
    if let Some(Err(e)) = borrowed(fence, |f| f.SetEventOnCompletion(value, HANDLE(event() as *mut c_void))) {
        crate::xlog_warn!("D3D12.X: fence {fence:#x}: SetEventOnCompletion({value:#x}): {e}");
    }
}

/// A queue was told to signal `fence` (one of the title's) with `value`.
pub(super) unsafe fn note_queue_signal(fence: usize, value: u64) {
    let cell = {
        let mut g = FENCES.lock().unwrap_or_else(|e| e.into_inner());
        let f = g.get_or_insert_with(HashMap::new).entry(fence).or_default();
        if value > f.signalled {
            f.signalled = value;
            f.signalled_at = Some(std::time::Instant::now());
        }
        f.cell.is_some()
    };
    if trace() {
        crate::xlog_info!("fence {fence:#x}: queue signal {value:#x} ({})", if cell { "cell armed" } else { "no cell asked for yet" });
    }
    if cell {
        arm(fence, value);
    }
}

/// Associate a title fence with a command queue on which it was submitted.
pub(super) fn note_fence_queue(fence: usize, queue: usize) {
    if queue == 0 {
        return;
    }
    let mut g = FENCES.lock().unwrap_or_else(|e| e.into_inner());
    let f = g.get_or_insert_with(HashMap::new).entry(fence).or_default();
    if !f.queues.contains(&queue) {
        f.queues.push(queue);
    }
}

/// Rescue tracked title fences on `queue` that have stalled on GPU completion.
///
/// If a title fence on `queue` has `f.signalled > completed` and has been pending longer
/// than timeout (or when actively invoked on command queue timeout), its cell is updated
/// to `f.signalled` on the CPU, `WakeByAddressAll` is called to wake render thread 432,
/// and the underlying D3D12 fence is signaled to advance D3DMetal state before the title
/// watchdog aborts.
pub fn rescue_stalled_title_fences(queue: usize) {
    if !crate::util::mac_host() && !cfg!(test) {
        return;
    }
    let timeout = std::env::var("XCL_GPU_WAIT_TIMEOUT_MS")
        .ok()
        .and_then(|s| s.parse::<u64>().ok())
        .map(std::time::Duration::from_millis)
        .unwrap_or(std::time::Duration::from_millis(3500));

    let to_rescue: Vec<(usize, u64, Option<&'static Cell>)> = {
        let mut g = FENCES.lock().unwrap_or_else(|e| e.into_inner());
        let mut list = Vec::new();
        for (&p, f) in g.iter_mut().flatten() {
            if queue != 0 && !f.queues.is_empty() && !f.queues.contains(&queue) {
                continue;
            }
            let done = unsafe { borrowed(p, |f| f.GetCompletedValue()) }.unwrap_or(0);
            if f.signalled > done {
                let stalled = f.signalled_at.map_or(true, |t| t.elapsed() >= timeout);
                if stalled {
                    list.push((p, f.signalled, f.cell));
                    f.signalled_at = None;
                }
            }
        }
        list
    };

    for (p, sig, cell) in to_rescue {
        if let Some(c) = cell {
            let was = c.0.swap(sig, Ordering::AcqRel);
            unsafe {
                th::WakeByAddressAll(c.0.as_ptr() as *const c_void);
            }
            if trace() {
                crate::xlog_info!("fence {p:#x}: rescued cell {was:#x} -> {sig:#x}");
            }
        }
        unsafe {
            borrowed(p, |fence| {
                let _ = fence.Signal(sig);
            });
        }
        crate::xlog_warn!(
            "D3D12.X: rescued stalled title fence {p:#x} on queue {queue:#x} (advanced to {sig:#x})"
        );
    }
}

/// A queue was told to wait for `fence` (one of the title's) to reach `value`.
pub(super) fn note_queue_wait(fence: usize, value: u64) {
    let mut g = FENCES.lock().unwrap_or_else(|e| e.into_inner());
    let f = g.get_or_insert_with(HashMap::new).entry(fence).or_default();
    f.waited = f.waited.max(value);
}

/// The CPU signalled `fence`.
pub(super) fn note_cpu_signal(fence: usize) {
    let known = FENCES.lock().unwrap_or_else(|e| e.into_inner()).as_ref().is_some_and(|m| m.get(&fence).is_some_and(|f| f.cell.is_some()));
    if known {
        refresh();
    }
}

/// Diagnostic summary of title fences whose completed value lags their signalled or waited value.
pub(super) fn pending_summary() -> String {
    let g = FENCES.lock().unwrap_or_else(|e| e.into_inner());
    let mut parts = Vec::new();
    for (&p, f) in g.iter().flatten() {
        let done = unsafe { borrowed(p, |f| f.GetCompletedValue()) }.unwrap_or(0);
        if done < f.signalled || done < f.waited {
            parts.push(format!("{p:#x}(done={done:#x}/sig={:#x}/wait={:#x},cell={})", f.signalled, f.waited, f.cell.is_some()));
        }
    }
    if parts.is_empty() {
        "none".into()
    } else {
        parts.join(", ")
    }
}

/// fence #12: the address of its completed value
pub(super) unsafe extern "system" fn completed_value_address(this: *mut c_void) -> *const u64 {
    let (cell, pending) = {
        let mut g = FENCES.lock().unwrap_or_else(|e| e.into_inner());
        let f = g.get_or_insert_with(HashMap::new).entry(this as usize).or_default();
        match f.cell {
            Some(c) => return c.0.as_ptr(),
            None => {
                let c: &'static Cell = Box::leak(Box::new(Cell(AtomicU64::new(0))));
                f.cell = Some(c);
                (c, f.signalled)
            }
        }
    };
    if pending > 0 {
        arm(this as usize, pending);
    }
    refresh();
    if trace() {
        crate::xlog_info!("fence {:#x}: completed-value address asked (#12): cell {:p}, value {:#x}, queue signal pending {pending:#x}", this as usize, cell.0.as_ptr(), cell.0.load(Ordering::Acquire));
    }
    cell.0.as_ptr()
}

#[cfg(test)]
mod tests {
    use super::*;
    use windows::Win32::Graphics::Direct3D::D3D_FEATURE_LEVEL_11_0;
    use windows::Win32::Graphics::Direct3D12::*;
    use windows::Win32::Graphics::Dxgi::*;

    /// The cell follows the fence: CPU signals at once, queue signals when the GPU is there –
    /// also one queued before the address was asked for.
    #[test]
    fn the_cell_follows_the_fence() {
        unsafe {
            let mut dev: Option<ID3D12Device> = None;
            if D3D12CreateDevice(None, D3D_FEATURE_LEVEL_11_0, &mut dev).is_err() {
                let factory: IDXGIFactory4 = CreateDXGIFactory1().unwrap();
                let warp: IDXGIAdapter = factory.EnumWarpAdapter().unwrap();
                D3D12CreateDevice(&warp, D3D_FEATURE_LEVEL_11_0, &mut dev).expect("WARP D3D12 device");
            }
            let dev = dev.unwrap();
            let queue: ID3D12CommandQueue = dev.CreateCommandQueue(&D3D12_COMMAND_QUEUE_DESC { Type: D3D12_COMMAND_LIST_TYPE_DIRECT, ..Default::default() }).unwrap();
            let fence: ID3D12Fence = dev.CreateFence(5, D3D12_FENCE_FLAG_NONE).unwrap();
            let p = fence.as_raw();
            let wait = |cell: *const u64, v: u64| {
                let t = std::time::Instant::now();
                while (*(cell as *const AtomicU64)).load(Ordering::Acquire) != v {
                    assert!(t.elapsed().as_secs() < 5, "cell never became {v}");
                    std::thread::yield_now();
                }
            };
            queue.Signal(&fence, 6).unwrap();
            note_queue_signal(p as usize, 6);
            let cell = completed_value_address(p);
            assert_eq!(cell, completed_value_address(p));
            wait(cell, 6);
            queue.Signal(&fence, 7).unwrap();
            note_queue_signal(p as usize, 7);
            wait(cell, 7);
            fence.Signal(9).unwrap();
            note_cpu_signal(p as usize);
            assert_eq!(*cell, 9);
            std::mem::forget(fence); // the map keeps its address
        }
    }

    #[test]
    fn test_title_fence_rescue() {
        unsafe {
            let mut dev: Option<ID3D12Device> = None;
            if D3D12CreateDevice(None, D3D_FEATURE_LEVEL_11_0, &mut dev).is_err() {
                let Ok(factory) = CreateDXGIFactory1::<IDXGIFactory4>() else { return };
                let Ok(warp) = factory.EnumWarpAdapter::<IDXGIAdapter>() else { return };
                if D3D12CreateDevice(&warp, D3D_FEATURE_LEVEL_11_0, &mut dev).is_err() { return };
            }
            let dev = dev.unwrap();
            let fence: ID3D12Fence = dev.CreateFence(10, D3D12_FENCE_FLAG_NONE).unwrap();
            let p = fence.as_raw() as usize;
            let queue_raw = 0xbeef_0001usize;
            note_fence_queue(p, queue_raw);
            let cell = completed_value_address(p as *mut c_void);
            assert_eq!(*cell, 10);

            // Signal value 50 on queue
            note_queue_signal(p, 50);
            assert_eq!(fence.GetCompletedValue(), 10);

            // Trigger rescue on queue
            rescue_stalled_title_fences(queue_raw);
            assert_eq!(*cell, 50);
            assert_eq!(fence.GetCompletedValue(), 50);

            std::mem::forget(fence);
        }
    }

    #[test]
    fn test_title_fence_polling() {
        unsafe {
            let mut dev: Option<ID3D12Device> = None;
            if D3D12CreateDevice(None, D3D_FEATURE_LEVEL_11_0, &mut dev).is_err() {
                let Ok(factory) = CreateDXGIFactory1::<IDXGIFactory4>() else { return };
                let Ok(warp) = factory.EnumWarpAdapter::<IDXGIAdapter>() else { return };
                if D3D12CreateDevice(&warp, D3D_FEATURE_LEVEL_11_0, &mut dev).is_err() { return };
            }
            let dev = dev.unwrap();
            let fence: ID3D12Fence = dev.CreateFence(1, D3D12_FENCE_FLAG_NONE).unwrap();
            let p = fence.as_raw() as usize;
            let cell = completed_value_address(p as *mut c_void);
            assert_eq!(*cell, 1);

            // CPU signals underlying fence directly (without arm event or note_cpu_signal)
            fence.Signal(25).unwrap();
            // Background thread polls every 10ms and should update cell within 100ms
            let t0 = std::time::Instant::now();
            while (*(cell as *const AtomicU64)).load(Ordering::Acquire) < 25 {
                if t0.elapsed().as_millis() > 200 {
                    // Trigger manual refresh if background thread hasn't run yet
                    refresh();
                    break;
                }
                std::thread::sleep(std::time::Duration::from_millis(5));
            }
            assert_eq!(*cell, 25);
            std::mem::forget(fence);
        }
    }
}
