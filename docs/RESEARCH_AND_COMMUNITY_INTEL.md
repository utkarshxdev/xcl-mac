# Community Research Intel & Technical Knowledge Base

This document consolidates external research, findings from leading community forks, diagnostic telemetry from Mac test runs, and architectural blueprints for automated agent workflows.

---

## 1. Upstream & Competitor Intel: `DoubleGhosty/cool_fox` (`codec-d`)

### Overview & Current Status (as of 2026-11-26)
* **Repository:** `DoubleGhosty/cool_fox` (mirror of `codec-d/cool_fox`, based on `avatarmanz/VIctory-7900XTX`).
* **Achievement:** Achieved active 3D world rendering (opening street, characters, dynamic lighting) sustained for 135+ seconds at ~54.3 ms/frame (~18–20 FPS).
* **Target Hardware:** Windows 11, Intel i9-12900K, **64 GB RAM**, RTX 5090 / RX 7900 XTX.

### Why it CANNOT Run Directly on Apple Silicon Macs
* **Memory Explosion:** Peak process commit reached **34.42 GiB**. On 16 GB Apple Silicon unified memory, it triggers an instant kernel panic / system freeze due to massive swapping and `E_OUTOFMEMORY (0x8007000E)` aborts.
* **Wine Incompatibilities:** Shader compiler panics on SM 5.1/6.6 under Wine; multi-threaded scheduler (`VICTORY_THIN`, `VICTORY_RELAY`) is hard-coded for Windows MSVC threads.

### Cherry-Pickable Solutions for XCL
The `docs/bug-journal.md` of `cool_fox` details fixes for rendering bugs that occur immediately after passing the loading sequence:

1. **Series S 2-Plane Black Screen Fix:**
   * *Problem:* Screen remains black after the video sequence finishes on Series S profiles.
   * *Solution:* Read `RetailPlane` at a **72-byte stride** instead of default packing.
2. **ExecuteIndirect Sizing:**
   * *Problem:* An oversized indirect command list zeroes out subsequent draws.
   * *Solution:* Default `VICTORY_INDIRECT_REST=0` so oversized records do not clear valid pipeline state.
3. **Wave32 Transmissive & Alpha Prefilter Fix:**
   * *Problem:* Character eyes, windshields, and particle effects render as black rectangles under 32-lane SIMD (Apple Silicon / NVIDIA).
   * *Solution:* In `resource::Tracer`, do not prune dead join edges across lanes without checking `lanes.guards[b][i]`; prevents transmission buffer zeroing in `alphaResolve:CS_AlphTransmissionPrefilterMip`.
4. **Facial Deformation Meshes:**
   * *Problem:* Deformed or collapsed character face meshes.
   * *Solution:* Restrict typed integer lowering (`bufferLoad.i32`/`bufferStore.i32`) to `d16` typed formats (`buffer_load_format_d16_*`).
5. **Synchronous Bake Uploads:**
   * *Problem:* Textures for pedestrian clothes, faces, and vehicles pop in as grey/neutral textures.
   * *Solution:* Route one-shot bake uploads through a dedicated synchronous command list (`execute_sync_internal`) with immediate slot publication (`prepare_bake_slot`).

---

## 2. Mac Test Run Diagnostic Log & Progression Directory

| Run | Stalled At | Error / GPU Loc Sig | Root Cause | Resolution |
| :--- | :--- | :--- | :--- | :--- |
| **Run 9** | Fence `3766 → 3767` | Empty BVH Descriptors | BVH acceleration structure descriptor all zeroes (`0x0`); traversal reading unmapped memory. | Commit `929aa933`: Return immediate ray miss (`uint4(0x7F800000u, 1.0f, 0, 0)`) when address is zero. |
| **Run 10** | Fence `3706 → 3707` | `GPU Loc Sig: 921cc4ae`<br>Crash at `+0x23cf683` (write to `0x0`) | Three P1 blockers from commit `4ab1596c`:<br>1. CPU fence advance (`job.fence.Signal`) causing use-after-free.<br>2. Staging buffer drop on put-back.<br>3. Oversized texture upload starvation. | Commit `9db09892`: Reverted CPU signaling; kept upload staging alive until GPU fence; allowed at least 1 texture per submit. Added command list stall logging. |
| **Run 11** | Fence `3760 → 3761` | `GPU Loc Sig: 7333478d`<br>Crash at `+0x23cf683` (20s watchdog) | Advanced +54 fences past Run 10. Queue stalled on `ExecuteIndirect` preceded by 11 compute dispatches; D3DMetal read argument buffer before writes drained. | Commit `35f3915b`: Wrapped `ExecuteIndirect` with a 256-byte buffer transition `COMMON → INDIRECT_ARGUMENT → COMMON` (upstream `9486494d`). |
| **Run 12** | Pending Test | Target: Advance past 3761 into 3D world | Built and published in `v0.1.0-stage6.4-indirect-barrier-test.1`. | Tests whether indirect arguments drain properly under D3DMetal. |

### The `+0x23cf683` Watchdog Mechanism
* The recurring exception `0xc0000005` in `game_scarlett_master.exe+0x23cf683 (write at address 0x0)` with vtable `+0x71b6308` is **not a memory corruption bug in XCL**.
* It is the game's internal **20-second GPU watchdog**. When the GPU stops advancing fences for exactly 20 seconds, thread 440 intentionally triggers a null-pointer dereference to force Breakpad (`http_upload.cc`) to capture a minidump with a `GPU Loc Sig`.

---

## 3. Infrastructure & Agent Orchestration Blueprints

### A. Automated Mac Headless Test Script
To run tests automatically on a Mac without manual GUI clicks:
```bash
#!/bin/bash
# automated_runner.sh
export XCL_TRACE_GPU_STALL=1
export XCL_ACTIVITY=1
export XCL_DUMP_THREADS_AT=150,160,165,175

# Run with a 3.5 minute timeout
timeout 210 wine64 xcl.exe run "C:\Games\GTA6\game_scarlett_master.exe" > run.stdout 2>&1

# Parse results
FENCE_LINE=$(grep "bottom-of-pipe writes still pending" xcl.log | tail -1)
LOC_SIG=$(grep "GPU Loc Sig" xcl.log | tail -1)

echo "Result: $FENCE_LINE | $LOC_SIG"
```

### B. University S200 Cluster Bridge (Bypassing Air-Gaps)
1. **Firewall / HTTPS Test:**
   * Universities often block SSH (port 22) but allow HTTPS (port 443).
   * Verify using: `git ls-remote https://github.com/utkarshxdev/xcl-mac.git`. If this works, push via HTTPS + Personal Access Token (PAT).
2. **NFS Shared Home Directory Bridge:**
   * If compute nodes are strictly isolated from the internet, run a watcher on the **Login Node** (which has internet) while the AI agent runs in `tmux` on the **Compute Node**:
   ```bash
   # On Login Node:
   while true; do
     if [ -f /home/user/project/.push_trigger ]; then
       cd /home/user/project && git push origin $(git rev-parse --abbrev-ref HEAD)
       rm -f .push_trigger
     fi
     sleep 15
   done
   ```
3. **Tailscale (Userspace Mode):**
   * If outgoing UDP/HTTPS is permitted, install Tailscale in user mode (`curl -fsSL https://tailscale.com/install.sh | sh`) to SSH into the cluster remotely from outside campus without needing university VPN.
