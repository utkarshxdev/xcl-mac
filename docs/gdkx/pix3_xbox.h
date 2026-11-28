/*==========================================================================;
 *
 *  Copyright (C) Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       PIX3_xbox.h
 *  Content:    PIX include file
 *              Don't include this file directly - use pix3.h
 *
 ****************************************************************************/

#pragma once

#ifndef _PIX3_XBOX_H_
#define _PIX3_XBOX_H_

#ifdef USE_PIX_MEMORY_TRACE
    #include "PIXMemory.h"
#endif

#include <intrin.h>

// PIXEventsThreadInfo is defined in PIXEventsCommon.h
struct PIXEventsThreadInfo;

static const UINT64 PIXEventsThreadInfoOffset1 = 48;
static const UINT64 PIXEventsThreadInfoOffset2 = 128;

#if defined(__d3d12_xs_h__)
#define USE_PIX_RETAIL
#endif

__forceinline PIXEventsThreadInfo* PIXGetThreadInfo()
{
#ifdef _X86_
    return nullptr;  // Not supported
#else
    return (PIXEventsThreadInfo*)(__readgsqword(PIXEventsThreadInfoOffset1) + PIXEventsThreadInfoOffset2);
#endif
}

#if defined (USE_PIX)
inline void PIXNotifyWakeFromFenceSignal(_In_ HANDLE)
{
    // This is currently not implemented on XBOX
}
extern "C" HRESULT WINAPI PIXSetLinkWorkerThreadAffinityMask(DWORD_PTR);
#else
//extern "C" HRESULT WINAPI PIXSetLinkWorkerThreadAffinityMask(DWORD_PTR) { return S_OK; }
#endif

// These values denote PIX marker event data that was created by the WinPixEventRuntime.
// In early 2023 we revised the PIX marker format and defined a new version number.
#define WINPIX_EVENT_PIX3BLOB_VERSION 2
#define WINPIX_EVENT_PIX3BLOB_V2 6345127 // A number that other applications are unlikely to have used before

#define D3D12_EVENT_METADATA 0 //nothing
#define D3D12_EVENT_METADATA_LEGACY 0
#define PIXGetTimestampCounter() __rdtsc()

#if defined(__d3d12_xs_h__) || defined(__d3d12_x_h__)

// PIX specific error codes for programmatic captures
#define PIX_ERROR_USER_CANCELED                                 0x8abc0072  // User canceled this request
#define PIX_ERROR_CAPTURE_TIMEOUT_WAITING_FOR_GPU_WORK          0x8abc01f5  // Timeout waiting for GPU work to complete.
#define PIX_ERROR_CAPTURE_NO_WRITEBACK_SPACE                    0x8abc01f6  // Insufficent ring space for writeback.
#define PIX_ERROR_CAPTURE_OUT_OF_TOOLING_MEMORY                 0x8abc01fb  // PIX ran out of tooling memory while a capture was in progress. Increase MaximumToolingMemorySizeMb parameter or adjust capture options.
#define PIX_ERROR_CAPTURE_OUT_OF_RESERVED_TOOLING_MEMORY        0x8abc01fc  // PIX couldn't reserve tooling memory. Decrease MaximumToolingMemorySizeMb parameter.

inline void PIXInsertGPUMarkerOnContextForBeginEvent(_In_ ID3D12GraphicsCommandList* commandList, UINT8 eventType, _In_reads_bytes_(size) void* data, UINT size)
{
    commandList->InsertGpuMarker(eventType, data, size);
}

inline void PIXInsertGPUMarkerOnContextForSetMarker(_In_ ID3D12GraphicsCommandList* commandList, UINT8 eventType, _In_reads_bytes_(size) void* data, UINT size)
{
    commandList->InsertGpuMarker(eventType, data, size);
}

inline void PIXInsertGPUMarkerOnContextForEndEvent(_In_ ID3D12GraphicsCommandList* commandList, UINT8 eventType, _In_reads_bytes_(size) void* data, UINT size)
{
    commandList->InsertGpuMarker(eventType, data, size);
}

