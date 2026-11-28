//-------------------------------------------------------------------------------------
// XBTC.h
//  
// Xbox Texture Compressor (XBTC) archive file manipulation
//
// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//
//-------------------------------------------------------------------------------------

#pragma once

#include <Windows.h>

#ifdef _GAMING_XBOX_SCARLETT
#  include <d3d12_xs.h>
#elif defined(_GAMING_XBOX)
#  error XBTC is not supported on Xbox One
#else
#  include <dxgi.h>
#endif

#include <stdint.h>
#include <stdio.h>

// Exported DLL functions follow this convention
#define XBTCDLLAPI(typename) extern "C" typename __cdecl

//-------------------------------------------------------------------------------------------------
// A unique identifier to register the archive file format.
#define XBTC_ARCHIVE_IDENTIFIER \
    ((uint32_t('X') << 0) | (uint32_t('B') << 8) | (uint32_t('T') << 16) | (uint32_t('C') << 24))

//-------------------------------------------------------------------------------------------------
// The version number matching the runtime to the archive format.
#define XBTC_ARCHIVE_VERSION (3)

//-------------------------------------------------------------------------------------------------
// Helper macros
#define XBTC_MIN(a, b) ((a) < (b) ? (a) : (b))
#define XBTC_MAX(a, b) ((a) > (b) ? (a) : (b))

//-------------------------------------------------------------------------------------------------

enum XBTC_TEX_DIMENSION
{
    XBTC_TEX_DIMENSION_1D,
    XBTC_TEX_DIMENSION_2D,
    XBTC_TEX_DIMENSION_3D,
    XBTC_TEX_DIMENSION_CUBE
};

//-------------------------------------------------------------------------------------------------
// Swizzle patterns and tile granularity.

enum XBTC_TILING_MODE
{
    XBTC_TILING_SCARLETT_LINEAR,        // Texel elements are arranged in row major order, mips are 256B aligned
    XBTC_TILING_SCARLETT_256B,          // Tiles are 256B  (NO mip tail)
    XBTC_TILING_SCARLETT_4KB,           // Tiles are 4KB   (4KB mip tail)
    XBTC_TILING_SCARLETT_64KB,          // Tiles are 64KB  (64KB mip tail)
    XBTC_TILING_STANDARD_SWIZZLE        // D3D Standard Swizzle (64KB tiles)
};

//-------------------------------------------------------------------------------------------------
// Texture compression options.

enum XBTC_COMPRESSION_FLAGS
{
    XBTC_COMPRESSION_NONE             = 0x0,  // Try both codecs and keep the best result
    XBTC_COMPRESSION_DISABLE_DEFLATE  = 0x1,  // Disable the data-generic Deflate codec
    XBTC_COMPRESSION_DISABLE_BCPACK   = 0x2,  // Disable the texture-specific BCPack codec
    XBTC_COMPRESSION_DISABLE_ALL      = 0x3,  // Disable both forms of compression
    XBTC_COMPRESSION_USE_SLOW_DEFLATE = 0x4   // Better compression with slower search
};
DEFINE_ENUM_FLAG_OPERATORS(XBTC_COMPRESSION_FLAGS);

//-------------------------------------------------------------------------------------------------
// Texture streaming options.

enum XBTC_STREAM_GRANULARITY
{
    XBTC_STREAM_GRANULARITY_WHOLE_MIPS  = 0x0,  // Compress whole mips for streaming
    XBTC_STREAM_GRANULARITY_64KB        = 0x1,  // Compress individual pages for streaming (requires 64KB tiling)
    XBTC_STREAM_GRANULARITY_128KB       = 0x2,  // Compress 2x1 (contiguous) pages for streaming (requires 64KB tiling)
    XBTC_STREAM_GRANULARITY_256KB       = 0x3   // Compress 2x2 page clusters for streaming (requires 64KB tiling)
};

//-------------------------------------------------------------------------------------------------
// Archive v.2 format description
// ------------------------------------------------------------------------------------------------
// XBTC_ARCHIVE_HEADER      1   Header
// XBTC_TEXTURE_INFO        n   Texture information for each resource in archive
// XBTC_OFFSET_AND_INDEX    n   Base offset and stream index for each texture's streams
// XBTC_STREAM_INFO         m   Relative offset, stream length, and compression format
// Binary blob              m   Each compressed stream, aligned to 16 bytes

//-------------------------------------------------------------------------------------------------
// First 16 bytes of an archive file.

struct XBTC_ARCHIVE_HEADER
{
    uint32_t    Identifier;             // File identifier; "XBTC" as char[]
    uint32_t    Version         : 8;    // Archive format version
    uint32_t    TextureCount    : 24;   // Number of individual textures in archive
    uint64_t    DataOffset      : 24;   // Header + Index size in bytes
    uint64_t    DataSize        : 40;   // Size of all data streams in bytes
};
static_assert(sizeof(XBTC_ARCHIVE_HEADER) == 16, "Unexpected change in size of XBTC_ARCHIVE_HEADER");


//-------------------------------------------------------------------------------------------------
// Description of a texture in the archive.

struct XBTC_TEXTURE_INFO
{
    uint64_t Format      : 8;    // Must not be DXGI_FORMAT_UNKNOWN
    uint64_t Dimension   : 2;    // Texture dimension
    uint64_t Width       : 14;   // Width in pixels of mip 0 (minus 1)
    uint64_t Height      : 14;   // Height in pixels of mip 0 (minus 1)
    uint64_t Depth       : 11;   // Depth or array count (minus 1)
    uint64_t TileMode    : 3;    // Linear, 256B, 4KB, or 64KB
    uint64_t MipCount    : 4;    // Total number of LODs
    uint64_t MipStreams  : 4;    // Number of additional LODs that can be streamed after initial load
    uint64_t Granularity : 2;    // One of XBTC_STREAM_GRANULARITY
    uint64_t Reserved    : 2;
};
static_assert(sizeof(XBTC_TEXTURE_INFO) == 8, "Unexpected change in size of XBTC_TEXTURE_INFO");

