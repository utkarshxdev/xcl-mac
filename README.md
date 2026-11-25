# XCL Mac Native — Apple Silicon & macOS Compatibility Layer for Xbox Series X|S

[![Discord](https://img.shields.io/badge/Discord-Join%20Community-5865F2?logo=discord&logoColor=white)](https://discord.gg/dYuNEsQMQ)
[![Mac cross-build](https://github.com/utkarshxdev/xcl-mac/actions/workflows/mac-cross.yml/badge.svg?branch=main)](https://github.com/utkarshxdev/xcl-mac/actions/workflows/mac-cross.yml?query=branch%3Amain)
[![Stage 6: Ingame 3D Rendering](https://img.shields.io/badge/Status-Stage%206%20(Ingame%203D)-success?logo=apple&color=34D399)](https://github.com/utkarshxdev/xcl-mac)
[![License: GPL-2.0](https://img.shields.io/badge/License-GPL%202.0-blue.svg)](LICENSE)
[![Platform: macOS](https://img.shields.io/badge/Platform-macOS%20%7C%20Apple%20Silicon-black?logo=apple)](https://github.com/utkarshxdev/xcl-mac)

**XCL Mac Native** (`utkarshxdev/xcl-mac`) is the dedicated, production-ready macOS and Apple Silicon distribution of [XCL](https://github.com/ivall/xcl) — the Xbox Series X|S (GDK) compatibility layer. 

By executing console x86_64 code seamlessly via Apple's Rosetta 2 and mapping RDNA2 console GPU pipelines through DirectX 12 into Apple **D3DMetal (Game Porting Toolkit 2+)**, XCL enables GDK retail titles to run on macOS under Wine and CrossOver **without CPU emulation**.

XCL works only on unpacked game files you already own. It does not download, decrypt or unpack game packages (XVC / MSIXVC) and contains nothing for circumventing DRM. The repository holds no code, headers or binaries from the console SDK, no keys and no game data; the Windows components XCL relies on (XGameRuntime, XAudio2, D3D12, DXC) come from your own system or the public Windows SDK and are not distributed with XCL.

This repository consolidates community breakthroughs from Ahmet Dedeler ([`Ahmet-Dedeler/xcl`](https://github.com/Ahmet-Dedeler/xcl)), Michael McCrimmons ([`mikebuckets171/xcl`](https://github.com/mikebuckets171/xcl)), and upstream PRs [#666](https://github.com/ivall/xcl/pull/666), [#541](https://github.com/ivall/xcl/pull/541), [#258](https://github.com/ivall/xcl/pull/258), and [#257](https://github.com/ivall/xcl/pull/257) on top of upstream `beaa245`.

---

## 🔬 The "30 FPS Illusion" & Real Frame Progression

In early community Mac experiments, frame counters often reported a steady "30 FPS", leading to assumptions that gameplay was already functioning smoothly. Rigorous telemetry and automated frame-dump analysis on an Apple Silicon M5 Pro revealed the underlying reality:

| Metric (120 s benchmark) | Baseline Port (`db065d8`) | Consolidated `mac-native` (`ee3c780` + patches) |
| :--- | :--- | :--- |
| **Frames Presented** | 2,255 (~18.8 FPS) | **2,576 (~21.5 FPS)** |
| **Console-MS Refusals** | 40 | **0** |
| **Translated Draws / Dispatches** | 32,768 / 1,621 | **41,914 / 2,048** |
| **2-second Windows with 0 Presents** | 18 of 59 | **13 of 59** |
| **Frame Dumps Compared (~1s apart)** | 64 pairs | **75 pairs** |
| **Pairs Where Picture Actually Changed** | **0 of 64 (0%)** | **Real 3D scene progression** |

### What Went Wrong in Unpatched Builds:
1. **D3DMetal Shader Refusals:** Dual-source blending (`SV_Target0` + `SV_Target1`) and missing PC programs caused D3DMetal to silently discard geometry draw calls. The presentation loop kept presenting an identical 2D loading frame.
2. **Rosetta 2 Forward-Emulation Panic:** AVX `VMASKMOVPS` / `VPMASKMOVD` instructions faulting across page boundaries triggered fatal crashes: `rosetta error: unexpectedly need to EmulateForward on a synchronous exception`.
3. **Texture Streaming Lockups:** Because `MEM_WRITE_WATCH` was disabled to protect Rosetta 2 from memory page traps, streamed textures and font glyph atlases were uploaded once and never refreshed.

### How `mac-native` Fixes It:
- **Dual-source blending stand-in pixel shaders** ensure D3DMetal pipelines are always created and never refused.
- **Dynamic AVX masked-store patching** decomposes risky vector stores into element-wise operations with registered Windows `UNWIND_INFO`.
- **Content-based texture polling & UMA shadow memory copies** keep streaming textures moving without triggering page protection faults.

For complete benchmarks, telemetry diffs, and hardware logs, see [`docs/MAC_BUILDS_ANALYSIS.md`](docs/MAC_BUILDS_ANALYSIS.md).

---

## ✨ Mac-Native Core Subsystems

All Mac-specific subsystems are strictly gated behind `crate::util::mac_host()` or `crate::util::rosetta()` so native Windows builds remain 100% bug-for-bug identical to upstream `main`.

### 1. Apple Silicon Rosetta 2 Compatibility
- **Restartable Masked-Store Patching (`crates/xcl/src/maskmov.rs`):** Intercepts and rewrites AVX `VMASKMOVPS` and `VPMASKMOVD` instructions at load time. Routes them through element-by-element safe stores equipped with structured exception handling unwinding metadata, completely bypassing Rosetta 2 forward-emulation traps.
- **Missing AMD Zen 2 Instruction Emulation:** Implements runtime handlers for AMD instructions absent in Apple Silicon Rosetta 2: `CLZERO`, `INSERTQ`, `MONITORX`, `CLWB`, `MOVNTSS`, and `MOVNTSD`.
- **16 MiB Title Stacks:** Prevents guard-page stack overflows under Wine and CrossOver by allocating full 16 MiB console thread stacks.

### 2. Apple D3DMetal & MetalFX Integration
- **COM Vtable Prefix Mirroring (`PREFIX = 16`):** D3DMetal retrieves internal Metal encoders at negative offsets (`ext - 16`). Extended COM vtables mirror 16 prefix header words to guarantee reliable dispatch without memory corruption.
- **Dual-Source Blending Stand-Ins:** Implements `SV_Target0` + `SV_Target1` stand-in pixel shaders (PR [#666](https://github.com/ivall/xcl/pull/666)), preventing D3DMetal pipeline creation rejections.
- **Command-List Creation:** Device slot 73 distinguishes shader-cache sessions by their explicit interface IDs. On Mac hosts, console-only command-list IDs are translated to the standard PC graphics command-list interface.
- **Depth-Only Format Mapping:** Automatically converts unsupported D3D12 depth-stencil views into compliant depth-only formats under D3DMetal.
- **Compute Blob Memory Budget Charging:** Eliminates redundant cache eviction churn in `keep_compute_blob`, charging budget only on vacant entries.

### 3. Unified Memory Architecture (UMA) Fast Paths
- **UMA Video Memory Shadowing (`Uma::shadowed`):** Emulates console shared memory without requiring `OpenExistingHeapFromAddress` (unsupported by Wine), enabling high-speed zero-copy CPU-to-GPU data flow.
- **Command Queue #31 GPU View Page Mappings:** Routes console GPU view page tables directly into host memory.
- **Content-Based Texture Polling (`poll_unwatched`):** Evaluates texture content changes via fast 64-bit hashing when write-watch is unavailable. Polling and overlap handling guard texture references against concurrent release.
- **Texture Uploads on Mac:** Keep `XCL_TEXTURE_SKIP_UNCHANGED=0` or leave it unset. Enabling the shortcut has produced blank frames on D3DMetal.

### 4. Wine & CrossOver Stability
- **XGameRuntime fallback** uses XCL's built-in async and accessibility APIs if the host runtime cannot initialize or provide its XAsync interface. That backend choice stays in place for the lifetime of the process.
- **Isolated DXC Thread Heaps (`own_heaps`):** Allocates private heap spaces for parallel DXC shader compilation threads, eliminating concurrency crashes in Wine's memory manager.
- **DirectStorage BCPack:** Decodes BC1, BC3, BC4, BC5, BC6H, and BC7 requests using the upstream XCL decoder.
- **Default Network Isolation (`netoff.rs`, `XCL_NETWORK=0`):** Stubs out telemetry, loopback pinging, and external socket hangs, ensuring seamless offline game startups.

---

## 🎮 Game Compatibility

| Game | Package / Version | State | What Works on macOS | Notes |
|---|---|---|---|---|
| **Minecraft Preview** | `Microsoft.MinecraftBeta` 1.18.3064.0 | ✅ | Fully playable | 60+ FPS, world generation, audio, gamepad |
| **Unicorn Overlord** | `SEGAofAmericaInc.Pa253b0c1Y` 1.0.0.6 | ✅ | Fully playable | Full campaign, combat, 2D/3D sprites |
| **Retail Titles** | Modern GDK Releases | 🚧 | Booting, menus, settings, intro movies | Real 3D scene loading; see telemetry analysis |
| **Fortnite** | `436609B6.FortniteClient` 2.0.1461.7919 | ✅ | Boots to menus | Online-only title (requires epic servers) |
| **XDefiant** | `Ubisoft.XDefiant-Game` 1.0.0.186 | ✅ | Boots to menus | Game servers shut down by publisher |

---

## 🚀 Running on macOS (CrossOver / Wine)

### Prerequisites
1. **Apple Silicon Mac:** M1, M2, M3, M4, or M5 (Pro/Max/Ultra recommended for large titles).
2. **CrossOver 24+** (or Wine-Crossover / Whisky) with **D3DMetal (GPTK 2+)** enabled.
3. **Windows 10/11 64-bit Bottle.**
4. **DXC (DirectXShaderCompiler 1.8+):** Place `dxcompiler.dll` and `dxil.dll` into the game directory or configure `XCL_DXC_DIR`.
5. **D3D12 Agility SDK:** Place `D3D12Core.dll` into a `D3D12\` folder next to `xcl.exe` or configure `XCL_D3D12_DIR`.

### Pre-Built Binaries
Download the latest `xcl-mac-wine-x86_64` build directly from the **[GitHub Actions Releases](https://github.com/utkarshxdev/xcl-mac/actions/workflows/mac-cross.yml)**.

### Launch Command
Launch your title through your Wine environment with the recommended Mac environment flags:

```bash
# Recommended launch command inside your bottle:
env XCL_TEXTURE_SKIP_UNCHANGED=0 XCL_NETWORK=0 wine64 xcl.exe run "C:\Games\YourTitle"
```

### Environment Variable Reference

| Variable | Default (Mac) | Default (Win) | Purpose |
| :--- | :---: | :---: | :--- |
| `XCL_TEXTURE_SKIP_UNCHANGED` | `0` | `1` | Skips repeat uploads when a texture's memory matches its last upload |
| `XCL_NETWORK` | `0` (Off) | `1` (On) | Isolates GDK telemetry to offline stubs (prevents hangs) |
| `XCL_DXC_HEAP` | `1` (On) | `0` (Off) | Gives each DXC compilation worker an isolated thread heap |
| `XCL_MASKED_STORES` | `1` (On) | `0` (Off) | Rewrites AVX masked stores to safe Rosetta 2 helper calls |
| `XCL_WATCHED_FALLBACK` | `0` (Off) | `1` (On) | Disables write-watch on fallback allocations outside windows |
| `XCL_COHERENT_HEAPS` | `0` (Off) | `1` (On) | Avoids write-watch page protection traps on coherent heaps |
| `XCL_WATCHDOG_BYPASS` | Off | — | Enables recovery for recognized watchdog sites. Unknown traps still follow normal exception handling. |
| `XCL_DSTORAGE_BCPACK_ACCEPT` | Off | — | Set to `1` to complete eligible BCPack requests with solid placeholder blocks. Texture decompression is required for the original pixels. |
| `XCL_FRAME_DUMP` | `0` | `0` | Dumps every Nth presented frame to disk with luminance telemetry |
| `XCL_FRAME_DUMP_FROM` | `0` | `0` | Delays frame sampling until frame index N is reached |

---

## 🛠️ Building from Source on macOS

You can cross-compile `xcl.exe` directly on macOS for the Wine target (`x86_64-pc-windows-gnullvm`) using LLVM-MinGW:

```bash
# 1. Clone repository
git clone https://github.com/utkarshxdev/xcl-mac.git
cd xcl-mac

# 2. Add Rust Windows target
rustup target add x86_64-pc-windows-gnullvm

# 3. Download LLVM-MinGW toolchain (macOS universal)
curl -fsSL "https://github.com/mstorsjo/llvm-mingw/releases/download/20261117/llvm-mingw-20261117-ucrt-macos-universal.tar.xz" -o /tmp/llvm-mingw.tar.xz
tar -C /tmp -xf /tmp/llvm-mingw.tar.xz
export PATH="/tmp/llvm-mingw-20261117-ucrt-macos-universal/bin:$PATH"
export CARGO_TARGET_X86_64_PC_WINDOWS_GNULLVM_LINKER=x86_64-w64-mingw32-clang

# 4. Verify all 22 Mac subsystems
./scripts/verify_mac_build.sh

# 5. Build release binary
cargo build --release --locked --target x86_64-pc-windows-gnullvm -p xcl

# Binary is ready at: target/x86_64-pc-windows-gnullvm/release/xcl.exe
```

---

## 🧪 Subsystem Verification

We provide an automated verification suite that audits all 22 Mac-native architectural subsystems:

```bash
./scripts/verify_mac_build.sh
```

```
======================================================================
          XCL Mac Native Build Subsystem Verification                 
======================================================================
Checking: D3DMetal COM vtable prefix mirroring (PREFIX = 16)           [PASS]
Checking: Extended vtable test fixture (test_ext_table)                [PASS]
Checking: Stand-in PS dual-source blending (SV_Target0 + SV_Target1)   [PASS]
Checking: D3DMetal shader refusal host fallback (refused_on_mac_host)  [PASS]
Checking: Compute blob cache charging fix (insert_compute_blob)        [PASS]
Checking: Rosetta missing AMD instruction handlers (CLZERO, INSERTQ)   [PASS]
Checking: Rosetta AVX masked store decoder module (maskmov.rs)         [PASS]
Checking: Rosetta masked store fault helper patching                   [PASS]
Checking: Rosetta CPU detection (util::rosetta)                        [PASS]
Checking: UMA Shadowed fallback implementation                         [PASS]
Checking: UMA note_copy_into for readback buffers                      [PASS]
Checking: GPU-visible fallback write-watch disable                     [PASS]
Checking: Command queue #31 GPU view page mappings (gpu_views)         [PASS]
Checking: Unwatched title textures content comparison (poll_unwatched) [PASS]
Checking: Unchanged title texture upload skip override                 [PASS]
Checking: CopyMemoryToMemoryX CPU fallback for non-UMA ranges          [PASS]
Checking: D3DMetal depth-only format mapping                           [PASS]
Checking: Mac host utility detection (util::mac_host)                  [PASS]
Checking: Wine isolated thread heap for DXC (own_heaps)                [PASS]
Checking: Network isolation default off (netoff)                       [PASS]
Checking: 16 MiB title stacks under Wine                               [PASS]
Checking: Mac cross-build CI workflow (.github/workflows/mac-cross.yml) [PASS]
======================================================================
Verification Summary: 22 Passed, 0 Failed
======================================================================
Status: Verification SUCCESSFUL! All Mac host subsystems present.
```

---

## 📜 Credits & License

- **Upstream Project:** [XCL](https://github.com/ivall/xcl) by `ivall`.
- **Mac Research & Patches:** Ahmet Dedeler ([@Ahmet-Dedeler](https://github.com/Ahmet-Dedeler)), Michael McCrimmons ([@mikebuckets171](https://github.com/mikebuckets171)), and community contributors.
- **License:** **GPL-2.0-only** (`LICENSE`). See `THIRD_PARTY.md` for third-party acknowledgments.
- **Notice:** XCL is an independent compatibility layer and contains no proprietary code, SDK binaries, or keys. It operates exclusively on unpacked game files that you legally own.