inline void PIXInsertGPUMarkerOnContextForBeginEvent(_In_ ID3D12CommandQueue* commandQueue, UINT8 eventType, _In_reads_bytes_(size) void* data, UINT size)
{
    commandQueue->InsertGpuMarker(eventType, data, size);
}

inline void PIXInsertGPUMarkerOnContextForSetMarker(_In_ ID3D12CommandQueue* commandQueue, UINT8 eventType, _In_reads_bytes_(size) void* data, UINT size)
{
    commandQueue->InsertGpuMarker(eventType, data, size);
}

inline void PIXInsertGPUMarkerOnContextForEndEvent(_In_ ID3D12CommandQueue* commandQueue, UINT8 eventType, _In_reads_bytes_(size) void* data, UINT size)
{
    commandQueue->InsertGpuMarker(eventType, data, size);
}

inline void PIXInsertTimingMarkerOnContext(_In_ ID3D12GraphicsCommandList* commandList, UINT8 eventType)
{
    commandList->InsertTimingMarker(eventType);
}

inline void PIXInsertTimingMarkerOnContext(_In_ ID3D12CommandQueue* commandQueue, UINT8 eventType)
{
    commandQueue->InsertTimingMarker(eventType);
}

inline void PIXSetGPUMarkerOnContext(_In_ ID3D12XboxDmaCommandList* commandList, _In_reads_bytes_(size) void* data, UINT size)
{
    commandList->SetMarker(D3D12_EVENT_METADATA, data, size);
}

inline void PIXBeginGPUEventOnContext(_In_ ID3D12XboxDmaCommandList* commandList, _In_reads_bytes_(size) void* data, UINT size)
{
    commandList->BeginEvent(D3D12_EVENT_METADATA, data, size);
}

inline void PIXEndGPUEventOnContext(_In_ ID3D12XboxDmaCommandList* commandList)
{
    commandList->EndEvent();
}

inline void PIXInsertGPUMarkerOnContextForBeginEvent(_In_ ID3D12XboxDmaCommandList* commandList, UINT8 eventType, _In_reads_bytes_(size) void* data, UINT size)
{
    commandList->InsertGpuMarker(eventType, data, size);
}

inline void PIXInsertGPUMarkerOnContextForSetMarker(_In_ ID3D12XboxDmaCommandList* commandList, UINT8 eventType, _In_reads_bytes_(size) void* data, UINT size)
{
    commandList->InsertGpuMarker(eventType, data, size);
}

inline void PIXInsertGPUMarkerOnContextForEndEvent(_In_ ID3D12XboxDmaCommandList* commandList, UINT8 eventType, _In_reads_bytes_(size) void* data, UINT size)
{
    commandList->InsertGpuMarker(eventType, data, size);
}

inline void PIXInsertTimingMarkerOnContext(_In_ ID3D12XboxDmaCommandList* commandList, UINT8 eventType)
{
    commandList->InsertTimingMarker(eventType);
}

#endif //__d3d12_xs_h__ or __d3d12_x_h__