//-------------------------------------------------------------------------------------------------
// Description of a buffer in the archive.  This structure aliases with XBTC_TEXTURE_INFO.  When
// Format is DXGI_FORMAT_UNKNOWN, the information refers to a buffer rather than a texture.
// Buffers need not be tiled or detiled.

struct XBTC_BUFFER_INFO
{
    uint64_t Format      : 8;   // Must be DXGI_FORMAT_UNKNOWN
    uint64_t SizeBytes   : 40;  // Size of buffer in bytes
    uint64_t Reserved    : 16;
};
static_assert(sizeof(XBTC_BUFFER_INFO) == sizeof(XBTC_TEXTURE_INFO), "XBTC_BUFFER_INFO must alias with XBTC_TEXTURE_INFO");


//-------------------------------------------------------------------------------------------------
// Table of contents lookup for a compressed texture.

struct XBTC_OFFSET_AND_INDEX
{
    uint64_t BaseOffset : 40;   // Base offset into data chunk.  Stream offsets are relative to this.
    uint64_t NextIndex  : 24;   // Base stream index of *next* resource.  Can be used to compute stream count and base stream index.
};
static_assert(sizeof(XBTC_OFFSET_AND_INDEX) == 8, "Unexpected change in size of XBTC_OFFSET_AND_INDEX");

//-------------------------------------------------------------------------------------------------
// A description of a compressed stream.

struct XBTC_STREAM_INFO
{
    uint32_t RelativeOffset;    // Stream start offset (relative to BaseOffset)
    uint32_t CompressedSize;    // Length of compressed stream
    uint32_t InflatedSize;      // Size of stream before Deflate
    uint32_t UncompressedSize;  // Uncompressed stream size
};
static_assert(sizeof(XBTC_STREAM_INFO) == 16, "Unexpected change in size of XBTC_STREAM_INFO");

//-------------------------------------------------------------------------------------------------
// Free the memory associated with an XBTC resource

XBTCDLLAPI(void) XBTCFreeResource(class XBTCRefCountedResource* pResource);

//-------------------------------------------------------------------------------------------------
class XBTCRefCountedResource
{
public:

    //-------------------------------------------------------------------------------------------------
    XBTCRefCountedResource() : m_RefCount(1) {}

    //-------------------------------------------------------------------------------------------------
    virtual ~XBTCRefCountedResource() {}

    //-------------------------------------------------------------------------------------------------
    // Increments the refcount and returns the new value.
    ULONG AddRef()
    {
        return ++m_RefCount;
    }

    //-------------------------------------------------------------------------------------------------
    // Decrements the refcount and returns the new value.  If the new value is 0, the object is deleted.
    ULONG Release()
    {
        ULONG NewRefCount = --m_RefCount;
        if (NewRefCount == 0)
        {
            XBTCFreeResource(this);
        }
        return NewRefCount;
    }

private:

    ULONG m_RefCount;
};

//-------------------------------------------------------------------------------------------------
// A container to hold texture data throughout the compression pipeline.

class XBTCTextureContainer : public XBTCRefCountedResource
{
public:

    //-------------------------------------------------------------------------------------------------
    // Get base pointer of texture data
    virtual void*   GetTextureMemory() const = 0;

    //-------------------------------------------------------------------------------------------------
    // Get total size of texture data
    virtual size_t  GetTextureSize() const = 0;

    //-------------------------------------------------------------------------------------------------
    // Get a pointer to the start of a subresource's memory
    virtual void*   GetSubresourceMemory(size_t MipLevel, size_t ArraySlice=0) const = 0;

    //-------------------------------------------------------------------------------------------------
    // Get the memory size of the subresource. This is the same for each array slice.
    virtual size_t  GetSubresourceSize(size_t MipLevel) const = 0;

    //-------------------------------------------------------------------------------------------------
    // Get the row pitch of the subresource. This is the same for each array slice.
    virtual size_t  GetSubresourcePitch(size_t MipLevel) const = 0;
};

//-------------------------------------------------------------------------------------------------
// A generic texture arrangement similar to a DDS file. In memory, subresources are arranged
// compactly with largest resolution mip levels preceding smallest (ascending order w.r.t.
// LOD indices). Texture arrays have contiguous mip chains for reach array element.

XBTCDLLAPI(HRESULT) XBTCCreateLinearTexture(
    _Out_   XBTCTextureContainer**  ppLinearTexture,
    _In_    XBTC_TEX_DIMENSION      Dimension,
    _In_    DXGI_FORMAT             Format,
    _In_    size_t                  Width,
    _In_    size_t                  Height,
    _In_    size_t                  DepthOrArraySize,
    _In_    int8_t                  MipCount);

//-------------------------------------------------------------------------------------------------
// Create a generic data container.  This is equivalent to creating a 1D texture without mipmaps
// but skips the tiling requirement prior to compression.

XBTCDLLAPI(HRESULT) XBTCCreateBuffer(
    _Out_   XBTCTextureContainer**  ppBuffer,
    _In_    SIZE_T                  SizeInBytes
);

//-------------------------------------------------------------------------------------------------
// Options to control the BC encoder

