---
author: sdgmiller
title: "PresentX (Xbox One family)"
description: "Presents a set of resources for display."
kindex:
- PresentX
- ID3D12CommandQueue::PresentX
ms.author: mattbron
ms.topic: reference
edited: 07/30/2019
quality: good
ms.date: '08/01/2022'
permissioned-type: secret
---

# ID3D12CommandQueue::PresentX (Xbox One family)  

Presents a set of resources for display.

## Syntax

```cpp
HRESULT PresentX(
        UINT PlaneCount,
        const D3D12XBOX_PRESENT_PLANE_PARAMETERS* pPlaneParameters,
        const D3D12XBOX_PRESENT_PARAMETERS* pPresentParameters)
```

### Parameters

*PlaneCount* &nbsp;&nbsp;\_In\_range\_(1, D3D12XBOX_PRESENT_PLANES_MAX)
Type: UINT

The count of planes to present. Multiple planes can consist of a background plane with overlays and/or pairs of standard and High Dynamic Range compositions of the frame. Use a plane count of 0 to swap the display to null, which will set the display color to black. Xbox Series X&#124;S allows blending of planes larger than 1920 x 1080, both of which could be High Dynamic Range.

*pPlaneParameters* &nbsp;&nbsp;\_In\_reads\_(PlaneCount)
Type: const [D3D12XBOX_PRESENT_PLANE_PARAMETERS](../../../structs/d3d12xbox_present_plane_parameters.md)*

Pointer to the first element in an array of parameters that are used to control the presentation of each provided plane. Arrange the array by Z order: back to front. Set the elements by closest proximity and then by color-space (HDR-SDR) pairs.

*pPresentParameters* &nbsp;&nbsp;\_In\_opt\_
Type: const [D3D12XBOX_PRESENT_PARAMETERS](../../../structs/d3d12xbox_present_parameters.md)*

Optional. A pointer to a set of parameters used to control the presentation.

### Return value

Type: HRESULT

HRESULT success or error code.

Returns S_OK if the specified swap resources have been successfully queued for presentation.

Returns ERROR_WAS_STILL_DRAWING if [D3D12XBOX_PRESENT_FLAG_DO_NOT_WAIT](../../../enums/d3d12xbox_present_flags.md) is specified and one or more of the specified *ppResources* of *pPlaneParameters* are currently displayed or queued for presentation.

Returns E_INVALIDARG on invalid parameters or invalid composition of the presentation.

Returns D3DERR_DEVICEREMOVED if the GPU becomes unresponsive prior to the call to **PresentX**

## Remarks

Resources included in the *ppResources* arrays contained within *pPlaneParameters* serve as a replacement for legacy swap chains and must be created with support for being flipped to a display. To enable support for flipping, call [CreateCommittedResource](https://learn.microsoft.com/windows/win32/api/d3d12/nf-d3d12-id3d12device-createcommittedresource) and set the **D3D12_HEAP_FLAG_ALLOW_DISPLAY** flag ([D3D12_HEAP_FLAGS](../../../enums/d3d12_heap_flags.md)) in the *pDesc* parameter. Resources created without an underlying heap are not permitted yet.

Swap chains have evolved into a user-managed collection of display-able resources; however, due to TLB limitations, a limited number of this type of resources can be instantiated at one time. A display-able resource can't be released to the point of deallocation if it's currently being scanned out by a display or queued for presentation. You must first flip the display to another set of resources or to null (if the application is shut down). To perform a null presentation, call **PresentX(0, nullptr, nullptr);**.

With *ppResources* replacing swap chains, the color space of a swap resource is now declared as part of the plane parameters included in *pPlaneParameters*, and automatic tone-mapping for GameDVR is selected for a resource through the [D3D12XBOX_RESOURCE_FLAG_ALLOW_AUTOMATIC_GAMEDVR_TONE_MAP](../../../enums/d3d12_resource_flags.md) resource-creation flag, while incurring additional heap storage and per-Present GPU cost. Xbox Series X&#124;S adds two more swap resource formats **DXGI_FORMAT_R9G9B9E5_SHAREDEXP** and **DXGI_FORMAT_A8_UNORM**. A **DXGI_FORMAT_A8_UNORM** swap resource should always be paired with a **DXGI_FORMAT_R9G9B9E5_SHAREDEXP** swap resource. It serves as the alpha channel for the RGBE resource, when an alpha channel is desired (for instance, when blending more than one plane). Xbox Series X&#124;S also allows swap resources to be Delta Color Compressed through the [D3D12XBOX_RESOURCE_FLAG_ALLOW_DCC](../../../enums/d3d12_resource_flags.md) resource-creation flag.

Resources might not be presented if identification of the frames used in their production, or at least used in the initiation of their construction, is not provided. This is necessary for the correlation of presentation statistics to individual frames, for measuring full-frame pipeline latency from the origin, and for input sampling to the display. Identification is performed through the *Token* member of the *pPlaneParameters* parameters, where different [D3D12XBOX_FRAME_PIPELINE_TOKEN](../../../enums/d3d12xbox_frame_pipeline_token.md) values can be specified for each composition plane (background and overlay), given the potential for different plane presentation frequencies and thus different basis frames. The pipeline token should be included redundantly for planes that are all sourced from the same frame.

Frames and their associated swap resources must be presented in the order they were produced. A stream of frame pipeline tokens that originate from [WaitFrameEventX](../../id3d12device/methods/id3d12device_waitframeeventx.md) are terminated through *PresentX*, with no frame drops or re-ordering permitted. The contents of one composition plane may be persisted on the display while one or more other planes are being updated; to do this, pass the same resource and frame token as those provided for the plane in the previous call to *PresentX*.

To remove a composition plane established during a series of calls to *PresentX*, omit the plane's resources and tokens from the array pointed to by *pPlaneParameters* on a subsequent call to *PresentX*.

Presented swap resources are flipped to the display at the time of their scheduled VBlank; this assumes that the *PresentX* call is made and processed within the threshold of the target declared in *ImmediateThresholdPercent* of *pPresentParameters*. If **PresentX** misses the VBlank but arrives within the threshold, then it will be flipped to the display immediately. This threshold is based on a percentage of the display scan-out time (VBlank to VBlank), not on a percentage of the defined frame interval. Also, unlike immediate presentation, arranged through the use of the [D3D12XBOX_PRESENT_FLAG_IMMEDIATE](../../../enums/d3d12xbox_present_flags.md) flag in *pPresentParameters* (which immediately triggers the origin of the next frame), immediate flips that are based on the threshold will signal the subsequent frame origin only if that origin's time has already passed.

## Requirements

**Header:** d3d12_x.h  

**Library:** d3d12_x.lib  

**Supported platforms:** Xbox One family

## See also

[ID3D12CommandQueue](./id3d12commandqueue_beginevent_public.md)