// Performance counter configuration and value retrieval.
//
// The Configure PMC calls will tell the ERA OS to permit the rdpmc instruction to execute (from code use the function/macro below) 
// so that counter values can be retrieved, and also set which event each counter will count. Pass an 
// index from the following tables. These counters correspond to the ones show in the system monitor in PIX.
// When a title configures the PMCs, Sysmon stops recording and manipulating the counters.
// The correct PMC to read for each event corresponds with the event index number
//
// Configuration of performance counters inside the cores.
// The function sets the PMC events to be the same across all 6 available cores. Note that PMCs count
// independently on each core; you cannot difference PMCs on different cores and expect coherent results.
//
//  0   Reserved
//  1   RetiredSseAvx                  The number of SSE/AVX operations retired  
//  2   DCacheAccesses                 The number of accesses to the L1 data cache for load and store references  
//  3   DCacheMisses                   The number of L1 data cache references which miss the data cache  
//  4   DCacheRefills                  The number of L1 data cache refills satisfied from the L2 cache (and/or the northbridge) from uncacheable memory  
//  5   L1DTLBMissL2DTLBHit            The number of L1 data cache accesses that miss in the L1 DTLB and hit in the L2 DTLB   
//  6   DtlbMiss                       The number of L1 data cache accesses that miss in both the L1 and L2 DTLBs  
//  7   L1DtlbHit                      The number of L1 data cache accesses that hit in the L1 DTLB  
//  8   L1ITLBMissL2ITLBHit            The number of instruction fetches that miss in the L1 ITLB but hit in the L2 ITLB  
//  9   ItlbMiss                       The number of instruction fetches that miss in the 4K ITLB and 2M ITLB  
// 10   ItlbInstructionFetchHits       The number of instruction fetches that hit in the 4K ITLB and 2M ITLB  
// 11   MisalignedAccess               The number of L1 data cache accesses that are misaligned. Misaligned accesses incur at least an extra cache access and an extra cycle of latency on reads  
// 12   IneffectiveSWPrefetches        The number of software prefetches that do not cause an actual L1 data cache refill  
// 13   CpuClkNotHalted                The number of clocks that the CPU is not in a halted state  
// 14   RetiredInstructions            The number of instructions retired (execution completed and architectural state updated). This count includes exceptions and interrupts.  
// 15   BranchInstructions             The number of branch instructions retired. This includes all types of architectural control flow changes, including exceptions and interrupts.  
// 16   MispredictedBranch             The number of branch instructions retired, of any type, that were not correctly predicted in either target or direction. This includes those for which prediction is not attempted (far control transfers, exceptions and interrupts), and excludes resyncs. ,
// 17   RetiredTakenBranch             The number of taken branches that were retired. This includes all types of architectural control flow changes, including exceptions and interrupts, and excludes resyncs.  
// 18   RetiredFarSyscall              The number of far syscalls retired  
// 19   RetiredNearReturns             The number of near return instructions retired  
// 20   RetiredReturnsMispredicted     A near return instruction was retired that mispredicted in either target or direction  
// 21   MispredictedTakenBranch        A taken branch instruction was retired that mispredicted in target address (but not in direction)  
// 22   (Depricated) MmxFPInstructions              A floating point (x87, MMX, or SSE) instruction was retired  
// 23   (Deprecated - Use RetiredMmxInstructions instead) RetiredFPInstructions          The number of SSE/AVX operations retired  
// 24   RetiredMmxInstructions         The number of MMX operations retired  
// 25   (Depricated) RetiredSseInstructions         The number of SSE operations retired  
// 26   InstructionFetchStalls         The number cycles that the instruction fetch engine is stalled  
// 27   DataCachePrefetches            L1 data cache prefetches  
// 28   DataCacheReadSize              The number of L1 data cache reads  
// 29   DataCacheWriteSize             The number of L1 data cache writes  
// 30   ITLBReloadStalls               The number of cycles when the fetch engine is stalled for an ITLB reload  
extern "C"
HRESULT WINAPI
ConfigurePMCs(unsigned int eventIndex0, unsigned int eventIndex1, unsigned int eventIndex2, unsigned int eventIndex3);

//
// !!! NOTE: This API is not implemented on Scarlett.
//
// Configuration of performance counters inside the north bridge
// The north bridge has 4 performance counters which can count events.  NB PMC values can be read
// from any core. North bridge memory transactions are 64 bytes.
//
// Events marked with * are recorded from the dram controllers (DCTs).  Set eventIndex6 only 
// when configuring these events since the same event need to be set for each dram controller
//  On Durango, we have 4 dram controllers and 4 counters, so you sum PMC6-PMC9 to get the
//    total result.
//    Eg: total memory traffic for NBAllReadWrites = (PCM6+PMC7+PCM8+PMC9) * 64
//  On Scorpio, we have 12 dram controllers and 4 counters so we only
//    count traffic on every 3rd DCT.  Memory traffic is mostly symmetric so it reasonable to 
//    assume that the total number of events is three times the sum PMC6-PMC9.  
//    Eg: total memory traffic for NBAllReadWrites = (PCM6+PMC7+PCM8+PMC9) * 3 * 64
//
//  0   Reserved
//  1   NBGarlicReads*              The number of garlic bus reads
//  2   NBGarlicWrites*             The number of garlic bus writes
//  3   NBGarlicReadsWrites*        The number of garlic bus reads and writes
//  4   NBOnionReads*               The number of onion bus reads 
//  5   NBOnionWrites*              The number of onion bus writes 
//  6   NBOnionReadsWrites*         The number of onion bus reads and write
//  7   NBAllReadWrites*            The number of all NB reads and writes
//  8   NBCPUToMemory               The number of CPU transactions to memory
//  9   NBIOToMemory                The number of IO transactions to memory
extern "C"
HRESULT WINAPI
ConfigureNBPMCs(unsigned int eventIndex6, unsigned int eventIndex7, unsigned int eventIndex8, unsigned int eventIndex9);

