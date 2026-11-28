---
author: sdgmiller
title: "D3D12XBOX_PRESENT_PLANE_PARAMETERS (Xbox One family)"
description: "Configures a presentation plane for composition with ID3D12CommandQueue::PresentX."
kindex: D3D12XBOX_PRESENT_PLANE_PARAMETERS
ms.author: mattbron
ms.topic: reference
edited: 07/04/2019
ms.date: '08/01/2022'
permissioned-type: secret
---

# D3D12XBOX_PRESENT_PLANE_PARAMETERS (Xbox One family)  

Configures a presentation plane for composition with [ID3D12CommandQueue::PresentX](../interfaces/id3d12commandqueue/methods/id3d12commandqueue_presentx.md).

## Syntax

```cpp
typedef struct D3D12XBOX_PRESENT_PLANE_PARAMETERS
{
    D3D12XBOX_FRAME_PIPELINE_TOKEN Token;
    UINT ResourceCount;
    ID3D12Resource** ppResources;
    D3D12_RECT* pSrcViewRects;
    D3D12XBOX_VIEW_RECT* pDestPlacementBase;
    DXGI_COLOR_SPACE_TYPE ColorSpace;
    D3D12XBOX_PRESENT_PLANE_SCALE_FILTER ScaleFilter;
    UINT ExtendedDescCount;
    D3D12XBOX_PRESENT_PLANE_DESC* pExtendedDescs;
    D3D12XBOX_PRESENT_PLANE_FLAGS Flags;
} D3D12XBOX_PRESENT_PLANE_PARAMETERS;
```

### Members

*pTokens*
Type: [D3D12XBOX_FRAME_PIPELINE_TOKEN](../enums/d3d12xbox_frame_pipeline_token.md)

Pipeline token that identifies the frame used in the production of *ppResources* that constitute the plane. The *Token* is generated at the frame origin and returned through a call to [ID3D12Device::WaitFrameEventX](../interfaces/id3d12device/methods/id3d12device_waitframeeventx.md) with the [D3D12XBOX_FRAME_EVENT_ORIGIN](../enums/d3d12xbox_frame_event_type.md) event.

*ResourceCount*
Type: UINT

Number of resources to be presented for the plane. Multiple resources may consist of dedicated color and alpha textures and/or of unique views for each player.

