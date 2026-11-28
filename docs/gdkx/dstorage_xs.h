/************************************************************************
*                                                                       *
*   dstorage_xs.h -- This module defines the Xbox DirectStorage APIs    *
*                                                                       *
*   Copyright (c) Microsoft Corp. All rights reserved.                  *
*                                                                       *
************************************************************************/

#if !defined(__cplusplus)
    #error C++11 required
#endif

#pragma once

#pragma warning(push)
#pragma warning(disable:4201)   // nonstandard extension used : nameless struct/union

#pragma pack(push, 16)

#include <unknwn.h>
#include <dstorageerr.h>

interface ID3D12Fence;
interface IDStorageStatusArrayX;

/// <summary>
/// The priority of a DStorage queue.
/// </summary>
enum DSTORAGE_PRIORITY : INT8 {
    DSTORAGE_PRIORITY_LOW      = -1,
    DSTORAGE_PRIORITY_NORMAL   = 0,
    DSTORAGE_PRIORITY_HIGH     = 1,
    DSTORAGE_PRIORITY_REALTIME = 2,

    /// <summary>
    /// The following values can be used for iterating over all priority levels.
    /// </summary>
    DSTORAGE_PRIORITY_FIRST    = DSTORAGE_PRIORITY_LOW,
    DSTORAGE_PRIORITY_LAST     = DSTORAGE_PRIORITY_REALTIME,

    DSTORAGE_PRIORITY_COUNT    = 4
};

#define DSTORAGE_MIN_QUEUE_CAPACITY             0x80
#define DSTORAGE_MAX_QUEUE_CAPACITY             0x2000
#define DSTORAGE_MAX_DESTINATION_SIZE           0x2000000

/// <summary>
/// The source type of a DStorage request.
/// </summary>
enum DSTORAGE_REQUEST_SOURCE_TYPE : UINT64 {
    /// <summary>
    /// The source of the DStorage request is a file.
    /// </summary>
    DSTORAGE_REQUEST_SOURCE_FILE = 0,

    /// <summary>
    /// The source of the DStorage request is a block of memory.
    /// </summary>
    DSTORAGE_REQUEST_SOURCE_MEMORY = 1,
};

/// <summary>
/// The DSTORAGE_QUEUE_DESC structure contains the properties of a DStorage
/// queue for the queue's creation.
/// </summary>
struct DSTORAGE_QUEUE_DESC {
    /// <summary>
    /// The source type of requests this DStorage queue can accept.
    /// </summary>
    DSTORAGE_REQUEST_SOURCE_TYPE SourceType;

    /// <summary>
    /// The maximum number of requests the queue can hold.
    /// </summary>
    UINT16 Capacity;

    /// <summary>
    /// The priority of the requests in this queue.
    /// </summary>
    DSTORAGE_PRIORITY Priority;

    /// <summary>
    /// Optional name of the queue. Used for debugging.
    /// </summary>
    _In_opt_z_ const CHAR *Name;
};

/// <summary>
/// The DSTORAGE_QUEUE_OPTIONS structure contains additional flags that are used
/// to control the behaviour of the Queue during creation.
/// </summary>
union DSTORAGE_QUEUE_OPTIONS {
    struct {
        /// <summary>
        /// Boolean value indicating whether auto submit is disabled.
        /// </summary>
        UINT64 AutoSubmitDisabled : 1;

        /// <summary>
        /// Reserved fields. Must be 0.
        /// </summary>
        UINT64 Reserved : 63;
    };
    UINT64 AsUINT64;
};

/// <summary>
/// The DSTORAGE_QUEUE_DESC1 structure contains the properties of a DStorage
/// queue for the queue's creation with the addition of an auto submit flag.
/// </summary>
struct DSTORAGE_QUEUE_DESC1 {
    /// <summary>
    /// The source type of requests this DStorage queue can accept.
    /// </summary>
    DSTORAGE_REQUEST_SOURCE_TYPE SourceType;

    /// <summary>
    /// The maximum number of requests the queue can hold.
    /// </summary>
    UINT16 Capacity;

    /// <summary>
    /// The priority of the requests in this queue.
    /// </summary>
    DSTORAGE_PRIORITY Priority;

    /// <summary>
    /// Optional name of the queue. Used for debugging.
    /// </summary>
    _In_opt_z_ const CHAR *Name;

    /// <summary>
    /// Additional flags that are used to control the behaviour of the Queue
    /// </summary>
    DSTORAGE_QUEUE_OPTIONS Options;
};