// Configuration of performance counters inside the L2 caches.
// There are two L2 caches and each L2 cache has 4 performance counters which can count events.  
// ConfigureL2IPMCs configures the events of both L2s to be the same.
// The first L2 is tied to cores 0-3.  The second is tied to cores 4-7.  Events for a specific L2 must be read from a core tired to the specific L2. 
// L2 transactions are 64 bytes.
//
//  0   Reserved
//  1   L2MissICFill                The number of IC requests that miss the L2
//  2   L2MissDCFill                The number of DC requests that miss the L2
//  3   L2MissPrefetchFill          The number of prefetch requests that miss the L2
//  4   L2MissAll                   The total number of L2 Misses
//  5   L2Writeback                 The number of L2 clean and dirty writebacks
//  6   L2CleanWriteback            The number of L2 clean writebacks
//  7   L2Fill                      The number of L2 Fills
//  8   L2FillWriteback             The total number of L2 fills and writebacks
//
// NOTE: On Scarlett, this configures the L3 cache counters.
// The L3 cache uses a different counter set:
//  0   Reserved
//  1   L3MissAll                   The total number of L3 misses
extern "C"
HRESULT WINAPI
ConfigureL3PMCs(unsigned int eventIndex10, unsigned int eventIndex11, unsigned int eventIndex12, unsigned int eventIndex13);

// PLEASE READ CAREFULLY!
//
// Retrieving the counter value is wrapped in a try/except block for a good reason.
// The __readpmc() intrinsic emits the rdpmc instruction. On any Xbox One not provisioned as a
// dev kit, this instruction will cause a hard crash unless the exception is handled.
// On a dev kit, rdpmc will cause a hard crash unless the exception is handled OR you
// have called the ConfigurePMCs function below first, which permits using rdpmc safely.
//
// If you choose to take the inline function / macro below to pieces for your own purposes
// and just use __readpmc directly in your own code, remember three things:
// - You are risking a hard crash if you leave rdpmc in your code at cert / release time
// - The code generation for the try/except is far less worrying than you might imagine
// - It will be a Very Good Idea to have an automated mechanism for removing rdpmc from your code
//
// Let's be careful out there.
inline void GetPMCValue(UINT32 index, UINT64* value)
{
    __try
    {
        *value =  __readpmc(index);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        // Either this is not a dev kit, or we have not yet called ConfigurePMCs to permit __readpmc use
        *value = 0; 
    }
}

typedef struct _PIXPowerMetrics {
    float GpuAvgPowerLoadPct;
    float reserved[3];
} PIXPowerMetrics, *PPIXPowerMetrics;

extern "C" HRESULT WINAPI PIXGetPowerMetrics(PPIXPowerMetrics metrics);

enum PIXPerformanceFlag : UINT64
{
    InMenu = 0x1,
    InLevel = 0x2,
    InLobby = 0x4,
    InCredits = 0x8,
    Loading = 0x10,
    Pause = 0x20,
    Constrained = 0x40,
    NetworkUpload = 0x80,
    NetworkDownload = 0x100,
};
 
struct PIXPerformanceClass
{
    PIXPerformanceFlag Flags : 48;      // 48-bits for flags
    UINT64 UserData          : 16;      // 16-bits for caller defined modifiers (i.e. a counter)
};

typedef struct PIXPerformanceClass *PPIXPerformanceClass;

extern "C" void WINAPI PIXSetPerformanceContext(PPIXPerformanceClass performanceClass, PCSTR dimensions[], ULONG32 countOfDimensions);

#endif //_PIX3_XBOX_H_
