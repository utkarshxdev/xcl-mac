# Stage 6.4 Run 13 review

**Status.** Run 13 is a candidate for tester validation. The available title logs end at Run 12. They show a GPU queue stall before playable control. The previously packaged Run 13 executable was built from `c61e8dc8`. Its ZIP and executable match the published SHA-256 values.

## What the title logs show

| Observation | Run 11 | Run 12 |
| --- | --- | --- |
| Last completed internal fence | 3760 | 3791 |
| First incomplete internal fence | 3761 | 3792 |
| Producer | Direct queue q0 | Direct queue q0 |
| Pending waits recorded by XCL | None | None |
| Translated indirect compute shader in the affected list | `36204a134ee59578` | `36204a134ee59578` |
| Title GPU location signature | `7333478d` | `7333478d` |
| Display planes observed | 1 | 1 |

See Run 11 log lines 48175–48178 and Run 12 log lines 48169–48172. Both submissions end with the same sequence of compute shaders. The snapshot covers an entire command list. It does not identify the first incomplete operation. Fence values count submissions, so the increase of 31 does not demonstrate progress into the world.

## Candidate for the next test

The patch keeps the `ExecuteIndirect` ordering barriers on translated paths and labels ordinary and repacked indirect arguments separately in the stall log. Upstream reproduced a stale argument read on NVIDIA and found that an extra transition prevented it. The local D3DMetal baseline probe read all 40 updates correctly on a direct queue and all 40 on a compute queue without the extra transition. [Upstream reproduction](https://github.com/xcl-dev/xcl/pull/1791)

The local D3DMetal backend skipped a repacked indirect compute call whose command signature set a root UAV. For a single translated compute dispatch, XCL now binds that UAV directly from the known scratch address. A plain indirect command reads the dimensions and optional count from GPU-visible memory. The new GPU test verified the shader receives the repacked root values and that a zero count suppresses execution. Seven focused tests passed with zero failures or skips. The separate multi-command repack test still fails on this local backend because the host skips its root-changing command signature.

Shadow copies of root bindings now reset the entries modified by the indirect command, matching D3D12 semantics. Presentation uses the previous path because the available traces only show one plane. The watchdog follows its existing opt-in behavior. The diagnostic build should run with `XCL_WATCHDOG_BYPASS=0` and `XCL_TRACE_GPU_STALL=1` in the tester's existing bottle. [D3D12 indirect command signatures](https://learn.microsoft.com/en-us/windows/win32/direct3d12/indirect-drawing)

The game-level criterion is continued GPU completion through the loading transition and player movement in a visible world. Collect the test executable's SHA-256, a fresh `xcl.log`, and `report.json` together so the result can be tied to a specific build.