/// <summary>
/// The DSTORAGE_QUEUE_INFO structure contains the properties and current state
/// of a DStorage queue.
/// </summary>
struct DSTORAGE_QUEUE_INFO {
    /// <summary>
    /// The DSTORAGE_QUEUE_DESC structure used for queue's creation.
    /// </summary>
    DSTORAGE_QUEUE_DESC Desc;

    /// <summary>
    /// The number of available empty slots. If a queue is empty, the number
    /// of empty slots equals capacity - 1. The reserved slot is used to
    /// distinguish between empty and full cases.
    /// </summary>
    UINT16 EmptySlotCount;

    /// <summary>
    /// The number of entries that would need to be enqueued to trigger
    /// automatic submission.
    /// </summary>
    UINT16 RequestCountUntilAutoSubmit;
};

/// <summary>
/// The DSTORAGE_QUEUE_INFO1 structure contains the properties and current state
/// of a DStorage queue including the auto submit flag status.
/// </summary>
struct DSTORAGE_QUEUE_INFO1 {
    /// <summary>
    /// The DSTORAGE_QUEUE_DESC1 structure used for queue's creation.
    /// </summary>
    DSTORAGE_QUEUE_DESC1 Desc;

    /// <summary>
    /// The number of available empty slots. If a queue is empty, the number
    /// of empty slots equals capacity - 1. The reserved slot is used to
    /// distinguish between empty and full cases.
    /// </summary>
    UINT16 EmptySlotCount;

    /// <summary>
    /// The number of entries that would need to be enqueued to trigger
    /// automatic submission.
    /// </summary>
    UINT16 RequestCountUntilAutoSubmit;
};

/// <summary>
/// The type of BCPACK decompression to perform after the content is read from
/// the file.
/// </summary>
enum DSTORAGE_BCPACK_MODE : UINT64 {
    /// <summary>
    /// Perform no BCPACK decompression.
    /// </summary>
    DSTORAGE_BCPACK_MODE_NONE = 0,

    /// <summary>
    /// Perform BCPACK decompression of BC1 texture data.
    /// </summary>
    DSTORAGE_BCPACK_MODE_BC1  = 1,

    /// <summary>
    /// Perform BCPACK decompression of BC2 texture data.
    /// </summary>
    DSTORAGE_BCPACK_MODE_BC2  = 2,

    /// <summary>
    /// Perform BCPACK decompression of BC3 texture data.
    /// </summary>
    DSTORAGE_BCPACK_MODE_BC3  = 3,

    /// <summary>
    /// Perform BCPACK decompression of BC4 texture data.
    /// </summary>
    DSTORAGE_BCPACK_MODE_BC4  = 4,

    /// <summary>
    /// Perform BCPACK decompression of BC5 texture data.
    /// </summary>
    DSTORAGE_BCPACK_MODE_BC5  = 5,

    /// <summary>
    /// Perform BCPACK decompression of BC6H texture data.
    /// </summary>
    DSTORAGE_BCPACK_MODE_BC6H = 6,

    /// <summary>
    /// Perform BCPACK decompression of BC7 texture data.
    /// </summary>
    DSTORAGE_BCPACK_MODE_BC7  = 7,
};

/// <summary>
/// The type of texture swizzling to perform after the content is through the
/// decompression stage.
/// </summary>
enum DSTORAGE_SWIZZLE_MODE : UINT64 {
    /// <summary>
    /// Perform no swizzling.
    /// </summary>
    DSTORAGE_SWIZZLE_MODE_NONE = 0,

    /// <summary>
    /// The output is interleaved with a 4KB+4KB+8KB pattern.
    /// </summary>
    DSTORAGE_SWIZZLE_MODE_4K_4K_8K = 1,

    /// <summary>
    /// The output is interleaved with a 4KB+4KB+24KB pattern.
    /// </summary>
    DSTORAGE_SWIZZLE_MODE_4K_4K_24K = 2,

    /// <summary>
    /// The output is interleaved with a 4KB+12KB+4KB+4KB+8KB pattern.
    /// </summary>
    DSTORAGE_SWIZZLE_MODE_4K_12K_4K_4K_8K = 3,

    /// <summary>
    /// The output is interleaved with a 4KB+12KB+4KB+12KB pattern.
    /// </summary>
    DSTORAGE_SWIZZLE_MODE_4K_12K_4K_12K = 4,

    /// <summary>
    /// The valid swizzle mode values are below DSTORAGE_SWIZZLE_MODE_MAX.
    /// </summary>
    DSTORAGE_SWIZZLE_MODE_MAX
};

/// <summary>
/// Options for a DStorage request.
/// </summary>
union DSTORAGE_REQUEST_OPTIONS {
    struct {
        /// <summary>
        /// Boolean value indicating whether to perform RFC 1950 decompression.
        /// </summary>
        UINT64 ZlibDecompress : 1;

