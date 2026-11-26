# Test Infrastructure Documentation: GTA VI XCL Mac Native Integration

## 1. Overview & Objectives

The XCL Mac Native test infrastructure provides requirement-driven, opaque-box end-to-end (E2E) testing for GTA VI running on Apple Silicon (macOS via Wine/CrossOver + D3DMetal).

This testing track independently verifies the critical subsystems established in Stage 6.4:
- **Requirement R1**: Primary graphics command queue `q0` stall elimination, bottom-of-pipe rescue, title fence cell updates, and avoidance of the title's 20-second watchdog crash.
- **Requirement R2**: D3DMetal texture upload throttling, host staging memory capping (<= 64 MiB), batch item/byte sizing (32 MiB / 24 items), submit quotas (64 MiB / 32 textures per submit), and GPU fence synchronization.
- **Requirement R3**: 3D world geometry handoff, console mesh shader subobject 25 remapping, two-pass `DispatchMesh` scratch buffer execution, vertex buffer slot purging, GameInput virtual gamepad/KBM mapping, and XUser device association.
- **Requirement R4**: Platform gating parity behind `crate::util::mac_host()`, `scripts/verify_mac_build.sh` verification suite, and Windows GNU cross-compilation parity.

---

## 2. Feature Inventory & Mapping

| Feature ID | Requirement | Primary Subsystems | Source Files | Test Coverage |
|------------|-------------|--------------------|--------------|---------------|
| **F-01** | R1: Fence 3766 Stall Elimination & Bottom-of-Pipe Rescue | Queue Watchdog, CPU Writes Worker, Title Fence Rescue | `crates/xcl/src/hle/d3d12x/cpu_writes.rs`, `crates/xcl/src/hle/d3d12x/fence_x.rs`, `crates/xcl/src/hle/d3d12x/list_x.rs` | Tier 1 (R1.1–R1.6), Tier 2 (Queue Wrap, Multi-Fence), Tier 3, Tier 4 |
| **F-02** | R1: Title Fence Periodic Polling & Completion Cell | Background Polling (10ms), Cell Atomic Synchronization | `crates/xcl/src/hle/d3d12x/fence_x.rs` | Tier 1 (R1.4, R1.5), Tier 3, Tier 4 |
| **F-03** | R2: Staging Memory & Texture Upload Batch Sizing | Batch Byte Limit (32 MiB), Batch Item Limit (24) | `crates/xcl/src/hle/d3d12x/title_textures.rs` | Tier 1 (R2.1, R2.2), Tier 2 (256MB burst), Tier 3, Tier 4 |
| **F-04** | R2: Submit Upload Quota & Loading Decoupling | Per-submit quota (64 MiB / 32 textures), Deferred Uploads | `crates/xcl/src/hle/d3d12x/title_textures.rs` | Tier 1 (R2.3, R2.4), Tier 2, Tier 4 (Loading Handoff) |
| **F-05** | R2: GPU Queue Wait & Staging Buffer Retention Cap | Non-blocking `queue.Wait`, Staging Buffer Cap (<= 64 MiB) | `crates/xcl/src/gpu/texture.rs` | Tier 1 (R2.5, R2.6), Tier 2 (Staging boundary), Tier 3 |
| **F-06** | R3: Console Mesh Shader Subobject 25 Remapping | AS slot to Subobject 25 (`D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS`) | `crates/xcl/src/hle/d3d12x/real.rs`, `crates/xcl/src/hle/d3d12x/null.rs` | Tier 1 (R3.1), Tier 2 (Malformed stream), Tier 3, Tier 4 |
| **F-07** | R3: Two-Pass DispatchMesh Execution | Compute Pass 1 -> UAV Barrier -> Mesh Pass 2 -> UAV Barrier | `crates/xcl/src/hle/d3d12x/bind.rs` | Tier 1 (R3.2), Tier 2 (Zero groups), Tier 3, Tier 4 |
| **F-08** | R3: Vertex Buffer Purging on World Transition | Unbind on null or 0-sized vertex buffer views | `crates/xcl/src/hle/d3d12x/list_x.rs` | Tier 1 (R3.3), Tier 3, Tier 4 (Lucia Prologue) |
| **F-09** | R3: GameInput Virtual Controller & KBM Mapping | WASD Left Stick, Mouse Look Right Stick, Cursor Clipping | `crates/xcl/src/hle/gameinput.rs`, `crates/xcl/src/hle/kbm.rs` | Tier 1 (R3.4, R3.5), Tier 2 (Rapid switch), Tier 3, Tier 4 |
| **F-10** | R3: XUser Device Association Binding | `XCL gamepad 0` bound to `LOCAL_ID (1)` | `crates/xcl/src/hle/xgameruntime/mod.rs` | Tier 1 (R3.6), Tier 3, Tier 4 (Jason Movement) |
| **F-11** | R4: Mac Host Gating & Subsystem Verification Parity | `crate::util::mac_host()`, `verify_mac_build.sh`, cross-target | `crates/xcl/src/util.rs`, `scripts/verify_mac_build.sh` | Tier 1 (R4.1–R4.6), Tier 3, Tier 4 |