struct XBTC_ENCODING_OPTIONS
{
    XBTC_ENCODING_OPTIONS(
        uint32_t    PowerIter = 4,
        uint32_t    RefineIter = 2,
        uint32_t    ChannelCount = 4,
        uint32_t    Quality = 15,
        uint8_t     BC7Mask = 0xFF)
    {
        PCAPowerIterations =    XBTC_MIN(PowerIter, 15);
        RefinementIterations =  XBTC_MIN(RefineIter, 15);
        ChannelCountMinusOne =  XBTC_MIN(uint32_t(ChannelCount - 1), 3);
        QualityLevel =          XBTC_MIN(Quality, 15);
        BC7ModeMask =           (BC7Mask & 0xFF);
    }

    uint32_t    PCAPowerIterations      : 4;    // Diminishing returns after 4 iterations
    uint32_t    RefinementIterations    : 4;    // Try adjusting endpoints for quality gains
    uint32_t    ChannelCountMinusOne    : 2;    // Number of channels to consider (mainly for BC7)
    uint32_t    BC7ModeMask             : 8;    // Restrict BC7 encoding to these modes
    uint32_t    QualityLevel            : 4;    // Higher quality means less compression

    // Proposed options but not yet in effect
    uint32_t    ContainsColorMap        : 1;    // Reduce error for color data (sRGB / Rec.709)
    uint32_t    ContainsNormalMap       : 1;    // Reduce error for normal data
};

#define XBTC_DEFAULT_ENCODING_OPTIONS XBTC_ENCODING_OPTIONS()

//-------------------------------------------------------------------------------------------------
// Statistics reported by the BC encoder

struct XBTC_ENCODING_STATS
{
    uint64_t    NumSamples;             // Number of accumulated samples (pixels)
    uint32_t    CRC32;                  // 32-bit cyclic redundancy check (hash of encoding)
    float       ErrorRange;             // Maximum theoretical error per channel (e.g. '255')
    float       RMSE[4];                // Root mean square error per channel
    float       PSNR[4];                // Peak signal to noise ratio per channel
    float       MaxError[4];            // Maximum observed error per channel
    float       MaxNormError[4];        // Maximum error norm (length) over all pixels (1-4 dimensions)
    float       EncodingTimeMS;         // GPU encoding time in milliseconds
};

//-------------------------------------------------------------------------------------------------
// Block compress (encode to a BCn format) the source texture. Source texture should be linear.
// Allocates a linear texture for output.

XBTCDLLAPI(HRESULT) XBTCBlockCompressTexture(
    _Out_       XBTCTextureContainer**      ppBCTexture,
    _In_        const XBTCTextureContainer* pSourceTexture,
    _In_        DXGI_FORMAT                 BlockCompressFormat,
    _In_        XBTC_ENCODING_OPTIONS       Options = XBTC_DEFAULT_ENCODING_OPTIONS,
    _Out_opt_   XBTC_ENCODING_STATS*        pStats = nullptr);

//-------------------------------------------------------------------------------------------------
// Creates a tiled texture container and "swizzles" linear memory into it according to TilingMode.

XBTCDLLAPI(HRESULT) XBTCTileTexture(
    _Out_   XBTCTextureContainer**      ppTiledTexture,
    _In_    const XBTCTextureContainer* pLinearTexture,
    _In_    XBTC_TILING_MODE            TilingMode);

//-------------------------------------------------------------------------------------------------
// Creates a linear texture container and "unswizzles" tiled memory into it.

XBTCDLLAPI(HRESULT) XBTCDetileTexture(
    _Out_   XBTCTextureContainer**      ppLinearTexture,
    _In_    const XBTCTextureContainer* pTiledTexture);

//-------------------------------------------------------------------------------------------------
// Compress a single texture stream.  This function allocates a buffer to hold the compressed stream.
// When done with the compressed stream, call XBTCFreeCompressedStream(*ppCompressedStream).
// NOTE: If the stream is incompressible, sets *ppCompressStream to NULL.

XBTCDLLAPI(HRESULT) XBTCCompressStreamAlloc(
    _Out_   size_t*                 pCompressedSize,    // Resultant size after compression
    _Out_   size_t*                 pInflatedSize,      // Size of stream prior to Deflate
    _Out_   void**                  ppCompressedStream,
    _In_reads_bytes_(UncompressedSize)
        const void*                 pUncompressedStream,
    _In_    size_t                  UncompressedSize,   // Size of uncompressed stream
    _In_    DXGI_FORMAT             Format,             // Texture format (for BCPack)
    _In_    XBTC_COMPRESSION_FLAGS  Flags);             // Compression options


//-------------------------------------------------------------------------------------------------
// Frees the memory allocated to hold a compressed stream.

XBTCDLLAPI(void) XBTCFreeCompressedStream(
    _In_    void*                   pCompressedStream);

//-------------------------------------------------------------------------------------------------
// Compress a single texture stream.  If the stream is incompressible, copies the source buffer
// to the destination buffer.  (It will never expand data.)
// This is the non-allocating variant.  Provide a destination buffer at least as large as the source.

XBTCDLLAPI(HRESULT) XBTCCompressStream(
    _Out_   size_t*                 pCompressedSize,    // Resultant size after compression
    _Out_   size_t*                 pInflatedSize,      // Size of stream prior to Deflate
    _Out_writes_bytes_(UncompressedSize)
            void*                   pCompressedStream,
    _In_reads_bytes_(UncompressedSize)
        const void*                 pUncompressedStream,
    _In_    size_t                  UncompressedSize,   // Size of uncompressed stream
    _In_    DXGI_FORMAT             Format,             // Texture format (for BCPack)
    _In_    XBTC_COMPRESSION_FLAGS  Flags);             // Compression options