        /// <summary>
        /// DSTORAGE_BCPACK_MODE value indicating the type of BCPACK
        /// decompression to perform.
        /// </summary>
        DSTORAGE_BCPACK_MODE BcpackMode : 4;

        /// <summary>
        /// DSTORAGE_SWIZZLE_MODE value indicating the type of swizzle operation
        /// to perform.
        /// </summary>
        DSTORAGE_SWIZZLE_MODE SwizzleMode : 16;

        /// <summary>
        /// Boolean value indicating that destination memory is an array of physical pages
        /// allocated through XMemAllocatePhysicalPages.
        /// </summary>
        UINT64 DestinationIsPhysicalPages : 1;

        /// <summary>
        /// DSTORAGE_REQUEST_SOURCE_TYPE enum value indicating whether the
        /// source of the request is a file or a block of memory.
        /// </summary>
        DSTORAGE_REQUEST_SOURCE_TYPE SourceType : 1;

        /// <summary>
        /// When SourceType is DSTORAGE_REQUEST_SOURCE_MEMORY, this boolean
        /// value indicates that source memory is an array of physical pages
        /// allocated through XMemAllocatePhysicalPages.
        /// </summary>
        UINT64 SourceIsPhysicalPages : 1;

        /// <summary>
        /// Reserved fields. Must be 0.
        /// </summary>
        UINT64 Reserved : 40;
    };
    UINT64 AsUINT64;
};

/// <summary>
/// Flags controlling DirectStorage debug layer.
/// </summary>
enum DSTORAGE_DEBUG {
    /// <summary>
    /// DirectStorage debug layer is disabled.
    /// </summary>
    DSTORAGE_DEBUG_NONE                 = 0x00,

    /// <summary>
    /// Print error information to a debugger.
    /// </summary>
    DSTORAGE_DEBUG_SHOW_ERRORS          = 0x01,

    /// <summary>
    /// Trigger a debug break each time an error is detected.
    /// </summary>
    DSTORAGE_DEBUG_BREAK_ON_ERROR       = 0x02,

    /// <summary>
    /// Include IDStorageStatusArrayX and ID3D12Fence names in ETW events.
    /// </summary>
    DSTORAGE_DEBUG_RECORD_OBJECT_NAMES  = 0x04
};

/// <summary>
/// Represents a file to be accessed by DStorage.
/// </summary>
DECLARE_INTERFACE_IID_(IDStorageFileX, IUnknown, "43704f9f-27c6-4a9f-8815-d16527894df4")
{
    /// <summary>
    /// Closes the file, regardless of the reference count on this object.
    ///
    /// After an IDStorageFileX object is closed, it can no longer be used in
    /// DStorage requests.
    /// </summary>
    virtual void Close() = 0;

    /// <summary>
    /// Get a handle to a file represented by this object.
    ///
    /// The handle is opened with GENERIC_READ permissions and FILE_SHARE_READ
    /// sharing mode. It can be used for querying the file's size, etc.
    /// The handle should be closed with CloseHandle() when it's no longer
    /// needed.
    /// </summary>
    virtual HANDLE GetHandle() = 0;
};

/// <summary>
/// Represents a DStorage request.
/// </summary>
struct DSTORAGE_REQUEST {
    /// <summary>
    /// Combination of decompression, swizzle and other options for this request.
    /// </summary>
    DSTORAGE_REQUEST_OPTIONS Options;

    union {
        /// <summary>
        /// Address of the buffer to receive the final result of this request.
        /// </summary>
        _Out_writes_bytes_(DestinationSize) void *Destination;

        struct {
            /// <summary>
            /// Address of the page array to receive the final result of this request if
            /// DestinationIsPhysicalPages is specified. Each entry is a 64KB page number.
            /// </summary>
            PULONG_PTR DestinationPageArray;

            /// <summary>
            /// Offset into the 64KB page where the destination starts, if
            /// DestinationIsPhysicalPages is specified.
            /// </summary>
            UINT16 DestinationPageOffset;
        };
    };

    /// <summary>
    /// <para>Size, in bytes, of the buffer to receive the final result of this
    /// request. A single request must satisfy one of the following size
    /// requirements:</para>
    /// <para>* The DestinationSize is equal to or less than 32MiB</para>
    /// <para>  OR</para>
    /// <para>* The combined memory usage of SourceSize (for memory-to-memory requests only) +
    ///   DestinationSize is equal to or less than 1GiB, and the memory buffers that are
    ///   larger than 32MiB must use either 64KiB pages or 2MiB pages.</para>
    ///</summary>
    UINT32 DestinationSize;

