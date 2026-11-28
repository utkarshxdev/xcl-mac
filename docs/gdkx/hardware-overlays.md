---
author: sdgmiller
title: "Hardware overlays and swap resources"
description: "Techniques for Game OS developers to efficiently target the Background and HUD planes."
ms.assetid: 10213ab7-0744-5c43-5ecb-be6fba078cb0
kindex:
- Hardware overlays and swap resources
- overlay programming
- swap chains
- swap resources
- hdr
- display planes
- compositing
ms.topic: article
edited: 07/03/2019
ms.date: '08/01/2022'
permissioned-type: private
---


# Hardware overlays and swap resources


As a Game OS developer, you can use the following techniques to efficiently target the background and heads up display (HUD) planes.   

   *  [Overlay overview](#ID4EX)  
   *  [Overlay implementation](#ID4EGC)  
   *  [Performance metrics](#ID4EHE)
   *  [SDR and HDR](#ID4HDR)


<a id="ID4EX"></a>



## Overlay overview  


In graphics programming, the terms *scaler* and *overlay* have been used interchangeably to indicate presentation layers that can be composited by dedicated display hardware. In this topic, we refer to multiplane overlays as simply overlays. If there's only one plane visible, it's usually referred to as the *primary*.   


On Xbox One, there are three hardware overlay planes. Two of these planes are available for title use. A title might use these planes to separate out HUD and background content and have each rendered at different resolutions or updated at different frequencies. Alternatively, the two planes could be used in other ways than separating HUD and background.   


| Overlay| Description|
| --- | --- |
| Background plane| The farthest back (in z-order) of the three overlay planes. This order can't be changed. This plane is dedicated to the Game OS. It's never used by the system OS. This plane is called the background plane because most of the time games use this plane for background game content (as opposed to HUD content). |
| HUD plane| The middle (in z-order) of the three overlay planes. This plane is mostly used by the Game OS, but this plane can also be used for video playback UI by the system OS. This plane is called the HUD plane because most of the time games use this plane for higher resolution but sparser HUD content. |
| System plane| The front-most (in z-order) of the three overlay planes. This plane is dedicated to the system OS. It's never used by the Game OS. |


<a id="ID4EGC"></a>



## Overlay implementation  


On Direct3D for Xbox One, multiplane overlay is implemented as follows.   

   *  When [PresentX](/gaming/xbox-nda-docs/_content/gc/reference/graphics/d3d12_x/interfaces/id3d12commandqueue/methods/id3d12commandqueue_presentx) is called with two new swap resources (that is, not provided in the previous call) on scaler hardware, these resources are presented on the same vblank.   
   *  The color content of the swap resources must be premultiplied alpha.   
   *  There's a maximum 4:1 downscale and 1:16 upscale factor in both vertical and horizontal resolution. If the source presentation rectangle and destination presentation rectangle dimensions differ by more than this factor, the display behavior is undefined at runtime. In debug mode, the runtime checks for an error, and debug text notifies you that the acceptable upscale or downscale factors have been exceeded.   
   *  Both the HUD and background layers can be explicitly enabled or disabled for display. When disabled (omitted from the PresentX call), the layer is black and with an alpha value of 0.   
   *  For each [PresentX](/gaming/xbox-nda-docs/_content/gc/reference/graphics/d3d12_x/interfaces/id3d12commandqueue/methods/id3d12commandqueue_presentx) call, a swap resource can change its source rectangle parameters. This source rectangle must be equal to or be fully within the boundaries of allocated memory. If the source rectangle violates these conditions, the display behavior is undefined in retail builds. In debug mode, the runtime checks for an error and debug text notifies you and specifies the particular condition violated. The dimensions of the source rectangle are specified by [D3D12_RECT](/windows/desktop/direct3d12/d3d12-rect).   
   *  For each [PresentX](/gaming/xbox-nda-docs/_content/gc/reference/graphics/d3d12_x/interfaces/id3d12commandqueue/methods/id3d12commandqueue_presentx) call, a swap resource can change its destination rectangle parameters. This destination rectangle is specified by using [D3D12XBOX_VIEW_RECT](/gaming/xbox-nda-docs/_content/gc/reference/graphics/d3d12_x/structs/d3d12xbox_view_rect), representing the normalized coordinates of the destination rectangle. These floating-point coordinates must be in the range 0.0 to 1.0, inclusive. If these conditions aren't satisfied, the display behavior is undefined at runtime. In debug mode, the runtime checks for an error and debug text notifies you that you've specified an unacceptable (degenerate) destination rectangle or that the destination region is out of bounds.   
   *  There can only be one overlay plane per process. At presentation time, you must specify the hardware plane each swap resource is targeted to (0 or 1 - background or HUD, based on plane parameter index). The runtime returns an error if a swap resource targets a plane that exceeds the available limit.


> [!NOTE]
> Unless multiple views are employed (that is, split-screen multiplayer), the destination rectangle should always be the same size as the game plane, whereas the source rectangle is relative to the swap resource. There's a fixed buffer limit for all 1080p swap resources of 16. A larger number of buffers can be used for jitter absorption.   


The following sample code shows a Game OS app with a 1080p HUD plane and a 720p background plane, both being displayed at 1080p.   

```cpp
    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProperties.CreationNodeMask = D3D12XBOX_NODE_MASK;
    heapProperties.VisibleNodeMask = D3D12XBOX_NODE_MASK;

    D3D12_RESOURCE_DESC swapResourceDesc = {};
    swapResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    swapResourceDesc.Alignment = 0;
    swapResourceDesc.DepthOrArraySize = 1;
    swapResourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapResourceDesc.MipLevels = 1;
    swapResourceDesc.SampleDesc.Count = 1;
    swapResourceDesc.SampleDesc.Quality = 0;
    swapResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    swapResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

    // Create the descriptor for the background plane. Note that it's a lower resolution than HUD.
    swapResourceDesc.Width = 1280;
    swapResourceDesc.Height = 720;

    ID3D12Resource* pSwapPool_BG[2]; // Use two buffers to enable the flip effect.

    // Create the swap resource pool for the background plane.
    for (UINT i = 0; i < 2; ++i)
    {
        HRESULT hr = pd3dDevice->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_ALLOW_DISPLAY,
            &swapResourceDesc,
            D3D12_RESOURCE_STATE_PRESENT,
            nullptr,
            __uuidof(ID3D12Resource),
            (void**)&pSwapPool_BG[i]);

        if (FAILED(hr))
        {
            return hr;
        }
    }

    // Create the descriptor for the HUD plane.
    swapResourceDesc.Width = 1920;
    swapResourceDesc.Height = 1080;

    ID3D12Resource* pSwapPool_HUD[2]; // Use two buffers to enable the flip effect.

    // Create the swap resource pool for the HUD plane.
    for (UINT i = 0; i < 2; ++i)
    {
        HRESULT hr = pd3dDevice->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_ALLOW_DISPLAY,
            &swapResourceDesc,
            D3D12_RESOURCE_STATE_PRESENT,
            nullptr,
            __uuidof(ID3D12Resource),
            (void**)&pSwapPool_HUD[i]);

        if (FAILED(hr))
        {
            return hr;
        }
    }

    // TODO: schedule the frame and wait on its origin event to obtain its identifying token
    D3D12XBOX_FRAME_PIPELINE_TOKEN frameToken;
    {...}

    // TODO: some rendering using the appropriate swap resources as the appropriate render targets
    {...}

    // Prepare to present the swap resources to the display.
    D3D12XBOX_PRESENT_PLANE_PARAMETERS arrayPlaneParams[2] = {};
    D3D12_RECT sourceRect;
    D3D12XBOX_VIEW_RECT destRect;

    sourceRect.left = 0;
    sourceRect.top = 0;

    destRect.left = 0.0f;
    destRect.top = 0.0f;
    destRect.right = 1.0f;
    destRect.bottom = 1.0f;

    // Set up the present plane parameters for the background buffer.
    // Note that we are outputting this plane at 1080p.
    sourceRect.right = 1280;
    sourceRect.bottom = 720;

    arrayPlaneParams[0].Token = token;
    arrayPlaneParams[0].ResourceCount = 1;
    arrayPlaneParams[0].ppResources = &pSwapPool_BG[frameIndex & 1];
    arrayPlaneParams[0].pSrcViewRects = &sourceRect; // Alternatively, nullptr can be specified to derive a full
                                                     // coverage source rect from the dimensions of pSwapPool_BG.
    arrayPlaneParams[0].pDestPlacementBase = &destRect; // Alternatively, nullptr can be specified to derive a full
                                                     // coverage destination rect from the dimensions of the display.
    arrayPlaneParams[0].ColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

    // Set up the present parameters for the HUD buffer.
    // Note that we are outputting this plane at 1080p.
    sourceRect.right = 1920;
    sourceRect.bottom = 1080;

    arrayPlaneParams[1].Token = token;
    arrayPlaneParams[1].ResourceCount = 1;
    arrayPlaneParams[1].ppResources = &pSwapPool_HUD[frameIndex & 1];
    arrayPlaneParams[1].pSrcViewRects = &sourceRect; // Alternatively, nullptr can be specified to derive a full
                                                     // coverage source rect from the dimensions of pSwapPool_BG.
    arrayPlaneParams[1].pDestPlacementBase = &destRect; // Alternatively, nullptr can be specified to derive a full
                                                     // coverage destination rect from the dimensions of the display.
    arrayPlaneParams[1].ColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;

    pd3dCommandQueue->PresentX(
        2,
        arrayPlaneParams,
        nullptr);
```

<a id="ID4EHE"></a>



## Performance metrics  


If you're having trouble hitting your target frame-rate at your target resolution on the GPU, consider dynamically resizing your resolution based on GPU load. The [ID3D12CommandQueue::PresentX](/gaming/xbox-nda-docs/_content/gc/reference/graphics/d3d12_x/interfaces/id3d12commandqueue/methods/id3d12commandqueue_presentx) method allows the scaler resolution settings to be synchronously changed frame-to-frame. Remember that you can have your HUD on one overlay plane and your three-dimensional content on another overlay plane with a different resolution and blended by using per-pixel hardware. If your situation allows, you can also render one plane at a slower update rate by providing the same rendered swap resource in [D3D12XBOX_PRESENT_PLANE_PARAMETERS](/gaming/xbox-nda-docs/_content/gc/reference/graphics/d3d12_x/structs/d3d12xbox_present_plane_parameters) over multiple calls to PresentX.   


To get some metrics on performance, consider using the [GetFrameStatisticsX](/gaming/xbox-nda-docs/_content/gc/reference/graphics/d3d12_x/interfaces/id3d12device/methods/id3d12device_getframestatisticsx) method to obtain timing and other pertinent information about frames that have already been displayed in real time. This information is useful because it allows the app to vary the content of frames rendered in the future to match the latency between calls to [PresentX](/gaming/xbox-nda-docs/_content/gc/reference/graphics/d3d12_x/interfaces/id3d12commandqueue/methods/id3d12commandqueue_presentx) and when the presented frame is actually displayed for viewing. In addition, it's useful for the title to find out if it's queuing more frames than the rate that they can be displayed at, or if it's not generating frames quickly enough.   

<a id="ID4HDR"></a>

## SDR and HDR  

The hardware display planes can be setup with different resolutions, and on {% term scarlett-family %} setup with different color spaces and gamma spaces are also supported. This enables compositing in SDR and HDR, and even a mix of SDR and HDR at the same time.  

For example, the title could render an HDR scene at 1440p at 60Hz in one display plane, while rendering its UI/HUD in the other display plane in SDR at 4K at 30Hz.  

Note that HDR compositing is only supported in the March 2024 GDK and onwards. When a title is in HDR mode and presents two swap buffers, the graphics driver needs to determine what those swap buffers should be used for. One scenario is that the second swap buffer contains the title’s own SDR tone mapped image for GameDVR. The other scenario is that it is meant for compositing. The color space information set in [D3D12XBOX_PRESENT_PLANE_PARAMETERS](/gaming/xbox-nda-docs/_content/gc/reference/graphics/d3d12_x/structs/d3d12xbox_present_plane_parameters) is used by the graphics driver to determine which provided planes’ resources are to be selected for display presentation (HDR) and which are to be used for GameDVR encoding (SDR). When compositing in HDR, the system level auto tone mapping will be used to generate an SDR image for GameDVR.  

HDR compositing is done using 200 nits as paper white for the UI. That is, a value of 1.0 in UI will be displayed at 200 nits. Scene values in the display plane with the scene will have to be normalized as usual, therefore a value of 1.0 in the scene display plane will be displayed at 10,000 nits.  

For SDR compositing, the scene must be in display plane 0 and the UI/HUD in display plane 1. But, for HDR, the scene must be in display plane 1 and the UI/HUD in display plane 0.  

Below are two examples of how to setup [D3D12XBOX_PRESENT_PLANE_PARAMETERS](/gaming/xbox-nda-docs/_content/gc/reference/graphics/d3d12_x/structs/d3d12xbox_present_plane_parameters)  

Example: SDR Compositing  

```cpp
    planeParameters[0].ppResources = sdrSceneSwapBuffer;
    planeParameters[0].ColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
    planeParameters[1].ppResources = uiSwapBuffer;
    planeParameters[1].ColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
```

Example: HDR Compositing with the HDR scene in linear P3 color space and the UI in sRGB Rec.709 color space  

```cpp
    planeParameters[0].ppResources = uiSwapBuffer;
    planeParameters[0].ColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709;
    planeParameters[1].ppResources = hdrSceneSwapBuffer;
    planeParameters[1].ColorSpace = DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_D65P3;
```


<a id="ID4EAG"></a>
