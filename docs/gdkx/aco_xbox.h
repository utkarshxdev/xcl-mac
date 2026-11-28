/*
 * Copyright © 2025 Microsoft Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ACO_XBOX_H
#define ACO_XBOX_H

#ifdef _WIN32
#  ifdef ACO_XBOX_DLL_BUILDING
#    define ACO_DLL_API __declspec(dllexport)
#  else
#    define ACO_DLL_API __declspec(dllimport)
#  endif
#else
#  define ACO_DLL_API __attribute__((visibility("default")))
#endif

#ifndef _D3D12_XS_H_INCLUDED_
struct D3D12_ROOT_SIGNATURE_DESC1;
struct D3D12XBOX_SERIALIZE_GRAPHICS_PIPELINE_STATE;
struct D3D12XBOX_SERIALIZE_COMPUTE_PIPELINE_STATE;
#endif

#ifndef __cplusplus
typedef struct D3D12_ROOT_SIGNATURE_DESC1 D3D12_ROOT_SIGNATURE_DESC1;
typedef struct D3D12XBOX_SERIALIZE_GRAPHICS_PIPELINE_STATE D3D12XBOX_SERIALIZE_GRAPHICS_PIPELINE_STATE;
typedef struct D3D12XBOX_SERIALIZE_COMPUTE_PIPELINE_STATE D3D12XBOX_SERIALIZE_COMPUTE_PIPELINE_STATE;
#endif

#include <stddef.h>
#include <stdint.h>

#ifndef ACO_DXBC_VOCAB_DEFINED
#define ACO_DXBC_VOCAB_DEFINED

#define ACO_DXBC_FOURCC_DXBC 0x43425844u
#define ACO_DXBC_FOURCC_DXIL 0x4C495844u
#define ACO_DXBC_FOURCC_HASH 0x48534148u
#define ACO_DXBC_FOURCC_ILDB 0x42444C49u
#define ACO_DXBC_FOURCC_ILDN 0x4E444C49u
#define ACO_DXBC_FOURCC_RTS0 0x30535452u
#define ACO_DXBC_FOURCC_PSV0 0x30565350u
#define ACO_DXBC_FOURCC_STAT 0x54415453u
#define ACO_DXBC_FOURCC_ACGS 0x53474341u
#define ACO_DXBC_FOURCC_ACCC 0x43434341u

typedef struct aco_dxbc_part_info {
   uint32_t fourcc;
   uint32_t offset;
   uint32_t size;
} aco_dxbc_part_info;

#endif /* ACO_DXBC_VOCAB_DEFINED */