    union {
        struct {
            /// <summary>
            /// The file to perform this read request from, if SourceType is
            /// DSTORAGE_REQUEST_SOURCE_FILE.
            /// </summary>
            IDStorageFileX *File;

            /// <summary>
            /// The offset, in bytes, in the file to start the read request at,
            /// if SourceType is DSTORAGE_REQUEST_SOURCE_FILE.
            /// </summary>
            UINT64 FileOffset;
        };

        /// <summary>
        /// Address of the source buffer to be read from, if SourceType is
        /// DSTORAGE_REQUEST_SOURCE_MEMORY and SourceIsPhysicalPages is not
        /// specified.
        /// </summary>
        _In_reads_bytes_(SourceSize) void *Source;

        struct {
            /// <summary>
            /// Address of the page array to provide the source buffer to be
            /// read from if SourceType is DSTORAGE_REQUEST_SOURCE_MEMORY and
            /// SourceIsPhysicalPages is specified. Each entry is a 64KB page
            /// number.
            /// </summary>
            PULONG_PTR SourcePageArray;

            /// <summary>
            /// Offset into the 64KB page where the source starts, if SourceType
            /// is DSTORAGE_REQUEST_SOURCE_MEMORY and SourceIsPhysicalPages is
            /// specified.
            /// </summary>
            UINT16 SourcePageOffset;
        };
    };

    /// <summary>
    /// Size, in bytes, to be read from the file or memory source.
    /// </summary>
    UINT32 SourceSize;

    /// <summary>
    /// The number of bytes of BCPACK compressed data if ZlibDecompress and BcpackMode are both specified.
    /// </summary>
    UINT32 IntermediateSize;

    /// <summary>
    /// An arbitrary UINT64 number used for cancellation matching.
    /// </summary>
    UINT64 CancellationTag;

    /// <summary>
    /// Optional name of the request. Used for debugging. If specified, the
    /// string should be accessible until the request completes.
    /// </summary>
    _In_opt_z_ const CHAR *Name;
};

#define DSTORAGE_REQUEST_MAX_NAME       64

enum DSTORAGE_COMMAND_TYPE {
    DSTORAGE_COMMAND_TYPE_NONE = -1,
    DSTORAGE_COMMAND_TYPE_REQUEST = 0,
    DSTORAGE_COMMAND_TYPE_STATUS = 1,
    DSTORAGE_COMMAND_TYPE_SIGNAL = 2,
    DSTORAGE_COMMAND_TYPE_EVENT = 3,
};

/// <summary>
/// Structure to receive the detailed record of a failed DStorage request.
/// </summary>
struct DSTORAGE_ERROR_RECORD {
    /// <summary>
    /// The number of failed requests in the queue since the last
    /// RetrieveErrorRecord call.
    /// </summary>
    UINT32 FailureCount;

    /// <summary>
    /// Detailed record about the first failed command in the enqueue order.
    /// </summary>
    struct {

        /// <summary>
        /// The HRESULT code of the failure.
        /// </summary>
        HRESULT HResult;

        /// <summary>
        /// Type of the Enqueue command that caused the failure.
        /// </summary>
        DSTORAGE_COMMAND_TYPE CommandType;

        union {
            /// <summary>
            /// The parameters passed to the EnqueueRequest call.
            /// </summary>
            struct {
                /// <summary>
                /// Combination of decompression, swizzle and other options of the failed
                /// request.
                /// </summary>
                DSTORAGE_REQUEST_OPTIONS Options;

                /// <summary>
                /// Address of the buffer to receive the final result of the failed
                /// request.
                /// </summary>
                void *Destination;

                /// <summary>
                /// Size, in bytes, of the buffer to receive the final result of
                /// the failed request.
                /// </summary>
                UINT32 DestinationSize;

                /// <summary>
                /// For a file source request, the name of the file the request was
                /// targeted to.
                /// </summary>
                WCHAR Filename[MAX_PATH];

                /// <summary>
                /// For a file source request, the offset, in bytes, in the file where
                /// the read request was supposed to start at.
                /// </summary>
                UINT64 FileOffset;

                /// <summary>
                /// For a memory source request, the address of the buffer to be read
                /// from.
                /// </summary>
                void *Source;

                /// <summary>
                /// The number of bytes to be read from the source.
                /// </summary>
                UINT32 SourceSize;

                /// <summary>
                /// An arbitrary UINT64 number used for cancallation matching.
                /// </summary>
                UINT64 CancellationTag;

                /// <summary>
                /// Name of the request.
                /// </summary>
                CHAR Name[DSTORAGE_REQUEST_MAX_NAME];
            } Request;

