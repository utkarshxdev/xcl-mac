# Project: GTA VI XCL Mac Native Integration — Stage 6.4

## Architecture
Stage 6.4 resolves the Fence 3766 GPU command queue deadlock, throttles texture streaming to prevent D3DMetal device removal (0x887A0005), transitions geometry from loading screens into 3D world rendering ("Vice City, 2 years earlier"), establishes interactive player movement, and enforces 100% subsystem verification parity on Apple Silicon.

Key Subsystems:
- **Command Queue Synchronization & Bottom-of-Pipe Rescue** (`crates/xcl/src/hle/d3d12x/`):
  - `cpu_writes.rs`: Bottom-of-pipe memory write execution, active queue stall watchdog rescue, reduced timeout (4000ms), and signaling GPU fence on CPU on timeout.
  - `fence_x.rs`: Periodic background polling (10ms) in `xcl-fences`, title fence completion cell updates, and `rescue_stalled_title_fence` to prevent render thread starvation.
  - `list_x.rs`: Command list submission tracking, wait-cycles avoidance, and stall telemetry.
- **Staging Memory & Texture Upload Throttling on D3DMetal** (`crates/xcl/src/hle/d3d12x/title_textures.rs`, `crates/xcl/src/gpu/texture.rs`):
  - `title_textures.rs`: Mac host batch limit (32 MiB, `batch_bytes_limit`), max items limit (24), and per-submit pending texture quota (64 MiB / 32 textures per submit) to decouple massive 1-2 GiB loading bursts across frames.
  - `texture.rs`: Retained staging buffer trimming in `put_back()` (<= 64 MiB) and GPU queue synchronization (`queue.Wait`) to eliminate CPU stalls and device removal.
- **3D World Geometry Handoff & Player Control Transition** (`crates/xcl/src/hle/`):
  - `real.rs`: `mesh_shader_slots` remapping console MS in AS slot into subobject 25 (`D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS`), constructing translated two-pass compute + mesh stream.
  - `bind.rs`: Two-pass `DispatchMesh` execution, UAV scratch barrier emit, and indirect mesh dispatch.
  - `list_x.rs`: Vertex buffer purging (`ia_set_vertex_buffers_x`) on null/zero-sized views during transition from 2D loading to 3D world.
  - `gameinput.rs` & `kbm.rs`: GDK `GameInput.dll` COM interfaces, physical XInput polling, and keyboard/mouse mapping (WASD -> left stick, mouse look -> right stick, automatic cursor clipping via `ClipCursor`).
  - `xgameruntime/mod.rs`: `XUserRegisterForDeviceAssociationChanged` binding `XCL gamepad 0` to `LOCAL_ID (1)`.
- **Platform Gating & Verification Parity** (`crates/xcl/src/util.rs`, `scripts/verify_mac_build.sh`):
  - Strict host gating behind `crate::util::mac_host()`.
  - Subsystem verification script enforcing 39/39 (and new 40/40) checks cleanly, and Windows GNU cross-compilation with 0 errors.

## Feature Inventory
| # | Feature | Description | Milestone | Source |
|---|---------|-------------|-----------|--------|
| 1 | Fence 3766 q0 Stall Elimination & Bottom-of-Pipe Rescue | Active watchdog rescue in cpu_writes.rs and fence_x.rs to ensure queue q0 advances past fence 3767 before 20s watchdog | M1 | Survey (R1) |
| 2 | Title Fence Periodic Polling & Cell Watchdog | 10ms polling in xcl-fences thread to prevent render thread 432 deadlocks on title fence 0x6000013dc310 | M1 | Survey (R1) |
| 3 | D3DMetal Staging Buffer & Texture Batch Sizing | Cap texture upload batches to 32 MiB and max 24 items under mac_host() to eliminate Metal command buffer saturation & 0x887A0005 | M2 | Survey (R2) |
| 4 | Loading-to-World Burst Throttling | Enforce per-submit pending quota (64 MiB / 32 textures per submit) in title_textures.rs:refresh() to decouple 1-2 GiB texture bursts | M2 | Survey (R2) |
| 5 | Non-Blocking Texture Upload GPU Sync & Staging Cap | Replace synchronous CPU wait with queue.Wait(&up.fence, up.value) and cap retained staging buffer to 64 MiB | M2 | Survey (R2) |
| 6 | Two-Pass DispatchMesh Pipelines & Subobject 25 Bindings | Cleanly translate console MS in AS slot to subobject 25 and execute two-pass compute + mesh pipeline with UAV barrier | M3 | Survey (R3) |
| 7 | Vertex Buffer Slot Purge on World Transition | Purge null and zero-sized vertex buffer bindings in list_x.rs during loading-to-world handoff | M3 | Survey (R3) |
| 8 | Interactive Player Control Transition in 3D World | Enable GameInput virtual gamepad, kbm mapping (WASD movement, mouse look, cursor clipping), and XUser device association | M3 | Survey (R3) |
| 9 | Subsystem Verification & Cross-Platform Parity | Enforce 100% clean pass on verify_mac_build.sh (39/39+), cargo check on x86_64-pc-windows-gnullvm (0 errors), and mac_host() isolation | M4 | Survey (R4) |
| 10 | Requirement-Driven Opaque-Box E2E Testing Suite | Comprehensive E2E test suite covering queue execution, texture throttling, mesh pipelines, and input transitions (Tiers 1-4) | M_E2E | Survey (Criteria) |
| 11 | Full E2E Pass & Adversarial Coverage Hardening | 100% pass of E2E suite + Tier 5 adversarial stress testing on Mac Apple Silicon | Final | Survey (Criteria) |