---

## 3. Test Suite Architecture & Test Tiers

The test suite is structured into four progressive tiers to validate behavior from isolated unit functionality up to full game mission sequences:

```
+--------------------------------------------------------------------------------+
| Tier 4: Real-World Application Scenarios (Lucia Prologue, Jason Fishing, etc.) |
+--------------------------------------------------------------------------------+
                                       ^
+--------------------------------------------------------------------------------+
| Tier 3: Cross-Feature Interactions (Pairwise combinations of R1, R2, R3, R4)   |
+--------------------------------------------------------------------------------+
                                       ^
+--------------------------------------------------------------------------------+
| Tier 2: Boundary & Corner Cases (Queue wrap, 0-byte, 256MB burst, empty mesh)  |
+--------------------------------------------------------------------------------+
                                       ^
+--------------------------------------------------------------------------------+
| Tier 1: Feature Coverage (>=5 test cases per requirement R1, R2, R3, R4)       |
+--------------------------------------------------------------------------------+
```

### Tier 1: Feature Coverage (24 Tests)
- **R1: Command Queue Stall Elimination & Bottom-of-Pipe Rescue** (6 tests)
  - Watchdog timeout trigger at 4000ms.
  - CPU execution of deferred bottom-of-pipe writes upon timeout.
  - GPU fence signaling on CPU to advance queue past stall (3766 -> 3767).
  - Title fence rescue advancing `cell.0` from 0x1d6c to 0x1d6d.
  - Periodic 10ms background polling in `xcl-fences` thread.
  - Prevention of the title's 20-second watchdog crash (`0xc0000005` at `+0x23cf683`).
- **R2: Staging Memory & Texture Upload Throttling on D3DMetal** (6 tests)
  - Mac host batch byte limit capped strictly to 32 MiB.
  - Mac host batch item limit capped strictly to 24 items.
  - Per-submit pending upload quota capped to 64 MiB and 32 textures.
  - Decoupling of massive 512 MiB loading bursts across multiple frames.
  - Non-blocking GPU queue synchronization via `queue.Wait(&fence, val)`.
  - Staging buffer retention trimming to <= 64 MiB on Mac host.
- **R3: 3D World Geometry Handoff & Player Control Transition** (6 tests)
  - Console MS container in AS slot remapped to subobject 25.
  - Two-pass `DispatchMesh` execution with scratch buffer allocation and dual UAV barriers.
  - Vertex buffer slot purging on null/zero-sized views during 2D-to-3D handoff.
  - Keyboard WASD translation to normalized left thumbstick coordinates.
  - Mouse look delta translation to right thumbstick with automatic cursor clipping.
  - XUser device association registering `XCL gamepad 0` to `LOCAL_ID (1)`.
- **R4: Subsystem Verification & Cross-Platform Parity** (6 tests)
  - Strict isolation behind `crate::util::mac_host()`.
  - Windows host passthrough without Mac throttling overrides.
  - Validation of all checks in `scripts/verify_mac_build.sh`.
  - Cross-compilation parity for `x86_64-pc-windows-gnullvm`.
  - Rosetta CPU brand detection for Apple Silicon environments.
  - Memory alignment and thread safety across concurrent workers.