            /// <summary>
            /// The parameters passed to the EnqueueStatus call.
            /// </summary>
            struct {
                IDStorageStatusArrayX *StatusArray;
                UINT32 Index;
            } Status;

            /// <summary>
            /// The parameters passed to the EnqueueSignal call.
            /// </summary>
            struct {
                ID3D12Fence *Fence;
                UINT64 Value;
            } Signal;

            /// <summary>
            /// The parameters passed to the EnqueueSetEvent call.
            /// </summary>
            struct {
                HANDLE Handle;
            } Event;
        };
    } FirstFailure;
};

/// <summary>
/// Defines the valid staging buffer sizes.
/// </summary>
enum DSTORAGE_STAGING_BUFFER_SIZE : UINT32 {
    DSTORAGE_STAGING_BUFFER_SIZE_0 = 0,
    DSTORAGE_STAGING_BUFFER_SIZE_16MB = 16 * 1048576,
    DSTORAGE_STAGING_BUFFER_SIZE_20MB = 20 * 1048576,
    DSTORAGE_STAGING_BUFFER_SIZE_24MB = 24 * 1048576,
    DSTORAGE_STAGING_BUFFER_SIZE_28MB = 28 * 1048576,
    DSTORAGE_STAGING_BUFFER_SIZE_32MB = 32 * 1048576,
};

/// <summary>
/// Represents per-priority statistics.
/// </summary>
struct DSTORAGE_PRIORITY_STATS {

    /// <summary>
    /// The number of currently open queues.
    /// </summary>
    UINT32 CurrentQueueCount;

    /// <summary>
    /// The highest number of concurrently opened queues.
    /// </summary>
    UINT32 MaxQueueCount;

    /// <summary>
    /// Cumulative number of opened queues.
    /// </summary>
    UINT32 CumulativeQueueCount;

    /// <summary>
    /// The total number of status requests.
    /// </summary>
    UINT32 TotalStatusCount;

    /// <summary>
    /// The total number of fence requests.
    /// </summary>
    UINT32 TotalFenceCount;

    /// <summary>
    /// The number of currently pending requests. A request is considered
    /// to be in a pending state when the DirectStorage subsystem has started
    /// processing it, but the request hasn't been completed yet.
    /// A IDStorageQueueX::Submit() call is a signal for the DirectStorage
    /// to start processing requests in the given queue. There might
    /// be a delay between a IDStorageQueueX::Submit() call and when a request
    /// is considered pending if lower level DirectStorage resources are busy.
    /// </summary>
    UINT32 PendingRequestCount;

    /// <summary>
    /// The total number of submitted requests.
    /// </summary>
    UINT32 TotalRequestCount;

    /// <summary>
    /// The sum of requests' source sizes.
    /// </summary>
    UINT64 TotalRequestSourceByteCount;

    /// <summary>
    /// The sum of requests' destination sizes.
    /// </summary>
    UINT64 TotalRequestDestinationByteCount;
};

/// <summary>
/// Represents statistics.
/// </summary>
struct DSTORAGE_STATS
{
    /// <summary>
    /// Statistics of the low priority queues.
    /// </summary>
    DSTORAGE_PRIORITY_STATS LowPriorityStats;

    /// <summary>
    /// Statistics of the normal priority queues.
    /// </summary>
    DSTORAGE_PRIORITY_STATS NormalPriorityStats;

    /// <summary>
    /// Statistics of the high priority queues.
    /// </summary>
    DSTORAGE_PRIORITY_STATS HighPriorityStats;

    /// <summary>
    /// Statistics of the realtime priority queues.
    /// </summary>
    DSTORAGE_PRIORITY_STATS RealtimePriorityStats;

    /// <summary>
    /// The number of currently open virtual disks (game package, DLC packages,
    /// developer scratch, PLS).
    /// </summary>
    UINT8 CurrentOpenVirtualDiskCount;

    /// <summary>
    /// The highest number of concurrently opened virtual disks.
    /// </summary>
    UINT8 MaxOpenVirtualDiskCount;

    /// <summary>
    /// Cumulative number of opened virtual disks.
    /// </summary>
    UINT32 CumulativeOpenVirtualDiskCount;

    /// <summary>
    /// The number of requests sent by DirectStorage to storage devices that
    /// haven't been completed by the hardware yet.
    /// </summary>
    UINT32 PendingStorageRequestCount;

    /// <summary>
    /// The total number of requests sent by DirectStorage to storage devices.
    /// </summary>
    UINT32 TotalStorageRequestCount;

    /// <summary>
    /// The number of storage device retries.
    /// </summary>
    UINT32 StorageRetries;