//-------------------------------------------------------------------------------------------------
// Decompress a single texture stream. Returns E_FAIL if the stream is corrupt.
// Writes to pre-allocated memory.

XBTCDLLAPI(HRESULT) XBTCDecompressStream(
    _Out_writes_bytes_(UncompressedSize)
        void*                       pUncompressedStream,
    _In_reads_bytes_(CompressedSize)
        const void*                 pCompressedStream,
    _In_    size_t                  CompressedSize,     // Size of compressed stream
    _In_    size_t                  InflatedSize,       // Size before Deflate (after BCPack)
    _In_    size_t                  UncompressedSize,   // Size of uncompressed stream
    _In_    DXGI_FORMAT             Format);            // Texture format (for BCPack)

//-------------------------------------------------------------------------------------------------

class XBTCArchive : public XBTCRefCountedResource
{
public:

    //-------------------------------------------------------------------------------------------------
    // Get the number of archived resources.
    virtual uint32_t GetResourceCount() const = 0;

    //-------------------------------------------------------------------------------------------------
    // Get the number of compressed streams.
    virtual uint32_t GetStreamCount() const = 0;

    //-------------------------------------------------------------------------------------------------
    // Access texture information.  Consider null pointers an error (index out of range).
    virtual const XBTC_TEXTURE_INFO* GetTextureInfo(uint32_t ResourceIdx) const = 0;

    //-------------------------------------------------------------------------------------------------
    // Access texture offset and index information.  Consider null pointers an error (index out of range).
    virtual const XBTC_OFFSET_AND_INDEX* GetTextureOffset(uint32_t ResourceIdx) const = 0;

    //-------------------------------------------------------------------------------------------------
    // Access stream information.  Consider null pointers an error (index out of range).
    virtual const XBTC_STREAM_INFO* GetStreamInfo(uint32_t StreamIdx) const = 0;

    //-------------------------------------------------------------------------------------------------
    // Returns the the start address of the resource's compressed streams.
    virtual const uint8_t* GetResourceStreamDataStart(uint32_t ResourceIdx) const = 0;

    //-------------------------------------------------------------------------------------------------
    // Returns the total size of the resource's compressed streams.
    virtual int32_t GetResourceCompressedSize(uint32_t ResourceIdx) const = 0;

    //-------------------------------------------------------------------------------------------------
    // Returns the number of compressed streams associated with the resource.
    virtual int32_t GetResourceStreamCount(uint32_t ResourceIdx) const = 0;

    //-------------------------------------------------------------------------------------------------
    // Returns the first stream index associated with the resource.
    virtual int32_t GetResourceBaseStreamIndex(uint32_t ResourceIdx) const = 0;

    //-------------------------------------------------------------------------------------------------
    // Returns the uncompressed size of the first stream associated with a texture.  This is to be used
    // with streamable textures.
    virtual int32_t GetBaseTextureSize(uint32_t ResourceIdx) const = 0;
};

class XBTCArchiveWriter : public XBTCArchive
{
public:
    //-------------------------------------------------------------------------------------------------
    // Save archive contents to a file. Optionally return the size written.
    virtual HRESULT WriteArchiveToFile(const wchar_t* filePath, size_t* fileSize = nullptr) const = 0;
};

//-------------------------------------------------------------------------------------------------
// Create an empty writeable archive. You may append compressed textures to the archive with the
// following functions.

XBTCDLLAPI(HRESULT) XBTCCreateArchive(
    _Out_   XBTCArchiveWriter**         ppArchive);

//-------------------------------------------------------------------------------------------------
// Statistics related to compressing textures

struct XBTC_COMPRESSION_STATS
{
    size_t  UncompressedSizeBytes;      // Size of texture before compression (may be inflated due to tiling alignment)
    size_t  CompressedSizeBytes;        // Size of texture data after compression
    float   CompressionTime;            // Cumulative CPU time spent compressing on multiple threads
    float   WallTime;                   // Actual end-to-end time spent compressing

    float CompressionRatio() const
    {
        return (float)UncompressedSizeBytes / (float)CompressedSizeBytes;
    }

    float CompressedSizeAsPercent() const
    {
        return (float)CompressedSizeBytes / (float)UncompressedSizeBytes * 100.0f;
    }
};

//-------------------------------------------------------------------------------------------------
// Compress a buffer with arbitrary data and append it to the archive.

XBTCDLLAPI(HRESULT) XBTCCompressBuffer(
    _Inout_     XBTCArchiveWriter* pArchive,
    _Out_opt_   XBTC_COMPRESSION_STATS* pStats,
    _In_        const XBTCTextureContainer* pBufferContainer,
    _In_        XBTC_COMPRESSION_FLAGS      Flags);


//-------------------------------------------------------------------------------------------------
// Compress the source (pre-tiled) texture and append it to the archive.
// 
// Loss of 0 (lossless) will not re-encode a BC texture with rate distortion optimization.
// If the texture is re-encoded, the contents of the pTextureContainer will be overwritten.
// Loss will never be introduced for non-BC textures.

XBTCDLLAPI(HRESULT) XBTCCompressTexture(
    _Inout_     XBTCArchiveWriter*          pArchive,
    _Out_opt_   XBTC_COMPRESSION_STATS*     pStats,
    _In_        const XBTCTextureContainer* pTextureContainer,
    _In_        size_t                      MipStreams,     // Number of streamable LODs after the base texture
    _In_        XBTC_STREAM_GRANULARITY     Granularity,    // Streaming granularity
    _In_        XBTC_COMPRESSION_FLAGS      Flags,
    _In_        size_t                      Loss = 0);      // More loss will improve compression ratios. [0-15]