#ifdef __cplusplus
extern "C" {
#endif

ACO_DLL_API long AcoOfflineInit(void);
ACO_DLL_API void AcoOfflineShutdown(void);
ACO_DLL_API void AcoOfflineFree(void *ptr);

#define ACO_ALLOC_ATTR_ALIGN_MASK      0xFFFFull
#define ACO_ALLOC_ATTR_ALIGNMENT(attr) ((size_t)((attr) & ACO_ALLOC_ATTR_ALIGN_MASK))
#define ACO_ALLOC_ATTR_MAKE_ALIGN(a)   ((uint64_t)(a) & ACO_ALLOC_ATTR_ALIGN_MASK)

typedef void *(*PFN_AcoOfflineAlloc)(size_t size, uint64_t attributes);
typedef void  (*PFN_AcoOfflineFree )(void  *ptr,  uint64_t attributes);

ACO_DLL_API void AcoOfflineSetAllocationHooks(PFN_AcoOfflineAlloc pfnAlloc,
                                              PFN_AcoOfflineFree  pfnFree);

typedef struct aco_compile_options {
   uint32_t size;
   uint32_t target_min_waves;
   uint32_t use_re_z;
   uint32_t force_zorder;
   uint32_t enable_wgp_mode;
   uint32_t break_on_shader_start;
   uint32_t skip_dxbc_hash_check;
   uint32_t num_static_sampler_flags;
   const uint32_t *static_sampler_flags;
   uint32_t line_rasterization_mode;
   uint32_t reserved[19];
} aco_compile_options;

#define ACO_XBOX_SAMPLER_FLAG_UINT_BORDER_COLOR           0x1u
#define ACO_XBOX_SAMPLER_FLAG_NON_NORMALIZED_COORDINATES  0x2u

#define ACO_XBOX_LINE_RASTER_LEGACY                0u
#define ACO_XBOX_LINE_RASTER_ALIASED               1u
#define ACO_XBOX_LINE_RASTER_ALPHA_ANTIALIASED     2u
#define ACO_XBOX_LINE_RASTER_QUADRILATERAL_WIDE    3u
#define ACO_XBOX_LINE_RASTER_QUADRILATERAL_NARROW  4u

#define ACO_COMPILE_OPTIONS_DEFAULT { sizeof(aco_compile_options), 0, 0, 0, 0, 0, 0, 0, NULL, 0, {0} }

ACO_DLL_API long AcoOfflineSerializePipelineStateX(
    const void *pStreamDesc,
    const D3D12_ROOT_SIGNATURE_DESC1 *pRootSigDesc,
    D3D12XBOX_SERIALIZE_GRAPHICS_PIPELINE_STATE *pGraphicsOut,
    D3D12XBOX_SERIALIZE_COMPUTE_PIPELINE_STATE *pComputeOut);

ACO_DLL_API long AcoOfflineSerializePipelineStateXEx(
    const void *pStreamDesc,
    const D3D12_ROOT_SIGNATURE_DESC1 *pRootSigDesc,
    D3D12XBOX_SERIALIZE_GRAPHICS_PIPELINE_STATE *pGraphicsOut,
    D3D12XBOX_SERIALIZE_COMPUTE_PIPELINE_STATE *pComputeOut,
    const char *pszName,
    const aco_compile_options *pCompileOpts);

typedef enum aco_cook_stage {
   ACO_COOK_STAGE_VS = 0,
   ACO_COOK_STAGE_PS = 1,
   ACO_COOK_STAGE_GS = 2,
   ACO_COOK_STAGE_HS = 3,
   ACO_COOK_STAGE_DS = 4,
   ACO_COOK_STAGE_MS = 5,
} aco_cook_stage;

#define ACO_COOK_FLAG_COMPRESS_NIR 0x00000001u

ACO_DLL_API long AcoOfflineCookGraphicsStage(
    uint32_t stage,
    const void *pDxilBytes, uint32_t cbDxil,
    const D3D12_ROOT_SIGNATURE_DESC1 *pRootSigDesc,
    const aco_compile_options *pCompileOpts,
    void **ppCookedBlob, uint32_t *pCookedSize);

ACO_DLL_API long AcoOfflineCookGraphicsStageEx(
    uint32_t stage,
    const void *pDxilBytes, uint32_t cbDxil,
    const D3D12_ROOT_SIGNATURE_DESC1 *pRootSigDesc,
    const aco_compile_options *pCompileOpts,
    uint32_t cookFlags,
    void **ppCookedBlob, uint32_t *pCookedSize);

#define ACO_FOURCC_ACGS 0x53474341u
#define ACO_FOURCC_ACCC 0x43434341u

ACO_DLL_API long AcoOfflineInjectCookPartIntoDxil(
    const void *pDxilContainer, uint32_t cbDxilContainer,
    uint32_t fourcc,
    const void *pPayload, uint32_t cbPayload,
    void **ppOutContainer, uint32_t *pOutContainerSize);

ACO_DLL_API long AcoOfflineFindCookPartInDxil(
    const void *pDxilContainer, uint32_t cbDxilContainer,
    uint32_t fourcc,
    const void **ppPayload, uint32_t *pPayloadSize);

ACO_DLL_API long AcoOfflineEnumPartsInDxil(
    const void *pDxilContainer, uint32_t cbDxilContainer,
    aco_dxbc_part_info *pParts, uint32_t maxParts, uint32_t *pCount);

ACO_DLL_API long AcoOfflineStripPartsFromDxil(
    const void *pDxilContainer, uint32_t cbDxilContainer,
    const uint32_t *pFourccs, uint32_t numFourccs, int keepOnly,
    void **ppOutContainer, uint32_t *pOutContainerSize);

ACO_DLL_API long AcoOfflineStripDxilBitcodeFromDxil(
    const void *pDxilContainer, uint32_t cbDxilContainer,
    void **ppOutContainer, uint32_t *pOutContainerSize);

#ifdef __cplusplus
}
#endif

#endif /* ACO_XBOX_H */
