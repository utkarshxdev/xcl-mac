---
author: sdgmiller
title: "ExecuteIndirectBundleX (Xbox One family)"
description: "Indirectly executes a bundle from a buffer, where a bundle is a GPU command buffer representation of GPU draws and state setting. This API allows for more efficient creation and management of bundles than can be achieved with the standard ExecuteBundle API."
kindex:
- ID3D12GraphicsCommandList::ExecuteIndirectBundleX
- ExecuteIndirectBundleX
ms.author: mattbron
ms.topic: reference
edited: 07/18/2019
quality: good
ms.date: '08/01/2022'
permissioned-type: secret
---

# ID3D12GraphicsCommandList::ExecuteIndirectBundleX (Xbox One family)  

Indirectly executes a bundle from a buffer, where a bundle is a GPU command buffer representation of GPU draws and state setting. This API allows for more efficient creation and management of bundles than can be achieved with the standard [ExecuteBundle](https://learn.microsoft.com/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-executebundle) Direct3d 12 API.

## Syntax  
  
```cpp
void ExecuteIndirectBundleX(  
         ID3D12Resource* pBundleBuffer,  
         UINT BundleBufferOffset,  
         UINT MaxSize,  
         ID3D12Resource* pSizeBuffer,  
         UINT SizeBufferOffset,  
         UINT PredicationBufferOffset  
)  
```  
  
### Parameters  
  
*pBundleBuffer* &nbsp;&nbsp;\_In\_  
Type: [ID3D12Resource](https://learn.microsoft.com/windows/win32/api/d3d12/nn-d3d12-id3d12resource)*  
  
Specifies a pointer to a buffer resource that contains bundle data. The buffer must have been created with the **D3D12XBOX_RESOURCE_FLAG_ALLOW_INDIRECT_BUFFER** attribute set; otherwise, the buffer must be located in **PAGE_GPU_EXECUTE** memory.  

*BundleBufferOffset* &nbsp;&nbsp;\_In\_  
Type: UINT  
  
Offset, in bytes, from the start of the bundle buffer to the beginning of the bundle data. Must be a multiple of 4.  

*MaxSize* &nbsp;&nbsp;\_In\_  
Type: UINT  
  
The bundle size can be specified in one of two ways:  

1. If *pSizeBuffer* is NULL, then *MaxSize* specifies the exact number of bytes of the bundle to be executed.  

2. If *pSizeBuffer* is not NULL, then *MaxSize* specifies the maximum number of bytes to be executed. If the size specified by *pSizeBuffer* exceeds *MaxSize* at the GPU execution time, then the GPU completely discards the execution of the bundle. This is useful in avoiding a GPU hang when the GPU is used to create the bundle and the data overflows its allotted budget.

Bundles cannot exceed 4 MB in size.  

*pSizeBuffer* &nbsp;&nbsp;\_In\_opt\_  
Type: ID3D12Resource*  
  
Specifies a buffer resource ([ID3D12Resource](https://learn.microsoft.com/windows/win32/api/d3d12/nn-d3d12-id3d12resource)) that contains a 32-bit size, in bytes. This is used for GPU-generated sizes.

*SizeBufferOffset* &nbsp;&nbsp;\_In\_  
Type: UINT  
  
Offset, in bytes, from the start of the size buffer to the 32-bit size, in bytes.  

*PredicationBufferOffset* &nbsp;&nbsp;\_In\_  
Type: UINT  
  
Reserved for future use, and must be zero.  
  
### Return value

Type: void
  
## Remarks  
  
This API enables Build-Your-Own-Bundles functionality whereby a caller can create GPU command buffers by using their own CPU or GPU code, at runtime or offline. The bundle memory can also be explicitly managed. Both of these attributes contrast with the standard **ExecuteBundle** API, where the bundles can be created only at runtime, on the CPU, using D3D APIs, and with a significant object memory overhead.  

Build-Your-Own-Bundles functionality can contain all of the same kinds of draws and state changes as those that can be recorded into regular bundles. This set includes all high-frequency D3D APIs but no low-frequency D3D APIs, such as those for setting render targets or clears. The Xbox One's GPU command processor natively implements these APIs with very straightforward packet encodings. To create the command buffer data, use the D3D12XBOX_PACKET_* defines in the d3d12_x.h header in conjunction with the pipeline state object and Root Signature object introspection. Use the inlined functions in d3d12_x.h as a reference for how to do this; no additional documentation is provided beyond what is in the header.

The packet data for a pipeline state object is good only during the lifetime of that pipeline state object. This means that if a bundle is created offline, then all [SetPipelineState](https://learn.microsoft.com/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-setpipelinestate) invocations within that bundle must be fixed up to reflect the versions of those pipeline state objects created at runtime, before the bundle can be executed by the GPU. Similarly, any pointer data recorded offline into a bundle (such as for descriptor tables) must be fixed up at runtime, before execution.

**ExecuteIndirectBundleX** cannot itself be invoked from a bundle.

Note the use of [NopX](../gc\reference\graphics\d3d12_x\interfaces\id3d12graphicscommandlist/id3d12graphicscommandlist.md) to add no-operation padding to a Command List.

Beginning  with the March 2024 GDK **ExecuteIndirectBundleX** can be used on Async Compute Command Lists on Xbox Series consoles. When used on Async Compute the count and bundle buffers must reside within the same ID3D12Resource; Additionally, the count value must be located 4 bytes before the bundle contents i.e. *BundleBufferOffset* must be equal to *SizeBufferOffset* plus 4 bytes.

To execute a command list before it is closed, refer to [CreateCommandListX](../../id3d12device/methods/id3d12device_createcommandlistx.md) and [CloseBundleX](../gc\reference\graphics\d3d12_x\interfaces\id3d12graphicscommandlist/id3d12graphicscommandlist.md).
  
## Requirements  
  
**Header:** d3d12_x.h
  
***Library:** d3d12_x.lib
  
**Supported platforms:** Xbox One family  
  
## See also

[ID3D12GraphicsCommandList](./id3d12graphicscommandlist_dispatchx.md)  
[ExecuteBundle](https://learn.microsoft.com/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-executebundle)