//-------------------------------------------------------------------------------------------------
// Tile the texture, compress it, and append it to the archive.
// 
// Loss of 0 (lossless) will not re-encode a BC texture with rate distortion optimization.
// If the texture is re-encoded, the contents of the pTextureContainer will be overwritten.
// Loss will never be introduced for non-BC textures.

XBTCDLLAPI(HRESULT) XBTCTileAndCompressTexture(
    _Inout_     XBTCArchiveWriter*      pArchive,
    _Out_opt_   XBTC_COMPRESSION_STATS* pStats,
    _In_    const XBTCTextureContainer* pTextureContainer,
    _In_    XBTC_TILING_MODE            TilingMode,
    _In_    size_t                      MipStreams,     // Number of streamable LODs after the base texture
    _In_    XBTC_STREAM_GRANULARITY     Granularity,    // Streaming granularity
    _In_    XBTC_COMPRESSION_FLAGS      Flags,
    _In_    size_t                      Loss = 0);      // More loss will improve compression ratios. [0-15]

//-------------------------------------------------------------------------------------------------
// Extract a texture from the archive. Writes to pre-allocated tiled texture memory.

XBTCDLLAPI(HRESULT) XBTCDecompressTexture(
    _Out_writes_(UncompressedSize)
            void*                   pTiledMemory,
    _In_    size_t                  UncompressedSize,
    _In_    const XBTCArchive*      pArchive,
    _In_    uint32_t                TextureIdx);

//-------------------------------------------------------------------------------------------------
// Extract a texture from the archive. Allocates a texture container.

XBTCDLLAPI(HRESULT) XBTCDecompressTextureAlloc(
    _Out_   XBTCTextureContainer**  ppTextureContainer,
    _In_    const XBTCArchive*      pArchive,
    _In_    uint32_t                TextureIdx);

//-------------------------------------------------------------------------------------------------
// Extract a texture from the archive and linearizes it. Allocates a texture container.

XBTCDLLAPI(HRESULT) XBTCDecompressAndDetileTextureAlloc(
    _Out_   XBTCTextureContainer**  ppTextureContainer,
    _In_    const XBTCArchive*      pArchive,
    _In_    uint32_t                TextureIdx);


//-------------------------------------------------------------------------------------------------
// Stream indices can be identified either from relative virtual memory addressing in the
// case of paged textures (SFS) or by the mip level in all other cases.  The stream index
// offset should be added to the resource's base stream index to determine the absolute
// stream index.
//
// Note:  Interpret negative values as errors.

//-------------------------------------------------------------------------------------------------
// For SFS, each 64KB/128KB page after the base texture is compressed in order. The base texture is always stream 0.

inline int32_t XBTCGetRelativeStreamIndex(ptrdiff_t OffsetFromBaseAddr, size_t BaseTextureSize, size_t PageSize = 0x10000)
{
    // You shouldn't really be calling this function for the base texture stream
    if (OffsetFromBaseAddr == 0)
        return 0;

    // Subtract the size of the base texture which is not chunked into pages
    OffsetFromBaseAddr -= BaseTextureSize;

    // If the address is not aligned to a page boundry, return an error
    if (OffsetFromBaseAddr & (PageSize - 1))
        return -1;

    return (int32_t)(OffsetFromBaseAddr / PageSize) + 1;
}

//-------------------------------------------------------------------------------------------------
// For mip-streaming textures, the base texture is compressed first and each more resolute
// mip level after that.  Assumes the texture has dimension kTexture2D.

inline int32_t XBTCGetRelativeStreamIndex(uint32_t MipStreams, uint32_t MipLevel)
{
    return (int32_t)(MipStreams - MipLevel);
}

//-------------------------------------------------------------------------------------------------
// This is a helper class for looking up stream indices associated with PRT texture regions (motived
// by Sampler Feedback Streaming.) Use this in conjunction with XBTCArchiveReader to stream compressed
// regions of granularity 64KB, 128KB, or 256KB.

class XBTCStreamingTextureReader
{
public:
    XBTCStreamingTextureReader()
    {
        ZeroMemory(this, sizeof(*this));
    }

