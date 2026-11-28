//-------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// XG library for Xbox Next.
//
// XG is an auxiliary graphics library that is used for preparing resources and other
// graphics-related tasks that are performed outside the bounds of the Direct3D runtime
// interfaces.
//

#pragma once

#pragma warning(push)

#pragma warning (disable: 4619 4616 5204 4820)

#include <windows.h>

typedef UINT64 XG_GPU_VIRTUAL_ADDRESS;

//--------------------------------------------------------------------------------------------------
// XG GPU page size represents a 64KB page.
#define XG_GPU_PAGE_SIZE_SHIFT (16)
#define XG_GPU_PAGE_SIZE_BYTES (1 << XG_GPU_PAGE_SIZE_SHIFT)
#define XG_GPU_PAGE_SIZE_MASK (XG_GPU_PAGE_SIZE_BYTES - 1)

enum XG_SWIZZLE_MODE
{
    XG_SWIZZLE_MODE_LINEAR   = 0x00000000,
    XG_SWIZZLE_MODE_256B_S   = 0x00000001,
    XG_SWIZZLE_MODE_256B_D   = 0x00000002,
    XG_SWIZZLE_MODE_256B_R   = 0x00000003,
    XG_SWIZZLE_MODE_4KB_Z    = 0x00000004,
    XG_SWIZZLE_MODE_4KB_S    = 0x00000005,
    XG_SWIZZLE_MODE_4KB_D    = 0x00000006,
    XG_SWIZZLE_MODE_4KB_R    = 0x00000007,
    XG_SWIZZLE_MODE_64KB_Z   = 0x00000008,
    XG_SWIZZLE_MODE_64KB_S   = 0x00000009,
    XG_SWIZZLE_MODE_64KB_D   = 0x0000000a,
    XG_SWIZZLE_MODE_64KB_R   = 0x0000000b,
    XG_SWIZZLE_MODE_VAR_Z    = 0x0000000c,
    XG_SWIZZLE_MODE_VAR_S    = 0x0000000d,
    XG_SWIZZLE_MODE_VAR_D    = 0x0000000e,
    XG_SWIZZLE_MODE_VAR_R    = 0x0000000f,
    XG_SWIZZLE_MODE_64KB_Z_T = 0x00000010,
    XG_SWIZZLE_MODE_64KB_S_T = 0x00000011,
    XG_SWIZZLE_MODE_64KB_D_T = 0x00000012,
    XG_SWIZZLE_MODE_64KB_R_T = 0x00000013,
    XG_SWIZZLE_MODE_4KB_Z_X  = 0x00000014,
    XG_SWIZZLE_MODE_4KB_S_X  = 0x00000015,
    XG_SWIZZLE_MODE_4KB_D_X  = 0x00000016,
    XG_SWIZZLE_MODE_4KB_R_X  = 0x00000017,
    XG_SWIZZLE_MODE_64KB_Z_X = 0x00000018,
    XG_SWIZZLE_MODE_64KB_S_X = 0x00000019,
    XG_SWIZZLE_MODE_64KB_D_X = 0x0000001a,
    XG_SWIZZLE_MODE_64KB_R_X = 0x0000001b,
    XG_SWIZZLE_MODE_VAR_Z_X  = 0x0000001c,
    XG_SWIZZLE_MODE_VAR_S_X  = 0x0000001d,
    XG_SWIZZLE_MODE_VAR_D_X  = 0x0000001e,
    XG_SWIZZLE_MODE_VAR_R_X  = 0x0000001f,
    XG_SWIZZLE_MODE_INVALID  = 0xffffffff,
};