    /// <summary>
    /// The number of storage device errors.
    /// </summary>
    UINT32 StorageErrors;

    /// <summary>
    /// The number of currently pending decompression requests.
    /// </summary>
    UINT32 PendingDecompressionRequestCount;

    /// <summary>
    /// The total number of decompression requests.
    /// </summary>
    UINT32 TotalDecompressionRequestCount;

    /// <summary>
    /// The number of decompression errors.
    /// </summary>
    UINT32 DecompressionErrors;

    /// <summary>
    /// The number of stalls caused by a full staging buffer.
    /// </summary>
    UINT32 StagingBufferStallCount;

    /// <summary>
    /// The number of currently open files.
    /// </summary>
    UINT32 CurrentOpenFileCount;

    /// <summary>
    /// The highest number of concurrently opened files.
    /// </summary>
    UINT32 MaxOpenFileCount;

    /// <summary>
    /// Cumulative number of opened files.
    /// </summary>
    UINT32 CumulativeOpenFileCount;

    /// <summary>
    /// The current DirectStorage memory footprint (including
    /// the staging buffer, etc.).
    /// </summary>
    UINT32 CurrentTitleMemoryUsageBytes;

    /// <summary>
    /// The highest DirectStorage memory footprint (including
    /// the staging buffer, etc.).
    /// </summary>
    UINT32 MaxTitleMemoryUsageBytes;

    /// <summary>
    /// The total number of errors.
    /// The error count is incremented every time a request fails for any
    /// reason. Sample errors are: reading data from an invalid offset,
    /// storage device errors, decompression errors, invalid request
    /// parameters, etc.
    /// </summary>
    UINT32 ErrorCount;
};

/// <summary>
/// Represents the static DStorage object used to create DStorage queues, open
/// files for DStorage access, and other global operations.
/// </summary>
DECLARE_INTERFACE_IID_(IDStorageFactoryX, IUnknown, "fb3b54ac-bf82-463a-b777-73195837864d")
{
    /// <summary>
    /// Creates DStorage queue object.
    /// </summary>
    /// <param name="desc">Descriptor to specify the properties of the queue.</param>
    /// <param name="riid">Specifies the DStorage queue interface, such as
    /// __uuidof(IDStorageQueueX).</param>
    /// <param name="ppv">Receives the new queue created.</param>
    /// <returns>Standard HRESULT error code.</returns>
    virtual HRESULT CreateQueue(const DSTORAGE_QUEUE_DESC *desc, REFIID riid, _COM_Outptr_ void **ppv) = 0;

    /// <summary>
    /// Opens a file for DStorage access. The file must be stored on a DStorage
    /// supported NVMe device.
    /// </summary>
    /// <param name="path">Path of the file to be opened.</param>
    /// <param name="riid">Specifies the DStorage file interface, such as
    /// __uuidof(IDStorageFileX).</param>
    /// <param name="ppv">Receives the new file opened.</param>
    /// <returns>Standard HRESULT error code.</returns>
    virtual HRESULT OpenFile(_In_z_ const WCHAR *path, REFIID riid, _COM_Outptr_ void **ppv) = 0;

    /// <summary>
    /// Creates DStorage status array object.
    /// </summary>
    /// <param name="capacity">Specifies the number of status the array can
    /// hold.</param>
    /// <param name="name">Specifies object's name that will appear in
    //  the ETW events if enabled through the debug layer. This is an optional
    //  parameter.</param>
    /// <param name="riid">Specifies the DStorage status interface, such as
    /// __uuidof(IDStorageStatusArrayX).</param>
    /// <param name="ppv">Receives the new status array object created.</param>
    /// <returns>Standard HRESULT error code.</returns>
    virtual HRESULT CreateStatusArray(UINT32 capacity, _In_opt_ PCSTR name, REFIID riid, _COM_Outptr_ void **ppv) = 0;

    /// <summary>
    /// Sets a CPU affinity mask for DStorage. All operations DStorage does that
    /// aren't performed in the caller's stack will be performed in a thread
    /// confined by this affinity mask.
    /// </summary>
    /// <param name="affinity">The CPU affinity mask.</param>
    /// <returns>Standard HRESULT error code.</returns>
    virtual HRESULT SetCpuAffinity(UINT64 affinity) = 0;

    /// <summary>
    /// Sets flags used to control the debug layer.
    /// </summary>
    /// <param name="flags">A set of flags controlling the debug layer.</param>
    virtual void SetDebugFlags(UINT32 flags) = 0;

    /// <summary>
    /// Sets the size of staging buffer used to temporarily store content loaded
    /// from the storage device before they are decrypted / decompressed. If
    /// only memory sourced queues are used, the staging buffer can be 0 sized.
    /// In the current preview release, the only valid staging buffer sizes are
    /// enumerated in the DSTORAGE_STAGING_BUFFER_SIZE enum.
    /// </summary>
    /// <param name="size">Size, in bytes, of the staging buffer.</param>
    virtual HRESULT SetStagingBufferSize(UINT32 size) = 0;
};