    HRESULT PrepareTexture(const XBTCArchive* pArchive, uint32_t ResourceIdx)
    {
        const XBTC_TEXTURE_INFO* TexInfo = pArchive->GetTextureInfo(ResourceIdx);

        // Only valid textures with stream granularity other than whole mips are supported
        if (TexInfo == nullptr ||
            TexInfo->Format == DXGI_FORMAT_UNKNOWN ||
            TexInfo->Granularity == XBTC_STREAM_GRANULARITY_WHOLE_MIPS)
        {
            return E_INVALIDARG;
        }

        const DXGI_FORMAT Format = (DXGI_FORMAT)TexInfo->Format;
        const bool IsBlockCompressed =
            (Format >= DXGI_FORMAT_BC1_TYPELESS && Format <= DXGI_FORMAT_BC5_SNORM) ||
            (Format >= DXGI_FORMAT_BC6H_TYPELESS && Format <= DXGI_FORMAT_BC7_UNORM_SRGB);

        // 3D, 2DArray, and non-BC formats are not yet supported
        if (TexInfo->Depth != 0 || !IsBlockCompressed)
        {
            return E_NOTIMPL;
        }

        const uint32_t ElementSize =
            (Format >= DXGI_FORMAT_BC1_TYPELESS && Format <= DXGI_FORMAT_BC1_UNORM_SRGB) ||
            (Format >= DXGI_FORMAT_BC4_TYPELESS && Format <= DXGI_FORMAT_BC4_SNORM) ? 8 : 16;

        m_BaseStreamIndex = pArchive->GetResourceBaseStreamIndex(ResourceIdx);
        m_MipStreamCount = (uint32_t)TexInfo->MipStreams;

        static const uint32_t s_TileHeights[] = { 4, 16, 64, 256, 256 };
        uint32_t TileWidthInPixels = s_TileHeights[TexInfo->TileMode];
        uint32_t TileHeightInPixels = s_TileHeights[TexInfo->TileMode];
        if (ElementSize < 16)
            TileWidthInPixels *= 2;

        m_ClusterWidthLog2 = TexInfo->Granularity == XBTC_STREAM_GRANULARITY_64KB ? 0 : 1;
        m_ClusterHeightLog2 = TexInfo->Granularity == XBTC_STREAM_GRANULARITY_128KB ? 0 : 1;

        uint32_t StreamIndex = pArchive->GetResourceStreamCount(ResourceIdx);
        uint32_t LevelWidth = (uint32_t)TexInfo->Width + 1;
        uint32_t LevelHeight = (uint32_t)TexInfo->Height + 1;

        // For every mip that is streamable, count how many streams it requires and subtract that memory from the base
        for (size_t mip = 0; mip < m_MipStreamCount; ++mip)
        {
            const uint32_t WidthInPages = (LevelWidth + TileWidthInPixels - 1) / TileWidthInPixels;
            const uint32_t HeightInPages = (LevelHeight + TileHeightInPixels - 1) / TileHeightInPixels;
            const uint32_t StreamsThisMip = (WidthInPages >> m_ClusterWidthLog2) * (HeightInPages >> m_ClusterHeightLog2);

            StreamIndex -= StreamsThisMip;
            LevelWidth = (LevelWidth + 1) / 2;
            LevelHeight = (LevelHeight + 1) / 2;

            m_MipStreams[mip].FirstStreamIndex = StreamIndex;
            m_MipStreams[mip].WidthInPages = WidthInPages;
            m_MipStreams[mip].HeightInPages = HeightInPages;
        }

        return S_OK;
    }

    int32_t GetAbsoluteStreamIndex(size_t MipLevel, uint32_t TileX, uint32_t TileY) const
    {
        // Check for invalid mip level
        if (MipLevel >= m_MipStreamCount)
        {
            return -1;
        }

        const MipDigest& Digest = m_MipStreams[MipLevel];
        const uint32_t NumCols = Digest.WidthInPages >> m_ClusterWidthLog2;
        const uint32_t NumRows = Digest.HeightInPages >> m_ClusterHeightLog2;

        // Check for tile coordinates out of bounds
        if (TileX >= NumCols || TileY >= NumRows)
        {
            return -1;
        }

        const uint32_t AbsoluteStreamIndex = m_BaseStreamIndex + Digest.FirstStreamIndex +
            TileY * NumCols + TileX;

        return (int32_t)AbsoluteStreamIndex;
    }

    // Referring to a specific mip level, access the stream index associated with the memory region starting at the
    // offset relative to the start of the level's base address.
    int32_t GetAbsoluteStreamIndex(size_t MipLevel, ptrdiff_t OffsetFromLevelStart) const
    {
        // Check for invalid mip level or offset
        if (MipLevel >= m_MipStreamCount || (OffsetFromLevelStart & 0xFFFFFFFF0000FFFFull))
        {
            return -1;
        }

        const MipDigest& Digest = m_MipStreams[MipLevel];
        const uint32_t OffsetInPages = (uint32_t)OffsetFromLevelStart >> 16;

        // Check for offset out of bounds for the mip level
        if (OffsetInPages >= Digest.WidthInPages * Digest.HeightInPages)
        {
            return -1;
        }

        const uint32_t TileX = (OffsetInPages % Digest.WidthInPages) >> m_ClusterWidthLog2;
        const uint32_t TileY = (OffsetInPages / Digest.WidthInPages) >> m_ClusterHeightLog2;

        const uint32_t AbsoluteStreamIndex = m_BaseStreamIndex + Digest.FirstStreamIndex +
            TileY * (Digest.WidthInPages >> m_ClusterWidthLog2) + TileX;

        return (int32_t)AbsoluteStreamIndex;
    }

private:
    struct MipDigest
    {
        uint32_t FirstStreamIndex   : 16;
        uint32_t WidthInPages       : 8;    // Width of texture level in 64KB tiles
        uint32_t HeightInPages      : 8;    // Height of texture level in 64KB tiles
    };

    uint32_t m_MipStreamCount : 4;
    uint32_t m_ClusterWidthLog2 : 2;
    uint32_t m_ClusterHeightLog2 : 2;
    uint32_t m_BaseStreamIndex;
    MipDigest m_MipStreams[6];
};

//-------------------------------------------------------------------------------------------------

class XBTCArchiveReader : public XBTCArchive
{
public:

    //-------------------------------------------------------------------------------------------------
    XBTCArchiveReader() : m_InternalMemory(nullptr), m_FilePointer(nullptr) { Close(); }

    //-------------------------------------------------------------------------------------------------
    ~XBTCArchiveReader() { Close(); }

    //-------------------------------------------------------------------------------------------------
    // Tests if the file path is a valid archive and returns the amount of the file to read and
    // open the archive. Use with OpenFromMemory() to handle your own memory management or file I/O.
    static size_t IsValidArchive(const wchar_t* ArchiveFilePath);

