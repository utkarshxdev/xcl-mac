/*
 * Copyright © 2025 Microsoft Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef XDXCAPI_H
#define XDXCAPI_H

#include <windows.h>
#include <unknwn.h>
#include <oleauto.h>
#include <dxcapi.h>

#include "aco_xbox.h"

#ifdef __cplusplus
extern "C" {
#endif

DEFINE_GUID(IID_IXdxcCompiler3,
    0xdfa0cafb, 0x039a, 0x4ff4, 0x84, 0x79, 0xa7, 0xe7, 0x4c, 0xc2, 0xb1, 0xdb);

DEFINE_GUID(IID_IXdxcResult,
    0x04b4ab5f, 0xbbd7, 0x49bf, 0x91, 0xf5, 0x9d, 0x97, 0xec, 0xba, 0x57, 0x43);

DEFINE_GUID(CLSID_XdxcContainerInspector,
    0x7966321c, 0x50b4, 0x4161, 0xa4, 0xb4, 0x40, 0x9c, 0xc9, 0x4f, 0x2b, 0x8d);
DEFINE_GUID(IID_IXdxcContainerInspector,
    0x8bcaac90, 0x00a7, 0x4d0b, 0xae, 0x91, 0x81, 0x80, 0x2b, 0x01, 0xf0, 0xe5);

DEFINE_GUID(CLSID_XdxcContainerEditor,
    0x0265e831, 0x619b, 0x42b0, 0x91, 0x79, 0xbb, 0x50, 0x4f, 0x2e, 0xa1, 0x78);
DEFINE_GUID(IID_IXdxcContainerEditor,
    0xd3f2685f, 0x8f92, 0x42a7, 0x9d, 0xe5, 0x2b, 0xd7, 0x8c, 0x10, 0x2e, 0xe4);

typedef struct XDXC_COMPUTE_INSPECT_RESULT {
    UINT32 fileSize;
    UINT32 acccSize;
    UINT32 acccOffset;
    UINT32 packetOffset, packetSize;
    UINT32 metadataOffset, metadataSize;
    UINT32 csIsaOffset, csIsaSize;
    UINT32 csMetaOffset, csMetaSize;
} XDXC_COMPUTE_INSPECT_RESULT;

#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus

struct DECLSPEC_UUID("dfa0cafb-039a-4ff4-8479-a7e74cc2b1db")
IXdxcCompiler3 : public IDxcCompiler3 {
    virtual HRESULT STDMETHODCALLTYPE SetCompileOptions(
        const aco_compile_options *pOpts) = 0;
};

struct DECLSPEC_UUID("04b4ab5f-bbd7-49bf-91f5-9d97ecba5743")
IXdxcResult : public IDxcResult {
    virtual HRESULT STDMETHODCALLTYPE GetRawDxil(IDxcBlob **ppRawDxil) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetCookPayload(
        IDxcBlob **ppPayload, UINT32 *pFourCC) = 0;
};

struct DECLSPEC_UUID("8bcaac90-00a7-4d0b-ae91-81802b01f0e5")
IXdxcContainerInspector : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE InspectComputeContainer(
        const void *pContainer, UINT32 cbContainer,
        XDXC_COMPUTE_INSPECT_RESULT *pOut) = 0;
};

struct DECLSPEC_UUID("d3f2685f-8f92-42a7-9de5-2bd78c102ee4")
IXdxcContainerEditor : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE EnumParts(
        const void *pContainer, UINT32 cbContainer,
        aco_dxbc_part_info *pParts, UINT32 maxParts, UINT32 *pCount) = 0;

    virtual HRESULT STDMETHODCALLTYPE RewriteParts(
        const void *pContainer, UINT32 cbContainer,
        const UINT32 *pFourccs, UINT32 numFourccs, BOOL keepOnly,
        IDxcBlob **ppOut) = 0;

    virtual HRESULT STDMETHODCALLTYPE StripDxilBitcode(
        const void *pContainer, UINT32 cbContainer,
        IDxcBlob **ppOut) = 0;
};

#endif /* __cplusplus */

#endif /* XDXCAPI_H */