## Milestones
| # | Name | Scope | Dependencies | Status |
|---|------|-------|-------------|--------|
| M1 | Fence 3766 Command Queue q0 Stall Elimination | R1: `cpu_writes.rs`, `fence_x.rs`, `list_x.rs` active watchdog rescue | none | IN_PROGRESS |
| M2 | Staging Memory & Texture Upload Throttling on D3DMetal | R2: `title_textures.rs`, `texture.rs` batch limits, submit quota, and staging cap | none | IN_PROGRESS |
| M3 | 3D World Geometry Handoff & Player Control Transition | R3: `real.rs`, `bind.rs`, `list_x.rs`, `gameinput.rs`, `kbm.rs`, `xgameruntime/mod.rs` | M1, M2 | PLANNED |
| M4 | Subsystem Verification & Cross-Platform Parity | R4: `scripts/verify_mac_build.sh`, `util.rs`, cross-check parity | M1, M2, M3 | PLANNED |
| M_E2E | Requirement-Driven Opaque-Box E2E Testing Track | Test harness and test suites across Tiers 1-4, publishing TEST_READY.md | none | DONE |
| Final | 100% E2E Test Pass & Adversarial Hardening | Pass 100% E2E tests + Tier 5 coverage hardening with Challenger-Worker-Reviewer | M1, M2, M3, M4, M_E2E | PLANNED |

## Interface Contracts
### `cpu_writes` ↔ `fence_x`
- `cpu_writes::Job`: Executes non-UMA writes on timeout; invokes `fence_x::rescue_stalled_title_fences(job.queue)` when GPU execution fails to reach `job.value` within timeout.
- `fence_x::rescue_stalled_title_fences(queue: usize)`: Iterates pending title fences registered on `queue` where `f.signalled > completed`, updates `cell.0` to `f.signalled`, and signals underlying fence on host.

### `title_textures` ↔ `gpu::texture`
- `title_textures::batch_bytes_limit() -> usize`: Returns 32 MiB on `mac_host()`, 256 MiB on Windows.
- `title_textures::MAC_MAX_BATCH_ITEMS = 24`: Enforces maximum textures per batch.
- `title_textures::refresh`: Enforces `MAC_MAX_SUBMIT_UPLOAD_BYTES = 64 MiB` and `MAC_MAX_SUBMIT_UPLOAD_COUNT = 32`, deferring remaining textures across frames.
- `gpu::texture::put_back(key, up)`: Trims staging capacity to <= 64 MiB under `mac_host()`.

### `real` ↔ `bind` (Mesh Shader Two-Pass)
- `real::mesh_shader_slots(ms, as_slot) -> (Option<&[u8]>, Option<&[u8]>)`: Detects console MS in AS slot, assigns to subobject 25.
- `bind::dispatch_mesh(this, x, y, z)`: Allocates scratch buffer `ensure_res_buffer`, binds Pass 1 compute, dispatches, emits UAV barrier, binds Pass 2 mesh, dispatches, emits UAV barrier.

### `gameinput` ↔ `kbm` ↔ `xgameruntime`
- `gameinput::poll() -> GamepadState`: Merges `xinput_pad()` with `kbm::apply` and scripted inputs.
- `kbm::hold_cursor`: Clips and centers cursor in XCL window via `ClipCursor`.
- `xgameruntime::notify_association`: Associates `XCL gamepad 0` (`DEVICE_ID`) with `LOCAL_ID (1)`.

## Code Layout
- `crates/xcl/src/hle/d3d12x/cpu_writes.rs`: Bottom-of-pipe queue stall rescue and watchdog timing.
- `crates/xcl/src/hle/d3d12x/fence_x.rs`: Periodic background polling and title fence rescue.
- `crates/xcl/src/hle/d3d12x/title_textures.rs`: Texture upload batch sizing, submit throttling quota.
- `crates/xcl/src/gpu/texture.rs`: Texture staging buffer allocation and retention cap.
- `crates/xcl/src/hle/d3d12x/real.rs`: Mesh shader subobject 25 mapping and queue execution.
- `crates/xcl/src/hle/d3d12x/bind.rs`: Two-pass `DispatchMesh` scratch buffer execution.
- `crates/xcl/src/hle/d3d12x/list_x.rs`: Vertex buffer unbind on null/zero views and stall telemetry.
- `crates/xcl/src/hle/gameinput.rs`: GameInput COM interface and controller merging.
- `crates/xcl/src/hle/kbm.rs`: Keyboard/mouse to gamepad translation and cursor clipping.
- `crates/xcl/src/hle/xgameruntime/mod.rs`: Gamepad device user association.
- `crates/xcl/src/util.rs`: Platform gating (`mac_host`, `rosetta`).
- `scripts/verify_mac_build.sh`: Subsystem verification script.