    //-------------------------------------------------------------------------------------------------
    // Parse an archive header in memory
    bool OpenFromMemory(const XBTC_ARCHIVE_HEADER* ArchiveMemory);

    //-------------------------------------------------------------------------------------------------
    // Parse an archive header from a file pointer. This allocates memory for the header.
    bool OpenFromFile(const wchar_t* ArchiveFilePath);

    //-------------------------------------------------------------------------------------------------
    // Close the archive reader to allow for reuse or to free memory
    void Close();

    //-------------------------------------------------------------------------------------------------
    // Check if the archive is open and valid
    bool IsValid() const
    {
        return m_ResourceCount != 0;
    }

    //-------------------------------------------------------------------------------------------------
    // Check if the archive is open and valid
    operator bool () const
    {
        return IsValid();
    }

    //-------------------------------------------------------------------------------------------------
    virtual uint32_t GetResourceCount() const override { return m_ResourceCount; }

    //-------------------------------------------------------------------------------------------------
    virtual uint32_t GetStreamCount() const override { return m_StreamCount; }

    //-------------------------------------------------------------------------------------------------
    virtual const XBTC_TEXTURE_INFO* GetTextureInfo(uint32_t ResourceIdx) const override {
        return ResourceIdx < m_ResourceCount ? &m_TextureInfos[ResourceIdx] : nullptr;
    }

    //-------------------------------------------------------------------------------------------------
    virtual const XBTC_OFFSET_AND_INDEX* GetTextureOffset(uint32_t ResourceIdx) const override {
        return ResourceIdx < m_ResourceCount ? &m_BaseLocations[ResourceIdx] : nullptr;
    }

    //-------------------------------------------------------------------------------------------------
    virtual const XBTC_STREAM_INFO* GetStreamInfo(uint32_t StreamIdx) const override {
        return StreamIdx < m_StreamCount ? &m_StreamInfos[StreamIdx] : nullptr;
    }

    //-------------------------------------------------------------------------------------------------
    virtual const uint8_t* GetResourceStreamDataStart(uint32_t ResourceIdx) const override;

    //-------------------------------------------------------------------------------------------------
    virtual int32_t GetResourceCompressedSize(uint32_t ResourceIdx) const override;

    //-------------------------------------------------------------------------------------------------
    virtual int32_t GetResourceStreamCount(uint32_t ResourceIdx) const override;

    //-------------------------------------------------------------------------------------------------
    virtual int32_t GetResourceBaseStreamIndex(uint32_t ResourceIdx) const override;

    //-------------------------------------------------------------------------------------------------
    virtual int32_t GetBaseTextureSize(uint32_t ResourceIdx) const override;

    //-------------------------------------------------------------------------------------------------
    // Returns the offset from the start of the file for the this resource's compressed streams.
    int64_t GetResourceSeekOffset(uint32_t ResourceIdx) const;

    //-------------------------------------------------------------------------------------------------
    // If the archive was opened from a file, returns the file pointer for seeking and reading.
    FILE* GetOpenFile() const { return m_FilePointer; }

private:
    uint32_t                        m_ResourceCount;
    uint32_t                        m_StreamCount;
    uint32_t                        m_DataOffset;           // File seek offset to stream data
    const XBTC_TEXTURE_INFO*        m_TextureInfos;
    const XBTC_OFFSET_AND_INDEX*    m_BaseLocations;
    const XBTC_STREAM_INFO*         m_StreamInfos;
    const uint8_t*                  m_StreamData;
    char*                           m_InternalMemory;      // Memory to hold the archive contents with OpenFromFile()
    FILE*                           m_FilePointer;
};

//-------------------------------------------------------------------------------------------------

inline void XBTCArchiveReader::Close()
{
    m_ResourceCount = 0;
    m_StreamCount = 0;
    m_DataOffset = 0;
    m_TextureInfos = nullptr;
    m_BaseLocations = nullptr;
    m_StreamInfos = nullptr;
    m_StreamData = nullptr;
    if (m_InternalMemory != nullptr)
    {
        free(m_InternalMemory);
        m_InternalMemory = nullptr;
    }
    if (m_FilePointer != nullptr)
    {
        fclose(m_FilePointer);
        m_FilePointer = nullptr;
    }
}

//-------------------------------------------------------------------------------------------------

inline size_t XBTCArchiveReader::IsValidArchive(const wchar_t* archiveFilePath)
{
    XBTC_ARCHIVE_HEADER archive;

    FILE* file;
    if (0 != _wfopen_s(&file, archiveFilePath, L"rb" ))                         // file found?
    {
        return 0;
    }

    if (1 != fread_s(&archive, sizeof(archive), sizeof(archive), 1, file) ||    // read success?
        XBTC_ARCHIVE_IDENTIFIER != archive.Identifier ||                        // proper ID?
        XBTC_ARCHIVE_VERSION != archive.Version)                                // correct version?
    {
        fclose(file);
        return 0;
    }

    fclose(file);

    return archive.DataOffset;
}

//-------------------------------------------------------------------------------------------------