/// <summary>
/// Represents the static DStorage object used to create DStorage queues, open
/// files for DStorage access, and other global operations.
///
/// Extends the IDStorageFactoryX interface.
/// </summary>
DECLARE_INTERFACE_IID_(IDStorageFactoryX1, IDStorageFactoryX, "c74c2a5b-1e43-46a5-a224-9116358e1118")
{
    /// <summary>
    /// Gets DirectStorage statistics.
    /// The statistics do not include Win32 file IO.
    /// </summary>
    virtual void GetStats(_Out_ DSTORAGE_STATS *stats) = 0;
};

/// <summary>
/// Represents the static DStorage object used to create DStorage queues, open
/// files for DStorage access, and other global operations.
///
/// Extends the IDStorageFactoryX1 interface.
/// </summary>
DECLARE_INTERFACE_IID_(IDStorageFactoryX2, IDStorageFactoryX1, "62959d11-d45f-4884-a0a3-f7862ea243ae")
{
    /// <summary>
    /// Creates DStorage queue object.
    /// </summary>
    /// <param name="desc">Descriptor to specify the properties of the queue.</param>
    /// <param name="riid">Specifies the DStorage queue interface, such as
    /// __uuidof(IDStorageQueueX).</param>
    /// <param name="ppv">Receives the new queue created.</param>
    /// <returns>Standard HRESULT error code.</returns>
    virtual HRESULT CreateQueue1(const DSTORAGE_QUEUE_DESC1 *desc, REFIID riid, _COM_Outptr_ void **ppv) = 0;
};

/// <summary>
/// Represents an array of status entries to receive completion results for the
/// read requests before them.
/// </summary>
/// <remarks>
/// A status entry receives completion status for all the requests in the
/// DStorageQueue between where it is enqueued and the previously enqueued
/// status entry. Only when all requests enqueued before the status entry
/// complete (ie. IsComplete() for the entry returns true), the status entry
/// can be enqueued again.
/// </remarks>
DECLARE_INTERFACE_IID_(IDStorageStatusArrayX, IUnknown, "cd66d752-2786-44f7-9070-e2a6fdad3f1b")
{
    /// <summary>
    /// Returns a boolean value indicating all requests enqueued prior to the
    /// specified status entry have completed.
    /// </summary>
    /// <param name="index">Specifies the index of the status entry to retrieve.</param>
    /// <returns>Boolean value indicating completion.</returns>
    /// <remarks>This is equivalent to "GetHResult(index) != E_PENDING".</remarks>
    virtual bool IsComplete(UINT32 index) = 0;

    /// <summary>
    /// Returns the HRESULT code of all requests between the specified status
    /// entry and the status entry enqueued before it.
    /// </summary>
    /// <param name="index">Specifies the index of the status entry to retrieve.</param>
    /// <returns>HRESULT code of the requests.</returns>
    /// <remarks>
    /// <list type="bullet">
    /// <item><description>
    /// If any requests have not completed yet, the return value is E_PENDING.
    /// </description></item>
    /// <item><description>
    /// If all requests have completed, and there were failure(s), the return
    /// value stores the failure code of the first failed request in the enqueue
    /// order.
    /// </description></item>
    /// <item><description>
    /// If all requests have completed successfully, the return value is S_OK.
    /// </description></item>
    /// </list>
    /// </remarks>
    virtual HRESULT GetHResult(UINT32 index) = 0;
};