*ppResources*
Type: [ID3D12Resource](https://learn.microsoft.com/windows/win32/api/d3d12/nn-d3d12-id3d12resource)*

Pointer to the first element in an array of resources to be presented to the display for the plane. If support for unique view resources is to be added in the future, the array should be arranged by channel group (for example, RGBE or A8) and then by player. When presenting a RGBE + A8 pair, the RGBE resource must be ordered before the A8 resource.

*pSrcViewRects*
Type: [D3D12_RECT](https://learn.microsoft.com/windows/win32/direct3d12/d3d12-rect)*

Optional pointer to the first element of an array of rectangles, corresponding to the view region, of each player included in the presentation. The size of the array is defined by the [ViewCount](./d3d12xbox_present_parameters.md) parameter passed to [PresentX](../interfaces/id3d12commandqueue/methods/id3d12commandqueue_presentx.md). During presentation, each view region defined by its source rectangle will be scaled to a defined destination region on the display, as specified by *pDestPlacementBase*. These rectangles are permitted to overlap and/or match; they just identify the extents desired for each player view. Null can be passed for the member to define the source view rectangles that extend to fully cover the plane, as specified by the dimensions of the plane's associated resource.

*pDestPlacementBase*
Type: [D3D12XBOX_VIEW_RECT](./d3d12xbox_view_rect.md)*

Optional pointer to exactly one normalized view rectangle defining the destination region of the first player included in the presentation. During presentation, the source view rectangle of the first player in the plane's resource will be scaled to fill the destination region defined through *pDestPlacementBase*; the source view rectangles of all other players will be scaled to destination regions calculated indirectly, according to the relationship between the source and destination regions of the first player. More specifically, *pDestPlacementBase* is used to internally derive a placement offset and scaling factors for the first player view, and all other views employ identical scaling and complimentary offsets. PresentX](id3d12commandqueue_presentx.md) cannot be used to perform independent scaling and compositing of source views to arbitrary destination regions; instead, PresentX  uniformly scales and offsets source views to the display.

The destination placement view rectangle is expressed in normalized, floating-point coordinates. Left and top values of 0.0 correspond to the extreme upper-left of the destination display. Right and bottom values of 1.0 correspond to the extreme lower-right of the destination display. Null may be passed for the member to define full destination coverage for the first player view.

As an example of *pDestPlacementBase* usage in the presence of multiple *pSrcViewRects*, consider a split-screen game that involves three players. To present the player views from a 3840 x 2160 swap resource in the form of a single row with three columns of identical size that covers the entire display, a caller would provide the following:

  *pSrcViewRects*:  {0, 0, 1280, 2160}, {1280, 0, 2560, 2160}, {2560, 0, 3840, 2160}
  *pDestPlacementBase*:  {0.0f, 0.0f, 1.0f / 3.0f, 1.0f}

*ColorSpace*
Type: [DXGI_COLOR_SPACE_TYPE](https://learn.microsoft.com/windows/win32/api/dxgicommon/ne-dxgicommon-dxgi_color_space_type)*

Color space in which to interpret the resource data provided for the plane. The color space may also be used to determine which provided planes' resources are to be selected for display presentation (HDR) and which are to be used for GameDVR encoding (SDR). If no color space is given (that is, if the [D3D12XBOX_PRESENT_PLANE_PARAMETERS](./d3d12xbox_present_plane_parameters.md) themselves are null), then the color space is defined as **DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709**. Valid color spaces are **DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709** and **DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020**, both of which are gamma corrected. Xbox Series X&#124;S allows gamma corrected colorspaces **DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P709** and **DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_D65P3**. Additionally, Xbox Series X&#124;S allows linear light color spaces **DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709**, **DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_D65P3** and **DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P2020**. Linear light color spaces are only allowed with the swap resource format **DXGI_FORMAT_R9G9B9E5_SHAREDEXP**. Gamma corrected SDR color spaces are allowed with swap resources of any format, but HDR color spaces are only allowed with swap resources of format **DXGI_FORMAT_R10G10B10A2_UNORM**.

With **DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709**, content in the swapbuffer is expected to use 709 primaries and the range of values per channel are expected to be between 0.0 and 1.0. In HDR mode, the 0.0 to 1.0 range is interpreted as between 0 and 10,000 nits. In other words, a 10,000 nit D65 white is represented as (1.0, 1.0, 1.0). Similarly, with **DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_D65P3**, the 0.0 to 1.0 range is interpreted as between 0 and 10,000 nits and a D65 10,000 nit white is (1.0, 1.0, 1.0). The swapbuffer is expected to contain content that is created with P3 color primaries and a D65 white point. This is distinct from the DCI_P3 colorspace where the white point is not D65. With **DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P2020**, the swapbuffer is expected to contain 2020 color primaries with a D65 white point and in the range 0.0 and 1.0, which is interpreted as between 0 and 10,000 nits. A D65 10,000 nit white is (1.0, 1.0, 1.0).

The **DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P709** colorspace expects the swapbuffer content to be created with Rec. 709 color primaries with a D65 white point, but encoded with the PQ (ST.2084) curve. The **DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_D65P3** colorspace expects the swapbuffer content to be created with P3 color primaries with a D65 white point, but encoded with the PQ (ST.2084) curve and not with a power 1.0/2.6 curve or sRGB. This is distinct from DCI_P3, which uses a different encoding and white point. The **DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020** colorspace expects the swapbuffer content to be created with Rec. 2020 color primaries with a D65 white point and encoded with a PQ (ST.2084) curve.  

*ScaleFilter*
Type: [D3D12XBOX_PRESENT_PLANE_SCALE_FILTER](../enums/d3d12xbox_present_plane_scale_filter.md)

Preferred scale filter to employ when upsampling from a plane source to a larger display destination. The filter provided is merely a request and is not guaranteed to be applied on output. Scale filter is ignored on Xbox Series X&#124;S.

*ExtendedDescCount*
Type: UINT

Number of extended plane descriptions provided in the *pExtendedDescs* list.

*pExtendedDescs*
Type: [D3D12XBOX_PRESENT_PLANE_DESC](./d3d12xbox_present_plane_desc.md)*

Optional pointer to the first element in an array of extended plane descriptions. Use the pointer for additional customization of the plane used in presentation.

*Flags*
Type: [D3D12XBOX_PRESENT_PLANE_FLAGS](../enums/d3d12xbox_present_plane_flags.md)

A set of flags used to provide additional control over the plane's composition and display.


## Remarks

Titles can set up two display planes which can be used for compositing UI/HUD onto its scene automatically. In this case compositing is done with the display hardware at no extra GPU, memory or latency costs. For more information on how to set this up, refer to [Hardware overlays and swap resources](../features\graphics\directx/hardware-overlays.md)  


## Requirements

**Header:** d3d12_x.h  

**Supported platforms:** Xbox One family

## See also

[PresentX](../interfaces/id3d12commandqueue/methods/id3d12commandqueue_presentx.md)