### Tier 2: Boundary & Corner Cases (8 Tests)
- `u64` queue fence counter near `0xFFFF_FFFF_FFFF_FFFF` wraps safely.
- 0-byte texture or empty mip payload handles cleanly without allocation.
- Oversized 256 MiB texture bursts split into bounded chunks under Mac quota.
- Empty mesh view or `DispatchMesh(0, 0, 0)` exits early without allocating scratch memory.
- Rapid alternating toggling between physical gamepad and KBM inputs maintains smooth stick interpolation.
- Pipeline state stream containing malformed or unknown subobject types parses gracefully.
- Staging buffer retention trimming tests exact 64 MiB boundary (64 MiB retained, 64 MiB + 1 byte trimmed).
- Multiple stalled queues and title fences rescued simultaneously in a single pass.

### Tier 3: Cross-Feature Interactions (6 Tests)
- Texture upload burst during active two-pass `DispatchMesh` pipeline.
- Command queue stall watchdog triggered while texture upload batch is in-flight.
- Player input received during scene transition and vertex buffer purging.
- Throttled texture upload completion paired with 10ms title fence cell polling.
- Two-pass mesh dispatches combined with bottom-of-pipe memory writes.
- Virtual controller device association during rapid window focus transitions and cursor clipping.

### Tier 4: Real-World Application Scenarios (5 Tests)
- **Lucia Prologue Transition**: Bink video finish -> 2D loading screen -> vertex buffer purge -> subobject 25 mesh pipeline bind -> 3D scene render without 0x20202C slate color.
- **Loading Screen Burst Handoff**: 1 GiB city texture loading burst throttled at 64 MiB/submit across frames -> zero device removal (0x887A0005) -> queue advances past fence 3767.
- **Jason's Gone Fishing Mission Movement**: In-world player navigation in "Vice City, 2 years earlier" -> WASD and mouse look inputs control character smoothly.
- **Cold-Boot to World Pipeline Initialization**: Engine startup -> shader cache stub -> queue init -> memory mirroring -> 3D geometry transition without assertion aborts.
- **8-Minute Open-World Endurance Simulation**: 500 consecutive frame iterations with active queue execution, periodic fence polling, texture streaming, and mesh dispatch, verifying zero watchdog crashes.

---

## 4. Expected Output Derivation Methodology

All test assertions are derived strictly and independently from the authoritative requirements specified in `ORIGINAL_REQUEST.md` (Follow-up 2026-11-26T07:29:40Z) and `PROJECT.md`:
1. **Queue Progress**: Target fence values (e.g. 3767 on `q0`) must advance within <= 4000ms.
2. **Title Fence Cell**: `cell.0` memory address must reflect signaled value `0x1d6d` prior to 20.0s watchdog deadline.
3. **Texture Batch Sizing**: Batches on Mac must never exceed `32 * 1024 * 1024` bytes or `24` items.
4. **Submit Quota**: Pending texture uploads per submit must never exceed `64 * 1024 * 1024` bytes or `32` textures.
5. **Staging Buffer Size**: Retained staging pool size on Mac host must never exceed `64 * 1024 * 1024` bytes.
6. **Subobject Remapping**: Input AS subobject (type 24) containing MS stage bytecode must be output as subobject 25 with AS stripped.
7. **Mesh Dispatch**: Scratch buffer allocation size must equal `groups * dwords * 4`, with exactly 2 UAV barriers emitted.
8. **Controller Input**: W key alone produces Left Stick `(0.0, 1.0)`, D key produces `(1.0, 0.0)`. Device ID maps to Local ID 1.

---

## 5. Execution Instructions

The test suite runs using standard Python 3 `unittest` without external dependencies:

```bash
# Run the Stage 6.4 E2E test suite
python3 -m unittest tests/e2e/test_stage6_4_e2e.py

# Run with verbose output
python3 -m unittest -v tests/e2e/test_stage6_4_e2e.py

# Run all E2E test suites
python3 -m unittest discover -s tests/e2e -p "test_*.py"
```