typedef enum XG_TEXTURE_LAYOUT
{
    XG_TEXTURE_LAYOUT_UNKNOWN = 0,
    XG_TEXTURE_LAYOUT_DEFAULT = XG_TEXTURE_LAYOUT_UNKNOWN,
    XG_TEXTURE_LAYOUT_ROW_MAJOR = 1,
    XG_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE = 2,
    XG_TEXTURE_LAYOUT_64KB_STANDARD_SWIZZLE	= 3,

    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_LINEAR   = (0x100 | XG_SWIZZLE_MODE_LINEAR  ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_256B_S   = (0x100 | XG_SWIZZLE_MODE_256B_S  ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_256B_D   = (0x100 | XG_SWIZZLE_MODE_256B_D  ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_256B_R   = (0x100 | XG_SWIZZLE_MODE_256B_R  ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_4KB_Z    = (0x100 | XG_SWIZZLE_MODE_4KB_Z   ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_4KB_S    = (0x100 | XG_SWIZZLE_MODE_4KB_S   ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_4KB_D    = (0x100 | XG_SWIZZLE_MODE_4KB_D   ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_4KB_R    = (0x100 | XG_SWIZZLE_MODE_4KB_R   ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_64KB_Z   = (0x100 | XG_SWIZZLE_MODE_64KB_Z  ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_64KB_S   = (0x100 | XG_SWIZZLE_MODE_64KB_S  ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_64KB_D   = (0x100 | XG_SWIZZLE_MODE_64KB_D  ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_64KB_R   = (0x100 | XG_SWIZZLE_MODE_64KB_R  ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_VAR_Z    = (0x100 | XG_SWIZZLE_MODE_VAR_Z   ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_VAR_S    = (0x100 | XG_SWIZZLE_MODE_VAR_S   ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_VAR_D    = (0x100 | XG_SWIZZLE_MODE_VAR_D   ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_VAR_R    = (0x100 | XG_SWIZZLE_MODE_VAR_R   ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_64KB_Z_T = (0x100 | XG_SWIZZLE_MODE_64KB_Z_T),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_64KB_S_T = (0x100 | XG_SWIZZLE_MODE_64KB_S_T),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_64KB_D_T = (0x100 | XG_SWIZZLE_MODE_64KB_D_T),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_64KB_R_T = (0x100 | XG_SWIZZLE_MODE_64KB_R_T),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_4KB_Z_X  = (0x100 | XG_SWIZZLE_MODE_4KB_Z_X ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_4KB_S_X  = (0x100 | XG_SWIZZLE_MODE_4KB_S_X ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_4KB_D_X  = (0x100 | XG_SWIZZLE_MODE_4KB_D_X ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_4KB_R_X  = (0x100 | XG_SWIZZLE_MODE_4KB_R_X ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_64KB_Z_X = (0x100 | XG_SWIZZLE_MODE_64KB_Z_X),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_64KB_S_X = (0x100 | XG_SWIZZLE_MODE_64KB_S_X),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_64KB_D_X = (0x100 | XG_SWIZZLE_MODE_64KB_D_X),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_64KB_R_X = (0x100 | XG_SWIZZLE_MODE_64KB_R_X),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_VAR_Z_X  = (0x100 | XG_SWIZZLE_MODE_VAR_Z_X ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_VAR_S_X  = (0x100 | XG_SWIZZLE_MODE_VAR_S_X ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_VAR_D_X  = (0x100 | XG_SWIZZLE_MODE_VAR_D_X ),
    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_VAR_R_X  = (0x100 | XG_SWIZZLE_MODE_VAR_R_X ),

    XG_TEXTURE_LAYOUT_SWIZZLE_MODE_MASK = 0x11F,
} XG_TEXTURE_LAYOUT;

inline bool XGConvertSwizzleModeToExtensionTextureLayout(XG_SWIZZLE_MODE SwizzleMode, XG_TEXTURE_LAYOUT* pTextureLayout)
{
    if (SwizzleMode > XG_SWIZZLE_MODE_VAR_R_X)
    {
        return false;
    }
    *pTextureLayout = (XG_TEXTURE_LAYOUT)(XG_TEXTURE_LAYOUT_SWIZZLE_MODE_LINEAR + (UINT)SwizzleMode);
    return true;
}

inline bool XGConvertExtensionTextureLayoutToSwizzleMode(XG_TEXTURE_LAYOUT TextureLayout, XG_SWIZZLE_MODE* pSwizzleMode)
{
    if (TextureLayout < XG_TEXTURE_LAYOUT_SWIZZLE_MODE_LINEAR || TextureLayout > XG_TEXTURE_LAYOUT_SWIZZLE_MODE_VAR_R_X)
    {
        return false;
    }
    *pSwizzleMode = (XG_SWIZZLE_MODE)(TextureLayout & 0x1F);
    return true;
}

//-------------------------------------------------------------------------------------------------
// XG_FORMAT is identical to DXGI_FORMAT on Xbox, and is a superset of DXGI_FORMAT for D3D 11.1 on Windows 8.

enum XG_FORMAT
{
    XG_FORMAT_UNKNOWN                     = 0,
    XG_FORMAT_R32G32B32A32_TYPELESS       = 1,
    XG_FORMAT_R32G32B32A32_FLOAT          = 2,
    XG_FORMAT_R32G32B32A32_UINT           = 3,
    XG_FORMAT_R32G32B32A32_SINT           = 4,
    XG_FORMAT_R32G32B32_TYPELESS          = 5,
    XG_FORMAT_R32G32B32_FLOAT             = 6,
    XG_FORMAT_R32G32B32_UINT              = 7,
    XG_FORMAT_R32G32B32_SINT              = 8,
    XG_FORMAT_R16G16B16A16_TYPELESS       = 9,
    XG_FORMAT_R16G16B16A16_FLOAT          = 10,
    XG_FORMAT_R16G16B16A16_UNORM          = 11,
    XG_FORMAT_R16G16B16A16_UINT           = 12,
    XG_FORMAT_R16G16B16A16_SNORM          = 13,
    XG_FORMAT_R16G16B16A16_SINT           = 14,
    XG_FORMAT_R32G32_TYPELESS             = 15,
    XG_FORMAT_R32G32_FLOAT                = 16,
    XG_FORMAT_R32G32_UINT                 = 17,
    XG_FORMAT_R32G32_SINT                 = 18,
    XG_FORMAT_R32G8X24_TYPELESS           = 19,
    XG_FORMAT_D32_FLOAT_S8X24_UINT        = 20,
    XG_FORMAT_R32_FLOAT_X8X24_TYPELESS    = 21,
    XG_FORMAT_X32_TYPELESS_G8X24_UINT     = 22,
    XG_FORMAT_R10G10B10A2_TYPELESS        = 23,
    XG_FORMAT_R10G10B10A2_UNORM           = 24,
    XG_FORMAT_R10G10B10A2_UINT            = 25,
    XG_FORMAT_R11G11B10_FLOAT             = 26,
    XG_FORMAT_R8G8B8A8_TYPELESS           = 27,
    XG_FORMAT_R8G8B8A8_UNORM              = 28,
    XG_FORMAT_R8G8B8A8_UNORM_SRGB         = 29,
    XG_FORMAT_R8G8B8A8_UINT               = 30,
    XG_FORMAT_R8G8B8A8_SNORM              = 31,
    XG_FORMAT_R8G8B8A8_SINT               = 32,
    XG_FORMAT_R16G16_TYPELESS             = 33,
    XG_FORMAT_R16G16_FLOAT                = 34,
    XG_FORMAT_R16G16_UNORM                = 35,
    XG_FORMAT_R16G16_UINT                 = 36,
    XG_FORMAT_R16G16_SNORM                = 37,
    XG_FORMAT_R16G16_SINT                 = 38,
    XG_FORMAT_R32_TYPELESS                = 39,
    XG_FORMAT_D32_FLOAT                   = 40,
    XG_FORMAT_R32_FLOAT                   = 41,
    XG_FORMAT_R32_UINT                    = 42,
    XG_FORMAT_R32_SINT                    = 43,
    XG_FORMAT_R24G8_TYPELESS              = 44,
    XG_FORMAT_D24_UNORM_S8_UINT           = 45,
    XG_FORMAT_R24_UNORM_X8_TYPELESS       = 46,
    XG_FORMAT_X24_TYPELESS_G8_UINT        = 47,
    XG_FORMAT_R8G8_TYPELESS               = 48,
    XG_FORMAT_R8G8_UNORM                  = 49,
    XG_FORMAT_R8G8_UINT                   = 50,
    XG_FORMAT_R8G8_SNORM                  = 51,
    XG_FORMAT_R8G8_SINT                   = 52,
    XG_FORMAT_R16_TYPELESS                = 53,
    XG_FORMAT_R16_FLOAT                   = 54,
    XG_FORMAT_D16_UNORM                   = 55,
    XG_FORMAT_R16_UNORM                   = 56,
    XG_FORMAT_R16_UINT                    = 57,
    XG_FORMAT_R16_SNORM                   = 58,
    XG_FORMAT_R16_SINT                    = 59,
    XG_FORMAT_R8_TYPELESS                 = 60,
    XG_FORMAT_R8_UNORM                    = 61,
    XG_FORMAT_R8_UINT                     = 62,
    XG_FORMAT_R8_SNORM                    = 63,
    XG_FORMAT_R8_SINT                     = 64,
    XG_FORMAT_A8_UNORM                    = 65,
    XG_FORMAT_R1_UNORM                    = 66,
    XG_FORMAT_R9G9B9E5_SHAREDEXP          = 67,
    XG_FORMAT_R8G8_B8G8_UNORM             = 68,
    XG_FORMAT_G8R8_G8B8_UNORM             = 69,
    XG_FORMAT_BC1_TYPELESS                = 70,
    XG_FORMAT_BC1_UNORM                   = 71,
    XG_FORMAT_BC1_UNORM_SRGB              = 72,
    XG_FORMAT_BC2_TYPELESS                = 73,
    XG_FORMAT_BC2_UNORM                   = 74,
    XG_FORMAT_BC2_UNORM_SRGB              = 75,
    XG_FORMAT_BC3_TYPELESS                = 76,
    XG_FORMAT_BC3_UNORM                   = 77,
    XG_FORMAT_BC3_UNORM_SRGB              = 78,
    XG_FORMAT_BC4_TYPELESS                = 79,
    XG_FORMAT_BC4_UNORM                   = 80,
    XG_FORMAT_BC4_SNORM                   = 81,
    XG_FORMAT_BC5_TYPELESS                = 82,
    XG_FORMAT_BC5_UNORM                   = 83,
    XG_FORMAT_BC5_SNORM                   = 84,
    XG_FORMAT_B5G6R5_UNORM                = 85,
    XG_FORMAT_B5G5R5A1_UNORM              = 86,
    XG_FORMAT_B8G8R8A8_UNORM              = 87,
    XG_FORMAT_B8G8R8X8_UNORM              = 88,
    XG_FORMAT_R10G10B10_XR_BIAS_A2_UNORM  = 89,
    XG_FORMAT_B8G8R8A8_TYPELESS           = 90,
    XG_FORMAT_B8G8R8A8_UNORM_SRGB         = 91,
    XG_FORMAT_B8G8R8X8_TYPELESS           = 92,
    XG_FORMAT_B8G8R8X8_UNORM_SRGB         = 93,
    XG_FORMAT_BC6H_TYPELESS               = 94,
    XG_FORMAT_BC6H_UF16                   = 95,
    XG_FORMAT_BC6H_SF16                   = 96,
    XG_FORMAT_BC7_TYPELESS                = 97,
    XG_FORMAT_BC7_UNORM                   = 98,
    XG_FORMAT_BC7_UNORM_SRGB              = 99,
    XG_FORMAT_AYUV                        = 100,
    XG_FORMAT_Y410                        = 101,
    XG_FORMAT_Y416                        = 102,
    XG_FORMAT_NV12                        = 103,
    XG_FORMAT_P010                        = 104,
    XG_FORMAT_P016                        = 105,
    XG_FORMAT_420_OPAQUE                  = 106,
    XG_FORMAT_YUY2                        = 107,
    XG_FORMAT_Y210                        = 108,
    XG_FORMAT_Y216                        = 109,
    XG_FORMAT_NV11                        = 110,
    XG_FORMAT_AI44                        = 111,
    XG_FORMAT_IA44                        = 112,
    XG_FORMAT_P8                          = 113,
    XG_FORMAT_A8P8                        = 114,
    XG_FORMAT_B4G4R4A4_UNORM              = 115,
    XG_FORMAT_R10G10B10_7E3_A2_FLOAT      = 116,
    XG_FORMAT_R10G10B10_6E4_A2_FLOAT      = 117,
    XG_FORMAT_D16_UNORM_S8_UINT           = 118,
    XG_FORMAT_R16_UNORM_X8_TYPELESS       = 119,
    XG_FORMAT_X16_TYPELESS_G8_UINT        = 120,
    XG_FORMAT_R10G10B10_SNORM_A2_UNORM    = 189,
    XG_FORMAT_R4G4_UNORM                  = 190,
    XG_FORMAT_R24G8_UNORM                 = 191,
    XG_FORMAT_FORCE_UINT                  = 0xffffffff
};

//-------------------------------------------------------------------------------------------------
// XG_RESOURCE_DIMENSION is the same as D3D11_RESOURCE_DIMENSION.

enum XG_RESOURCE_DIMENSION
{
    XG_RESOURCE_DIMENSION_UNKNOWN   = 0,
    XG_RESOURCE_DIMENSION_BUFFER    = 1,
    XG_RESOURCE_DIMENSION_TEXTURE1D = 2,
    XG_RESOURCE_DIMENSION_TEXTURE2D = 3,
    XG_RESOURCE_DIMENSION_TEXTURE3D = 4
};

//-------------------------------------------------------------------------------------------------
// XG_USAGE is the same as D3D11_USAGE.

enum XG_USAGE
{
    XG_USAGE_DEFAULT    = 0,
    XG_USAGE_IMMUTABLE  = 1,
    XG_USAGE_DYNAMIC    = 2,
    XG_USAGE_STAGING    = 3
};

//-------------------------------------------------------------------------------------------------
// XG_PLANE_USAGE is a value returned by the XG texture layout APIs to indicate the purpose of each
// of the 1-4 planes comprising a texture resource.

enum XG_PLANE_USAGE
{
    XG_PLANE_USAGE_UNUSED                  = 0,
    XG_PLANE_USAGE_DEFAULT                 = 1,
    XG_PLANE_USAGE_COLOR_MASK              = 2,
    XG_PLANE_USAGE_FRAGMENT_MASK           = 3,
    XG_PLANE_USAGE_HTILE                   = 4,
    XG_PLANE_USAGE_LUMA                    = 5,
    XG_PLANE_USAGE_CHROMA                  = 6,
    XG_PLANE_USAGE_DEPTH                   = 7,
    XG_PLANE_USAGE_STENCIL                 = 8,
    XG_PLANE_USAGE_DELTA_COLOR_COMPRESSION = 9,
};

//-------------------------------------------------------------------------------------------------
// XG_BIND_FLAG is equivalent to D3D11_BIND_FLAG.

enum XG_BIND_FLAG
{
    XG_BIND_VERTEX_BUFFER    = 0x1L,
    XG_BIND_INDEX_BUFFER     = 0x2L,
    XG_BIND_CONSTANT_BUFFER  = 0x4L,
    XG_BIND_SHADER_RESOURCE  = 0x8L,
    XG_BIND_STREAM_OUTPUT    = 0x10L,
    XG_BIND_RENDER_TARGET    = 0x20L,
    XG_BIND_DEPTH_STENCIL    = 0x40L,
    XG_BIND_UNORDERED_ACCESS = 0x80L,
    XG_BIND_DECODER          = 0x200L,
    XG_BIND_VIDEO_ENCODER    = 0x400L
};
DEFINE_ENUM_FLAG_OPERATORS(XG_BIND_FLAG);

//-------------------------------------------------------------------------------------------------
// XG_CPU_ACCESS_FLAG is equivalent to D3D11_CPU_ACCESS_FLAG.

enum XG_CPU_ACCESS_FLAG
{
    XG_CPU_ACCESS_WRITE = 0x10000L,
    XG_CPU_ACCESS_READ  = 0x20000L
};
DEFINE_ENUM_FLAG_OPERATORS(XG_CPU_ACCESS_FLAG);

//-------------------------------------------------------------------------------------------------
// XG_RESOURCE_MISC_FLAG is a subset of D3D11_RESOURCE_MISC_FLAG.
// The set of flags that exist in XG_RESOURCE_MISC_FLAG match the values of their corresponding
// flags in D3D11_RESOURCE_MISC_FLAG found in d3d11_x.h.

enum XG_RESOURCE_MISC_FLAG
{
    XG_RESOURCE_MISC_GENERATE_MIPS                  = 0x1L,
    XG_RESOURCE_MISC_TEXTURECUBE                    = 0x4L,
    XG_RESOURCE_MISC_DRAWINDIRECT_ARGS              = 0x10L,
    XG_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS         = 0x20L,
    XG_RESOURCE_MISC_BUFFER_STRUCTURED              = 0x40L,
    XG_RESOURCE_MISC_RESOURCE_CLAMP                 = 0x80L,
    XG_RESOURCE_MISC_RESTRICTED_CONTENT             = 0x1000L,
    XG_RESOURCE_MISC_ESRAM_RESIDENT                 = 0x20000L,
    XG_RESOURCE_MISC_NO_COLOR_COMPRESSION           = 0x80000L,
    XG_RESOURCE_MISC_NO_DEPTH_COMPRESSION           = 0x100000L,
    XG_RESOURCE_MISC_NO_STENCIL_COMPRESSION         = 0x200000L,
    XG_RESOURCE_MISC_ALLOW_DCC                      = 0x400000L,
    XG_RESOURCE_MISC_FORCE_TEXTURE_COMPATIBILITY    = 0x8000L,
    XG_RESOURCE_MISC_TILE_POOL                      = 0x4000000L,
    XG_RESOURCE_MISC_TILED                          = 0x8000000L,
};
DEFINE_ENUM_FLAG_OPERATORS(XG_RESOURCE_MISC_FLAG);

//-------------------------------------------------------------------------------------------------
// XG12_RESOURCE_MISC_FLAG is a subset of D3D12_RESOURCE_FLAG from D3D12.XS.
typedef enum XG12_RESOURCE_MISC_FLAG
{
    XG12_RESOURCE_MISC_NONE = 0,
    XG12_RESOURCE_MISC_ALLOW_RENDER_TARGET	= 0x1,
    XG12_RESOURCE_MISC_ALLOW_DEPTH_STENCIL	= 0x2,
    XG12_RESOURCE_MISC_ALLOW_UNORDERED_ACCESS = 0x4,
    XG12_RESOURCE_MISC_DENY_SHADER_RESOURCE = 0x8,
    XG12_RESOURCE_MISC_ALLOW_SIMULTANEOUS_ACCESS = 0x20,
    XG12_RESOURCE_MISC_DENY_COMPRESSION_DATA = 0x300000,
    XG12_RESOURCE_MISC_FLAG_DENY_DEPTH_COMPRESSION_DATA = 0x100000,
    XG12_RESOURCE_MISC_FLAG_DENY_STENCIL_COMPRESSION_DATA = 0x200000,
    XG12_RESOURCE_MISC_ALLOW_DCC = 0x400000,
    XG12_RESOURCE_MISC_FORCE_TEXTURE_COMPATIBILITY = 0x800000,
    XG12_RESOURCE_MISC_FLAG_DENY_DEPTH_STENCIL_HTILE = 0x80000000,
} XG12_RESOURCE_MISC_FLAG;
DEFINE_ENUM_FLAG_OPERATORS(XG12_RESOURCE_MISC_FLAG);

typedef enum XG_HARDWARE_VERSION
{
    XG_HARDWARE_VERSION_XBOX_ONE = 0,
    XG_HARDWARE_VERSION_XBOX_ONE_S = 1,
    XG_HARDWARE_VERSION_XBOX_ONE_X = 2,
    XG_HARDWARE_VERSION_XBOX_ONE_X_DEVKIT = 3,
    XG_HARDWARE_VERSION_XBOX_SCARLETT_LOCKHART = 4,
    XG_HARDWARE_VERSION_XBOX_SCARLETT_ANACONDA = 5,
    XG_HARDWARE_VERSION_XBOX_SCARLETT_DEVKIT = 6,
} XG_HARDWARE_VERSION;

//-------------------------------------------------------------------------------------------------
// XG_SAMPLE_DESC is equivalent to DXGI_SAMPLE_DESC.

struct XG_SAMPLE_DESC
{
    UINT Count;
    UINT Quality;
};

//-------------------------------------------------------------------------------------------------
// XG_BUFFER_DESC is equivalent to D3D11_BUFFER_DESC.

struct XG_BUFFER_DESC
{
    UINT ByteWidth;
    XG_USAGE Usage;
    UINT BindFlags;                 // Any of XG_BIND_FLAG.
    UINT CPUAccessFlags;            // Any of XG_CPU_ACCESS_FLAG.
    UINT MiscFlags;                 // Any of XG_RESOURCE_MISC_FLAG.
    UINT StructureByteStride;
};

//-------------------------------------------------------------------------------------------------
// XG_TEXTURE1D_DESC is a superset of D3D11_TEXTURE1D_DESC, and contains additional parameters used
// to describe a 1D texture for the XG texture layout APIs.

struct XG_TEXTURE1D_DESC
{
    UINT Width;
    UINT MipLevels;
    UINT ArraySize;
    XG_FORMAT Format;
    XG_USAGE Usage;
    UINT BindFlags;                 // Any of XG_BIND_FLAG.
    UINT CPUAccessFlags;            // Any of XG_CPU_ACCESS_FLAG.
    UINT MiscFlags;                 // Any of XG_RESOURCE_MISC_FLAG.
    XG_SWIZZLE_MODE SwizzleMode;
    UINT Pitch;
};

//-------------------------------------------------------------------------------------------------
// XG_TEXTURE2D_DESC is a superset of D3D11_TEXTURE2D_DESC, and contains additional parameters used
// to describe a 2D texture for the XG texture layout APIs.

struct XG_TEXTURE2D_DESC
{
    UINT Width;
    UINT Height;
    UINT MipLevels;
    UINT ArraySize;
    XG_FORMAT Format;
    XG_SAMPLE_DESC SampleDesc;
    XG_USAGE Usage;
    UINT BindFlags;                 // Any of XG_BIND_FLAG.
    UINT CPUAccessFlags;            // Any of XG_CPU_ACCESS_FLAG.
    UINT MiscFlags;                 // Any of XG_RESOURCE_MISC_FLAG.
    XG_SWIZZLE_MODE SwizzleMode;
    UINT Pitch;
};

//-------------------------------------------------------------------------------------------------
// XG_TEXTURE3D_DESC is a superset of D3D11_TEXTURE3D_DESC, and contains additional parameters used
// to describe a 3D texture for the XG texture layout APIs.

struct XG_TEXTURE3D_DESC
{
    UINT Width;
    UINT Height;
    UINT Depth;
    UINT MipLevels;
    XG_FORMAT Format;
    XG_USAGE Usage;
    UINT BindFlags;                 // Any of XG_BIND_FLAG.
    UINT CPUAccessFlags;            // Any of XG_CPU_ACCESS_FLAG.
    UINT MiscFlags;                 // Any of XG_RESOURCE_MISC_FLAG.
    XG_SWIZZLE_MODE SwizzleMode;
    UINT Pitch;
};

//-------------------------------------------------------------------------------------------------
// XG_RESOURCE_DESC is identical to D3D12_RESOURCE_DESC, and can describe a texture of any
// dimension to XG.

typedef struct XG_RESOURCE_DESC
{
    XG_RESOURCE_DIMENSION Dimension;
    UINT64 Alignment;
    UINT64 Width;
    UINT Height;
    UINT16 DepthOrArraySize;
    UINT16 MipLevels;
    XG_FORMAT Format;
    XG_SAMPLE_DESC SampleDesc;
    XG_TEXTURE_LAYOUT Layout;
    XG12_RESOURCE_MISC_FLAG MiscFlags;
} XG_RESOURCE_DESC;


//-------------------------------------------------------------------------------------------------
// XG_MIPLEVEL_LAYOUT describes a single mip level in the hardware layout of a texture.
// Note that each mip level comprises a number of contiguous slices in the case of volume textures only.

struct XG_MIPLEVEL_LAYOUT
{
    UINT64 SizeBytes;
    UINT64 OffsetBytes;
    UINT64 Slice2DSizeBytes;
    UINT32 PitchPixels;
    UINT32 PitchBytes;
    UINT32 AlignmentBytes;
    UINT32 PaddedWidthElements;
    UINT32 PaddedHeightElements;
    UINT32 PaddedDepthOrArraySize;
    UINT32 WidthElements;
    UINT32 HeightElements;
    UINT32 WidthPixels;
    UINT32 HeightPixels;
    UINT32 DepthOrArraySize;
    UINT32 SampleCount;
    XG_SWIZZLE_MODE SwizzleMode;
    UINT64 BankRotationAddressBitMask;
    UINT64 BankRotationBytesPerSlice;
    UINT32 SliceDepthElements;
    UINT32 IsMipTail;

    // If this mip level is a mip tail, and OffsetBytes is 0, then the mip image
    // location within the mip tail is described by these offsets in elements:
    UINT32 MipTailOffsetElementsX : 11;
    UINT32 MipTailOffsetElementsY : 11;
    UINT32 MipTailOffsetElementsZ : 10;
};

//-------------------------------------------------------------------------------------------------
// XG_PLANE_LAYOUT describes a single plane in the hardware layout of a resource.

struct XG_PLANE_LAYOUT
{
    // Usage is the purpose of this plane - whether it's the primary plane (DEFAULT), or a compression metadata surface.
    XG_PLANE_USAGE Usage;

    // SizeBytes is the total size of the plane, including all slices.
    UINT64 SizeBytes;

    // SliceSizeBytes is the size of a single array slice, which is an entire mip chain.
    // For 1D/2D array textures, a slice is always one texel element in depth.
    // For 3D (volume) textures, a slice may be more than one texel element in depth.
    // See SliceDepthElements on the mip level structs.
    UINT64 SliceSizeBytes;

    // BaseOffsetBytes is the offset of this plane from the base address of the resource.
    UINT64 BaseOffsetBytes;

    // BaseAlignmentBytes is the alignment of this plane within the resource.
    UINT64 BaseAlignmentBytes;

    // BytesPerElement is the number of bytes per texel element of all texels in this plane.
    // Note that a BC compression block (4x4 decompressed texels) is one texel element.
    UINT32 BytesPerElement;

    // StandardMipCount is the number of mip levels that are not part of the mip chain.
    UINT32 StandardMipCount;

    // MipTailMipCount is the number of mip levels that are part of the mip chain.
    UINT32 MipTailMipCount;

    // The array of mip level layouts comprising this plane.
    XG_MIPLEVEL_LAYOUT MipLayout[15];
};

//-------------------------------------------------------------------------------------------------
// XG_RESOURCE_LAYOUT describes the entire hardware layout of a single resource.  Certain resource
// configurations include up to 4 planes, each plane being a single mip chain.

struct XG_RESOURCE_LAYOUT
{
    UINT64 SizeBytes;
    UINT64 BaseAlignmentBytes;
    UINT32 MipLevels;
    UINT32 Planes;
    XG_PLANE_LAYOUT Plane[4];
    XG_RESOURCE_DIMENSION Dimension;
};

//--------------------------------------------------------------------------------------------------
typedef struct XG_SUBRESOURCE_TILING
{
    UINT32 WidthInTiles;
    UINT16 HeightInTiles;
    UINT16 DepthInTiles;
    UINT32 StartTileIndexInOverallResource;
} XG_SUBRESOURCE_TILING;

#define	XG_PACKED_TILE	( 0xffffffff )

//--------------------------------------------------------------------------------------------------
typedef struct XG_TILE_SHAPE
{
    UINT32 WidthInTexels;
    UINT32 HeightInTexels;
    UINT32 DepthInTexels;
} XG_TILE_SHAPE;

//--------------------------------------------------------------------------------------------------
typedef struct XG_TILE_SHAPE_ELEMENTS
{
    UINT32 WidthInElements;
    UINT32 HeightInElements;
    UINT32 DepthInElements;
} XG_TILE_SHAPE_ELEMENTS;

//--------------------------------------------------------------------------------------------------
typedef struct XG_PACKED_MIP_DESC
{
    UINT8  NumStandardMips;
    UINT8  NumPackedMips;
    UINT32 NumTilesForPackedMips;
    UINT32 StartTileIndexInOverallResource;
} XG_PACKED_MIP_DESC;

//--------------------------------------------------------------------------------------------------
#define	XG_TILED_RESOURCE_TILE_SIZE_IN_BYTES	( 65536 )

//--------------------------------------------------------------------------------------------------
typedef struct XG_DESCRIPTOR_TEXTURE_VIEW
{
    UINT32 Dword[8];
} XG_DESCRIPTOR_TEXTURE_VIEW;

//-------------------------------------------------------------------------------------------------
typedef struct XG_SWIZZLE_PATTERN_INFO
{
    // Internal: swizzle pattern equation index.
    UINT32 EquationIndex;

    // Dimensions (in elements) of the underlying swizzle block size.
    UINT32 BlockWidth;
    UINT32 BlockHeight;
    UINT32 BlockSlices;
} XG_SWIZZLE_PATTERN_INFO;

//-------------------------------------------------------------------------------------------------
// XGComputeBufferLayout computes the hardware alignment for a buffer.

HRESULT __cdecl XGComputeBufferLayout(
    _In_ const XG_BUFFER_DESC* pDesc,
    _Out_ XG_RESOURCE_LAYOUT* pLayout
);

//-------------------------------------------------------------------------------------------------
// XGComputeTexture1DLayout computes the hardware layout for a 1D texture.

HRESULT __cdecl XGComputeTexture1DLayout(
    _In_  const XG_TEXTURE1D_DESC* pDesc,
    _Out_ XG_RESOURCE_LAYOUT* pLayout
);

//-------------------------------------------------------------------------------------------------
// XGComputeTexture2DLayout computes the hardware layout for a 2D texture.

HRESULT __cdecl XGComputeTexture2DLayout(
    _In_  const XG_TEXTURE2D_DESC* pDesc,
    _Out_ XG_RESOURCE_LAYOUT* pLayout
);

//-------------------------------------------------------------------------------------------------
// XGComputeTexture3DLayout computes the hardware layout for a 3D texture.

HRESULT __cdecl XGComputeTexture3DLayout(
    _In_  const XG_TEXTURE3D_DESC* pDesc,
    _Out_ XG_RESOURCE_LAYOUT* pLayout
);

//-------------------------------------------------------------------------------------------------
// XGSuggestSwizzleMode returns the driver's suggested swizzle mode for the given resource configuration.
// If neither optimize for performance or optimize for memory footprint are specified, the driver's suggestion
// will optimize for performance.
// The Layout field in the pResourceDesc parameter is ignored if it contains one of the extension
// XG_TEXTURE_LAYOUT_SWIZZLE_MODE_* values. Use a Layout value of XG_TEXTURE_LAYOUT_64KB_UNDEFINED_SWIZZLE
// to indicate a reserved/tiled resource.
// The swizzle mode is returned either in XG_SWIZZLE_MODE or XG_TEXTURE_LAYOUT encoding.

typedef enum XG_SUGGEST_SWIZZLE_MODE_FLAGS
{
    XG_SUGGEST_SWIZZLE_MODE_FLAG_NONE = 0,
    XG_SUGGEST_SWIZZLE_MODE_FLAG_OPTIMIZE_FOR_PERFORMANCE = 1,
    XG_SUGGEST_SWIZZLE_MODE_FLAG_OPTIMIZE_FOR_MEMORY_FOOTPRINT = 2,
} XG_SUGGEST_SWIZZLE_MODE_FLAGS;

HRESULT __cdecl XGSuggestSwizzleMode(
    _In_  const XG_RESOURCE_DESC* pResourceDesc,
    _In_  XG_SUGGEST_SWIZZLE_MODE_FLAGS Flags,
    _Out_ XG_TEXTURE_LAYOUT* pTextureLayout
);

//-------------------------------------------------------------------------------------------------
// XGComputeOptimalSwizzleMode computes the bandwidth optimal swizzle mode for the given texture configuration.
// This API is deprecated; please use XGSuggestSwizzleMode instead.

XG_SWIZZLE_MODE __cdecl XGComputeOptimalSwizzleMode(
    _In_  XG_RESOURCE_DIMENSION ResourceDimension,
    _In_  XG_FORMAT Format,
    _In_  UINT32 Width,
    _In_  UINT32 Height,
    _In_  UINT32 DepthOrArraySize,
    _In_  UINT32 SampleCount,
    _In_  UINT BindFlags,                   // Any of XG_BIND_FLAG.
    _In_  UINT MiscFlags = 0                // Any of XG_RESOURCE_MISC_FLAG.
);

//-------------------------------------------------------------------------------------------------
// XGComputeOptimalDepthStencilTileModes computes the correct swizzle mode for the given depth
// stencil configuration.
// This API is deprecated; please use XGSuggestSwizzleMode instead.

VOID __cdecl XGComputeOptimalDepthStencilSwizzleMode(
    _In_  XG_FORMAT Format,
    _In_  UINT32 Width,
    _In_  UINT32 Height,
    _In_  UINT32 DepthOrArraySize,
    _In_  UINT32 SampleCount,
    _In_  BOOL Compressed,
    _In_  BOOL TiledResource,
    _In_  BOOL TextureCompatible,
    _Out_ XG_SWIZZLE_MODE* pDepthSwizzleMode
);

//-------------------------------------------------------------------------------------------------
// XGValidateDepthStencilSwizzleMode returns S_OK if the given tile mode is valid for the depth
// stencil format and sample count, or E_FAIL if the given tile mode is not valid. E_INVALIDARG is
// returned if the format is not a valid depth format or the tile mode is not appropriate for use
// as a depth buffer in any circumstances.

HRESULT __cdecl XGValidateDepthStencilSwizzleMode(
    _In_ XG_FORMAT Format,
    _In_ UINT32 SampleCount,
    _In_ XG_SWIZZLE_MODE SwizzleMode
);

//-------------------------------------------------------------------------------------------------
// XGTextureAddressComputer holds a computed texture layout for a given texture resource, along with
// additional state that allows the user to perform texture element microtiling and macrotiling.
// The class is refcounted and will delete itself when its refcount reaches 0.

class XGTextureAddressComputer
{
public:
    //-------------------------------------------------------------------------------------------------
    // Increments the refcount and returns the new value.
    virtual ULONG __cdecl AddRef() = 0;

    //-------------------------------------------------------------------------------------------------
    // Decrements the refcount and returns the new value.  If the new value is 0, the object is deleted.
    virtual ULONG __cdecl Release() = 0;

    //-------------------------------------------------------------------------------------------------
    // Returns the XG_RESOURCE_LAYOUT that was computed for this texture resource.
    virtual HRESULT __cdecl GetResourceLayout(
        _Out_ XG_RESOURCE_LAYOUT* pLayout
    ) = 0;

    //-------------------------------------------------------------------------------------------------
    // Returns the resource's total size in bytes. Equivalent to calling GetResourceLayout and using
    // the SizeBytes field.
    virtual UINT64 GetResourceSizeBytes() = 0;

    //-------------------------------------------------------------------------------------------------
    // Returns the resource's total size in 64KB pages, rounded up to the next whole page.
    UINT64 GetResourceSizePages() { return ((GetResourceSizeBytes() + XG_GPU_PAGE_SIZE_MASK) >> XG_GPU_PAGE_SIZE_SHIFT); }

    //-------------------------------------------------------------------------------------------------
    // Returns the resource's base alignment in bytes. Equivalent to calling GetResourceLayout and 
    // using the BaseAlignmentBytes field.
    virtual UINT64 GetResourceBaseAlignmentBytes() = 0;

    //-------------------------------------------------------------------------------------------------
    // Computes the offset in bytes of the given mip level on the given plane, from the base address
    // of the resource.
    virtual UINT64 __cdecl GetMipLevelOffsetBytes(
        _In_  UINT32 Plane,
        _In_  UINT32 MipLevel
    ) = 0;

    //-------------------------------------------------------------------------------------------------
    // Computes the offset in bytes of the given texel element, from the base address of the resource.
    virtual UINT64 __cdecl GetTexelElementOffsetBytes(
        _In_      UINT32 Plane,
        _In_      UINT32 MipLevel,
        _In_      UINT64 X,
        _In_      UINT32 Y,
        _In_      UINT32 ZOrSlice,
        _In_      UINT32 Sample,
        _Out_opt_ UINT32 * pBitPosition
    ) = 0;

    //-------------------------------------------------------------------------------------------------
    // Compute texel element coordinate of the given offset to the base address of the resource.
    // Return E_INVALIDARGS, if the offset is not within the memory range of the resource.
    virtual HRESULT __cdecl GetTexelCoordinate(
        _In_      UINT64 Offset,
        _Out_opt_ UINT32 * Plane,
        _Out_opt_ UINT32 * MipLevel,
        _Out_opt_ UINT64 * X,
        _Out_opt_ UINT32 * Y,
        _Out_opt_ UINT32 * ZOrSlice,
        _Out_opt_ UINT32 * Sample
    ) = 0;

    //-------------------------------------------------------------------------------------------------
    // Copies the 2D or 3D linear input data into the given subresource (mip + slice) of the resource,
    // using pTiledResourceBaseAddress as the resource's base address.  For offline texture preparation,
    // pTiledResourceBaseAddress does not need to be properly aligned.
    virtual HRESULT __cdecl CopyIntoSubresource(
        _Inout_ VOID* pTiledResourceBaseAddress,
        _In_    UINT32 Plane,
        _In_    UINT32 Subresource,
        _In_    const VOID* pLinearData,
        _In_    UINT32 RowPitchBytes,
        _In_    UINT32 SlicePitchBytes
    ) = 0;

    //-------------------------------------------------------------------------------------------------
    // Copies texel data out of the given subresource (mip + slice) into the 2D or 3D linear output buffer,
    // using pTiledResourceBaseAddress as the resource's base address.  For offline texture preparation,
    // pTiledResourceBaseAddress does not need to be properly aligned.
    virtual HRESULT __cdecl CopyFromSubresource(
        _Inout_ VOID* pLinearData,
        _In_    UINT32 Plane,
        _In_    UINT32 Subresource,
        _In_    UINT32 Sample,
        _In_    const VOID* pTiledResourceBaseAddress,
        _In_    UINT32 RowPitchBytes,
        _In_    UINT32 SlicePitchBytes
    ) = 0;

    //-------------------------------------------------------------------------------------------------
    // Returns the resource tiling layout, if the resource is created as a tiled resource using the
    // XG_RESOURCE_MISC_TILED flag.
    virtual HRESULT __cdecl GetResourceTiling(
        _Out_opt_   UINT32 *pNumTilesForEntireResource,
        _Out_opt_   XG_PACKED_MIP_DESC *pPackedMipDesc,
        _Out_opt_   XG_TILE_SHAPE *pStandardTileShapeForNonPackedMips,
        _Inout_opt_ UINT32 *pNumSubresourceTilings,
        _In_        UINT32 FirstSubresourceTilingToGet,
        _Out_writes_(*pNumSubresourceTilings) XG_SUBRESOURCE_TILING *pSubresourceTilingsForNonPackedMips
    ) = 0;

    //----------------------------------------------------------------------------------------------
    // Returns the texture view descriptor for the given plane.  Returns E_INVALIDARG if the given
    // plane cannot be described with a descriptor.
    virtual HRESULT GetTextureViewDescriptor(
        _In_  UINT32 PlaneIndex,
        _Out_ XG_DESCRIPTOR_TEXTURE_VIEW* pDescriptor
    ) = 0;

    //----------------------------------------------------------------------------------------------
    // Returns true if the resource is created as a tiled resource
    virtual bool IsTiledResource() const = 0;

    //----------------------------------------------------------------------------------------------
    // Returns the swizzle pattern information 
    virtual HRESULT GetSwizzlePatternInfo(
        _In_    UINT32 PlaneIndex,
        _Out_   XG_SWIZZLE_PATTERN_INFO* pInfo
    ) const = 0;
};

//-------------------------------------------------------------------------------------------------
// XGCreateTexture1DComputer creates a texture address computer object for the given 1D texture desc.

HRESULT __cdecl XGCreateTexture1DComputer(
    _In_  const XG_TEXTURE1D_DESC* pDesc,
    _Out_ XGTextureAddressComputer** ppComputer
);

//-------------------------------------------------------------------------------------------------
// XGCreateTexture2DComputer creates a texture address computer object for the given 2D texture desc.

HRESULT __cdecl XGCreateTexture2DComputer(
    _In_  const XG_TEXTURE2D_DESC* pDesc,
    _Out_ XGTextureAddressComputer** ppComputer
);

//-------------------------------------------------------------------------------------------------
// XGCreateTexture3DComputer creates a texture address computer object for the given 3D texture desc.

HRESULT __cdecl XGCreateTexture3DComputer(
    _In_  const XG_TEXTURE3D_DESC* pDesc,
    _Out_ XGTextureAddressComputer** ppComputer
);

//--------------------------------------------------------------------------------------------------
// XGCreateTextureComputerFromDescriptor creates a texture address computer object for the given
// texture descriptor.

HRESULT __cdecl XGCreateTextureComputerFromDescriptor(
    _In_  const XG_DESCRIPTOR_TEXTURE_VIEW* pDescriptor,
    _Out_ XGTextureAddressComputer** ppComputer
);

//--------------------------------------------------------------------------------------------------
// XGCreateTextureComputer creates a texture address computer object for the given D3D12-style
// resource desc. Buffer descs are not supported and will return E_INVALIDARG.

HRESULT __cdecl XGCreateTextureComputer(
    _In_  const XG_RESOURCE_DESC* pDesc,
    _Out_ XGTextureAddressComputer** ppComputer
);

//--------------------------------------------------------------------------------------------------
// XGComputeTileShape returns the shape of a single tiled resource tile for the given format.
// If Format is XG_FORMAT_UNKNOWN, then the BytesPerElement parameter is used instead;
// in that case BytesPerElement must be 1, 2, 4, 8, or 16 bytes.  If Format is not
// XG_FORMAT_UNKNOWN, then the value of BytesPerElement is ignored and is computed by this function.
// SampleCount must be 1 for all resource dimensions other than Texture2D.
// For Texture2D, SampleCount must be one of 1, 2, 4, or 8.
// The tile shape is returned both in units of texels and elements.  If Format is XG_FORMAT_UNKNOWN,
// then pTileShapeInTexels will be filled with zeros if present.
HRESULT __cdecl XGComputeTileShape(
    _In_      XG_RESOURCE_DIMENSION Dimension,
    _In_      XG_FORMAT Format,
    _In_      UINT32 BytesPerElement,
    _In_      UINT32 SampleCount,
    _Out_opt_ XG_TILE_SHAPE* pTileShapeInTexels,
    _Out_opt_ XG_TILE_SHAPE_ELEMENTS* pTileShapeInElements );


//--------------------------------------------------------------------------------------------------
// XGComputeBankRotationAddress computes a bank rotated address for a 2D tiled surface.
// The bank rotation index can be an arbitrary value; the rotation offset will be clamped to the
// bank rotation address mask associated with the selected mip level.
// If input parameters are invalid or bank rotation is not supported on the selected mip level,
// then this function returns false.
inline bool XGComputeBankRotationAddress(
    _In_  XG_GPU_VIRTUAL_ADDRESS BaseAddress,
    _In_  const XG_RESOURCE_LAYOUT* pLayout,
    _In_  UINT32 PlaneIndex,
    _In_  UINT32 MipLevelIndex,
    _In_  UINT32 BankRotationIndex,
    _Out_ XG_GPU_VIRTUAL_ADDRESS* pBankRotatedBaseAddress)
{
    // Validate inputs and outputs:
    if (pLayout == nullptr || pBankRotatedBaseAddress == nullptr)
    {
        return false;
    }

    // Output the base address as a default return value:
    *pBankRotatedBaseAddress = BaseAddress;

    if (PlaneIndex >= pLayout->Planes || MipLevelIndex >= pLayout->MipLevels)
    {
        return false;
    }

    const XG_MIPLEVEL_LAYOUT& MipLayout = pLayout->Plane[PlaneIndex].MipLayout[MipLevelIndex];

    // Ensure that the base address is properly aligned to the mip level alignment:
    if ((BaseAddress % MipLayout.AlignmentBytes) != 0)
    {
        return false;
    }

    // If the mip level does not support bank rotation, then the bit mask will be 0:
    if (MipLayout.BankRotationAddressBitMask == 0)
    {
        return false;
    }

    // Compute the bank rotation offset in bytes:
    UINT64 RotationOffsetBytes = MipLayout.BankRotationBytesPerSlice * BankRotationIndex;
    RotationOffsetBytes &= MipLayout.BankRotationAddressBitMask;

    // Apply the bank rotation offset to the base address and return the value:
    *pBankRotatedBaseAddress = BaseAddress + RotationOffsetBytes;

    return true;
}

//--------------------------------------------------------------------------------------------------
// Call XGSetHardwareVersion before calling other XG APIs to ensure that XG's behavior matches the
// desired hardware platform behavior.
void __cdecl XGSetHardwareVersion(
    _In_      XG_HARDWARE_VERSION Version);

//--------------------------------------------------------------------------------------------------
// If you need to call XGSetHardwareVersion at runtime to switch between hardware versions, use
// the XG exclusive locking APIs to synchronize creation of XG texture computer objects.
//
// Exclusive locking is disabled by default. When enabled, XG will acquire a critical section
// within all of the XGCreateTexture*Computer* and XGComputeTexture*Layout APIs.
//
// Before creating any texture computers, and before the D3D device is created, call 
// XGEnableExclusiveLocking to enable exclusive locking.
void __cdecl XGEnableExclusiveLocking(_In_ BOOL LocksEnabled);

// To synchronize calls to XGSetHardwareVersion around XG texture computer creation, use the
// XGEnter/LeaveExclusiveLock APIs to manually acquire and release the XG critical section.
// If exclusive locking is disabled, these APIs will do nothing (and XGEnterExclusiveLock will
// return FALSE).
BOOL __cdecl XGEnterExclusiveLock();
void __cdecl XGLeaveExclusiveLock();

//--------------------------------------------------------------------------------------------------
// XGInitializeLibrary will perform one-time allocation and initialization of global state within
// XG_XS.  It is safe to call multiple times; if XG_XS is already initialized, XGInitializeLibrary
// will return S_FALSE instead of S_OK.
// XGInitializeLibrary is not thread safe; please ensure it is not called at the same time as
// any other XG API.
// A call to XGInitializeLibrary is optional - all texture computer creation methods, as well as
// XGSetHardwareVersion, contain an implicit call to XGInitializeLibrary.
HRESULT __cdecl XGInitializeLibrary();

//--------------------------------------------------------------------------------------------------
// XGTerminateLibrary will deallocate global state within XG_XS.
// It is safe to call multiple times; if XG_XS is already terminated, XGTerminateLibrary will
// return S_FALSE instead of S_OK.
// XGTerminateLibrary is not thread safe; please ensure it is not called at the same time as
// any other XG API.
// Following a call to XGTerminateLibrary, XG_XS can be reinitialized via a call to XGInitializeLibrary
// or implicitly via other XG APIs.
HRESULT __cdecl XGTerminateLibrary();

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// XGBVHComputer

struct D3D12_SERIALIZED_RAYTRACING_ACCELERATION_STRUCTURE_HEADER;
struct D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC;
struct ID3D12GraphicsCommandList;
struct ID3D12Device;
typedef UINT64 D3D12_GPU_VIRTUAL_ADDRESS;

class XGBVHComputer
{
public:

    virtual ~XGBVHComputer() {}

    //-------------------------------------------------------------------------------------------------
    // Increments the refcount and returns the new value.
    virtual ULONG __cdecl AddRef() = 0;

    //-------------------------------------------------------------------------------------------------
    // Decrements the refcount and returns the new value.  If the new value is 0, the object is deleted.
    virtual ULONG __cdecl Release() = 0;

    //-------------------------------------------------------------------------------------------------
    // Deserialize a BVH "in place". The blob is validated as a serialized BVH and that it has the
    // proper 256-byte alignment and that the BVH was built using the current GDK version. This buffer
    // should be GPU-visible (e.g. write combined memory.)
    //
    // pSerializedHeader    : A pointer to the start of the serialized BVH blob (256B-aligned)
    // vaBvhStartAddress    : The resultant address of the start of the BVH (located within the blob)

    virtual HRESULT DeserializeBVH(
        _In_    const   D3D12_SERIALIZED_RAYTRACING_ACCELERATION_STRUCTURE_HEADER* pSerializedHeader,
        _Out_           D3D12_GPU_VIRTUAL_ADDRESS&                                 vaBvhStartAddress) = 0;


    //-------------------------------------------------------------------------------------------------
    // Decompresses a compressed BVH blob to a destination BVH address.
    //
    // Desc.SourceAccelerationStructureData     : Dehydrated BVH blob
    // Desc.DestAccelerationStructureData       : Rehydrated BVH destination
    // Desc.ScratchAccelerationStructureData    : Ignored
    // Desc.Inputs                              : Description of input geometry

    virtual HRESULT DecompressBVH(
        _In_ const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC& Desc) = 0;

    //-------------------------------------------------------------------------------------------------
    // Legacy API (before March 2023). This API does not support geometry transforms.

    virtual HRESULT DecompressBVH(
        _In_ const void*  CompressedBuffer,
        _In_ void*        DestinationBuffer,
        _In_ UINT64       DestinationBufferSize,
        _In_reads_(NumGeometries) const void** VertexBuffers,
        _In_reads_(NumGeometries) UINT32*      VertexStrides,
        _In_reads_(NumGeometries) XG_FORMAT*   VertexFormats,
        _In_reads_(NumGeometries) const void** IndexBuffers,
        _In_reads_(NumGeometries) XG_FORMAT*   IndexFormats,
        _In_ UINT32       NumGeometries ) = 0;

    //-------------------------------------------------------------------------------------------------
    // Validates the Decompressed BVH structure against the Original BVH
    // For tests / validation purposes (not performant)

    virtual HRESULT ValidateDecompressedBVH(
        _In_ const void*  OriginalBVHStructure,
        _In_ const void*  DecompressedBVHStructure ) = 0;

    //-------------------------------------------------------------------------------------------------
    // Determine how much scratch memory is needed to decompress a BVH with the GPU
    //
    // For each BVH, query the scratch space and provide a pointer to the allocated region in the
    // Desc.ScratchAccelerationStructureData.

    virtual UINT64 GetRequiredGpuScratchSizeInBytes(
        _In_ UINT NumGeometry) = 0;

    //-------------------------------------------------------------------------------------------------
    // Decompress one or more BVHs using the GPU. (Unavailable in the desktop build of XG.)
    // 
    // Requires XGCreateBVHComputer2 and a non-null device.
    // 
    // ScratchAccelerationStructureData must be allocated per BVH. Use GetRequiredGpuScratchSizeInBytes()
    // to determine allocation size.

    virtual HRESULT DecompressBVH_BatchedGPU(
        _In_ ID3D12GraphicsCommandList* pCommandList,
        _In_reads_(NumDescs) const D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC Descs[],
        _In_ UINT32                     NumDescs) = 0;
};

//--------------------------------------------------------------------------------------------------
// XGCreateBVHComputer creates a BVH computer for manipulating BVH structures at runtime

HRESULT __cdecl XGCreateBVHComputer(
    _Out_ XGBVHComputer** ppComputer );

//--------------------------------------------------------------------------------------------------
// Same as above but adds capability for decompressing BVHs on the GPU

HRESULT __cdecl XGCreateBVHComputer2(
    _In_  ID3D12Device*     pD3DDevice,
    _Out_ XGBVHComputer**   ppComputer );

//-------------------------------------------------------------------------------------------------

#define XG_DCC_BLOCK_SIZE_BYTES 256

typedef enum XG_DCC_COMPRESSION_LEVEL
{
    XG_DCC_COMPRESSION_LEVEL_64B = 0,
    XG_DCC_COMPRESSION_LEVEL_128B_TEXTURE_COMPATIBLE = 1,
    XG_DCC_COMPRESSION_LEVEL_256B = 2
} XG_DCC_COMPRESSION_LEVEL;

typedef enum XG_DCC_COLOR_TRANSFORM
{
    XG_DCC_COLOR_TRANSFORM_AUTO_TEXTURE_COMPATIBLE = 0,
    XG_DCC_COLOR_TRANSFORM_NONE_TEXTURE_COMPATIBLE = 1,
    XG_DCC_COLOR_TRANSFORM_LSC_CORRELATED = 2,
    XG_DCC_COLOR_TRANSFORM_MSC_CORRELATED = 3
} XG_DCC_COLOR_TRANSFORM;

typedef struct XG_DCC_OPTIONS
{
    XG_DCC_COMPRESSION_LEVEL CompressionLevel;
    XG_DCC_COLOR_TRANSFORM ColorTransform;
} XG_DCC_OPTIONS;

//-------------------------------------------------------------------------------------------------

HRESULT __cdecl XGEncodeDCC(
    _In_ UINT32 DataSizeBlocks,
    _In_reads_bytes_(DataSizeBlocks * XG_DCC_BLOCK_SIZE_BYTES) const void* pSourceData,
    _Out_writes_bytes_(DataSizeBlocks * XG_DCC_BLOCK_SIZE_BYTES) void* pDestData,
    _Out_writes_bytes_(DataSizeBlocks) void* pDestKeys,
    _Out_writes_opt_(DataSizeBlocks) UINT* pDestCompressedSizes,
    _In_ XG_FORMAT Format,
    _In_ XG_SWIZZLE_MODE SwizzleMode,
    _In_ XG_RESOURCE_DIMENSION Dimension,
    _In_ const XG_DCC_OPTIONS* pDccOptions);

//-------------------------------------------------------------------------------------------------

HRESULT __cdecl XGDecodeDCC(
    _In_ UINT32 DataSizeBlocks,
    _In_reads_bytes_(DataSizeBlocks * XG_DCC_BLOCK_SIZE_BYTES) const void* pSourceData,
    _In_reads_bytes_(DataSizeBlocks) const void* pSourceKeys,
    _Out_writes_bytes_(DataSizeBlocks * XG_DCC_BLOCK_SIZE_BYTES) void* pDestData,
    _In_ XG_FORMAT Format,
    _In_ XG_SWIZZLE_MODE SwizzleMode,
    _In_ XG_RESOURCE_DIMENSION Dimension,
    _In_ const XG_DCC_OPTIONS* pDccOptions);

//-------------------------------------------------------------------------------------------------

HRESULT __cdecl XGValidateDescriptor(
    _In_ UINT32 DescriptorDwordCount,
    _In_ const UINT32* pDescriptor,
    _In_ UINT32 Flags);

//-------------------------------------------------------------------------------------------------

// Transform block compressed texture data for improved compressibility. Use DirectStorage to
// transform the data back to its original form using the SwizzleMode option.
//
HRESULT __cdecl XGShuffleTextureBufferForDirectStorage(
    _In_reads_bytes_(BufferSizeBytes) const void* pSourceData,  // Source BC texture stream
    _Out_writes_bytes_(BufferSizeBytes) void* pDestData,        // Destination buffer
    _In_ SIZE_T BufferSizeBytes,                                // Source and destination buffer sizes
    _In_ XG_FORMAT TextureFormat,                               // Only BC1, BC3, BC4, and BC5 will be transformed
    _Out_opt_ UINT32* DStorageSwizzleMode);                     // Returns the swizzle mode selected

#pragma warning(pop)