inline bool XBTCArchiveReader::OpenFromMemory(const XBTC_ARCHIVE_HEADER* archive)
{
    if (IsValid() ||                                       // already open?
        XBTC_ARCHIVE_IDENTIFIER != archive->Identifier ||  // proper ID?
        XBTC_ARCHIVE_VERSION != archive->Version)          // correct version?
    {
        return false;
    }

    m_ResourceCount = archive->TextureCount;
    m_TextureInfos = (const XBTC_TEXTURE_INFO*)(archive + 1);
    m_BaseLocations = (const XBTC_OFFSET_AND_INDEX*)(m_TextureInfos + m_ResourceCount);
    m_StreamInfos = (const XBTC_STREAM_INFO*)(m_BaseLocations + m_ResourceCount);
    m_StreamCount = m_BaseLocations[m_ResourceCount - 1].NextIndex;
    m_DataOffset = (uint32_t)archive->DataOffset;
    m_StreamData = (const uint8_t*)archive + m_DataOffset;
    m_FilePointer = nullptr;

    return true;
}

//-------------------------------------------------------------------------------------------------

inline bool XBTCArchiveReader::OpenFromFile(const wchar_t* archiveFilePath)
{
    if (IsValid())                                                              // already open?
        return false;                                   

    XBTC_ARCHIVE_HEADER archive;

    FILE* file;
    if (0 != _wfopen_s(&file, archiveFilePath, L"rb" ))                         // file found?
    {
        return false;
    }

    if (1 != fread_s(&archive, sizeof(archive), sizeof(archive), 1, file) ||    // read success?
        XBTC_ARCHIVE_IDENTIFIER != archive.Identifier ||                        // proper ID?
        XBTC_ARCHIVE_VERSION != archive.Version)                                // correct version?
    {
        fclose(file);
        return false;
    }

    const size_t sizeOfTOC = archive.DataOffset - sizeof(XBTC_ARCHIVE_HEADER);
    m_InternalMemory = (char*)malloc(sizeOfTOC);
    if (!fread_s(m_InternalMemory, sizeOfTOC, sizeOfTOC, 1, file))
    {
        fclose(file);
        free(m_InternalMemory);
        m_InternalMemory = nullptr;
        return false;
    }

    m_ResourceCount = archive.TextureCount;
    m_TextureInfos = (const XBTC_TEXTURE_INFO*)m_InternalMemory;
    m_BaseLocations = (const XBTC_OFFSET_AND_INDEX*)(m_TextureInfos + m_ResourceCount);
    m_StreamInfos = (const XBTC_STREAM_INFO*)(m_BaseLocations + m_ResourceCount);
    m_StreamCount = m_BaseLocations[m_ResourceCount - 1].NextIndex;
    m_DataOffset = (uint32_t)archive.DataOffset;
    m_StreamData = nullptr;
    m_FilePointer = file;

    return true;
}

//-------------------------------------------------------------------------------------------------

inline int32_t XBTCArchiveReader::GetResourceStreamCount(uint32_t ResourceIdx) const
{
    if (ResourceIdx >= m_ResourceCount)
    {
        return -1;
    }
    else if (ResourceIdx == 0)
    {
        return (int32_t)m_BaseLocations[ResourceIdx].NextIndex;
    }
    else
    {
        return (int32_t)m_BaseLocations[ResourceIdx].NextIndex - 
            (int32_t)m_BaseLocations[ResourceIdx-1].NextIndex;
    }
}

//-------------------------------------------------------------------------------------------------

inline int32_t XBTCArchiveReader::GetResourceBaseStreamIndex(uint32_t ResourceIdx) const
{
    if (ResourceIdx == 0)
    {
        return 0;
    }
    else if (ResourceIdx >= m_ResourceCount)
    {
        return -1;
    }
    else
    {
        return (int32_t)m_BaseLocations[ResourceIdx-1].NextIndex;
    }
}

//-------------------------------------------------------------------------------------------------

inline int32_t XBTCArchiveReader::GetBaseTextureSize(uint32_t ResourceIdx) const
{
    const XBTC_TEXTURE_INFO* TexInfo = GetTextureInfo(ResourceIdx);

    // Invalid resources and buffers do not have a base texture size
    if (TexInfo == nullptr || TexInfo->Format == DXGI_FORMAT_UNKNOWN)
    {
        return -1;
    }
        
    // Textures without separate mip streams have a base size of zero.
    if (TexInfo->MipStreams == 0)
    {
        return 0;
    }

    const int32_t BaseStreamIndex = m_BaseLocations[ResourceIdx-1].NextIndex;

    return (int32_t)GetStreamInfo(BaseStreamIndex)->UncompressedSize;
}

//-------------------------------------------------------------------------------------------------

inline const uint8_t* XBTCArchiveReader::GetResourceStreamDataStart(uint32_t ResourceIdx) const
{
    if (m_StreamData == nullptr || ResourceIdx >= m_ResourceCount)
    {
        return nullptr;
    }
    else
    {
        return m_StreamData + m_BaseLocations[ResourceIdx].BaseOffset;
    }
}

//-------------------------------------------------------------------------------------------------

inline int64_t XBTCArchiveReader::GetResourceSeekOffset(uint32_t ResourceIdx) const
{
    if (ResourceIdx >= m_ResourceCount)
    {
        return -1;
    }
    else
    {
        return m_DataOffset + m_BaseLocations[ResourceIdx].BaseOffset;
    }
}

//-------------------------------------------------------------------------------------------------

inline int32_t XBTCArchiveReader::GetResourceCompressedSize(uint32_t ResourceIdx) const
{
    if (ResourceIdx >= m_ResourceCount)
    {
        return -1;
    }

    const int32_t lastStreamIdx = (ResourceIdx == m_ResourceCount - 1) ?
        m_StreamCount - 1 :
        (int32_t)m_BaseLocations[ResourceIdx].NextIndex - 1;
  
    const XBTC_STREAM_INFO* lastStream = GetStreamInfo(lastStreamIdx);

    return lastStream->RelativeOffset + lastStream->CompressedSize;
}