/// <summary>
/// Represents a DStorage queue to perform read operations.
/// </summary>
DECLARE_INTERFACE_IID_(IDStorageQueueX, IUnknown, "a3f6649a-8c2c-4b3a-8b6e-10186a04b9bc")
{
    /// <summary>
    /// Enqueues a read request to the queue. The request remains in the queue
    /// until Submit is called, or until the queue is 1/2 full.
    /// </summary>
    /// <param name="request">The read request to be queued.</param>
    virtual void EnqueueRequest(const DSTORAGE_REQUEST *request) = 0;

    /// <summary>
    /// Enqueues a status write. The status write happens when all requests
    /// before the status write entry complete. If there were failure(s)
    /// since the previous status write entry, the HResult of the enqueued
    /// status entry stores the failure code of the first failed request in the
    /// enqueue order.
    /// </summary>
    /// <param name="statusArray">IDStorageStatusArrayX object.</param>
    /// <param name="index">Index of the status entry in the
    /// IDStorageStatusArrayX object to receive the status.</param>
    virtual void EnqueueStatus(IDStorageStatusArrayX *statusArray, UINT32 index) = 0;

    /// <summary>
    /// Enqueues fence write. The fence write happens when all requests before
    /// the fence entry complete.
    /// </summary>
    /// <param name="fence">An ID3D12Fence to be written.</param>
    /// <param name="value">The value to write to the fence.</param>
    virtual void EnqueueSignal(ID3D12Fence *fence, UINT64 value) = 0;

    /// <summary>
    /// Submits all requests enqueued so far to DStorage to be executed.
    /// </summary>
    virtual void Submit() = 0;

    /// <summary>
    /// Attempts to cancel a group of previously enqueued read requests. All
    /// previously enqueued requests whose CancellationTag matches the formula
    /// (CancellationTag & mask ) == value will be cancelled.
    /// A cancelled request may or may not complete its original read request.
    /// A cancelled request is not counted as a failure in either
    /// IDStorageStatusX or DSTORAGE_ERROR_RECORD.
    /// </summary>
    /// <param name="mask">The mask for the cancellation formula.</param>
    /// <param name="value">The value for the cancellation formula.</param>
    virtual void CancelRequestsWithTag(UINT64 mask, UINT64 value) = 0;

    /// <summary>
    /// Closes the DStorage queue, regardless of the reference count on this
    /// object.
    /// After the Close function returns, the queue will no longer complete any
    /// more requests, even if some are submitted.
    /// </summary>
    virtual void Close() = 0;

    /// <summary>
    /// Obtains an event to wait on. When there is any error happening for read
    /// requests in this queue, the event will be signalled, and
    /// RetrieveErrorRecord may be called to retrieve diagnostic information.
    /// </summary>
    /// <returns>HANDLE to an event.</returns>
    virtual HANDLE GetErrorEvent() = 0;

    /// <summary>
    /// When the error event is signaled, this function can be called to
    /// retrieve a DSTORAGE_ERROR_RECORD. Once the error record is retrieved,
    /// this function should not be called until the next time the error event
    /// is signaled.
    /// </summary>
    /// <param name="record">Receives the error record.</param>
    virtual void RetrieveErrorRecord(_Out_ DSTORAGE_ERROR_RECORD *record) = 0;

    /// <summary>
    /// Obtains information about the queue. It includes the DSTORAGE_QUEUE_DESC
    /// structure used for queue's creation as well as the number of empty slots
    /// and number of entries that need to be enqueued to trigger automatic
    /// submission.
    /// </summary>
    virtual void Query(_Out_ DSTORAGE_QUEUE_INFO *info) = 0;
};

/// <summary>
/// Represents a DStorage queue to perform read operations.
/// </summary>
DECLARE_INTERFACE_IID_(IDStorageQueueX1, IDStorageQueueX, "3c02cb0c-c050-4195-99a1-a1ce94ba33c5")
{
    /// <summary>
    /// Enqueues an operation to set the specified event object to a signaled state.
    /// The event object is set when all requests before it complete.
    /// If there are no free entries in the queue the enqueue operation will
    /// block until one becomes available.
    /// </summary>
    /// <param name="handle">A handle to an event object.</param>
    virtual void STDMETHODCALLTYPE EnqueueSetEvent(HANDLE handle) = 0;
};

/// <summary>
/// Represents a DStorage queue to perform read operations.
/// </summary>
DECLARE_INTERFACE_IID_(IDStorageQueueX2, IDStorageQueueX1, "3af5c948-6297-426c-bffc-8f18959fb87b")
{
    /// <summary>
    /// Obtains information about the queue. It includes the DSTORAGE_QUEUE_DESC1
    /// structure used for queue's creation as well as the number of empty slots
    /// and number of entries that need to be enqueued to trigger automatic
    /// submission, along with the auto submit flag status.
    /// </summary>
    virtual void Query1(_Out_ DSTORAGE_QUEUE_INFO1 *info) = 0;
};

extern "C" {

/// <summary>
/// Returns the static DStorage factory object used to create DStorage queues,
/// open files for DStorage access, and other global operations.
/// </summary>
/// <param name="riid">Specifies the DStorage factory interface, such as
/// __uuidof(IDStorageFactoryX1)</param>
/// <param name="ppv">Receives the DStorage factory object.</param>
/// <returns>Standard HRESULT error code.</returns>
HRESULT WINAPI DStorageGetFactory(REFIID riid, void **ppv);

}

#pragma pack(pop)
#pragma warning(pop)

