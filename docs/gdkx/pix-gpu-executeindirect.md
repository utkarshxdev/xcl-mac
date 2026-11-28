---
author: coleb
title: "PIX ExecuteIndirect Support overview"
description: "Description of PIX ExecuteIndirect support."
kindex:
- PIX ExecuteIndirect
- PIX, ExecuteIndirect
- ExecuteIndirect (PIX)
ms.author: coleb
ms.topic: article
edited: 05/03/2022
ms.date: '08/01/2022'
permissioned-type: private
---

# PIX ExecuteIndirect Analysis Support

Analysis of ExecuteIndirect calls in PIX poses challenges.  When an ExecuteIndirect argument buffer is GPU generated, the child draws/dispatches of the ExecuteIndirect may have a non-deterministic order.  This can make both performance analysis and debugging operations in PIX very difficult, as the content of an individual child index be completely different each time it is evaluated.

## ExecuteIndirect on Xbox Series X&#124;S

On Xbox Series X&#124;S, ExecuteIndirect analysis support has been updated to significantly improve the PIX user experience.  For an ExecuteIndirect call which may result in more than one draw/dispatch, or which has an indirect specification of non draw/dispatch parameters (setting root signature parameters, vertex buffers, index buffers, pipeline state objects, etc), during analysis PIX will "lock" the GPU generated arguments and other GPU generated dependencies consumed by the ExecuteIndirect to a single copy which is generated when analysis starts, while viewing a child draw/dispatch of the ExecuteIndirect call.  This allows debugging operations, such as viewing [pipeline](../tools\tools-console\pix/pix.md) bindings, to be consistent and not constantly vary due to non-determinism.

However, this "lock" of the GPU generated arguments has a significant impact on the GPU performance.  For this reason, performance information such as [counters](../tools\tools-console\pix/pix.md) and [Dr. PIX](../tools\tools-console\pix/pix.md) results are not available for individual child indices of these "complex" ExecuteIndirect calls.  Performance information is available for the parent ExecuteIndirect, which is selectable by its own unique Global ID in PIX.

This memory "lock" only applies to the children of an ExecuteIndirect call.  For example, if the ExecuteIndirect parent is selected, the entire argument buffer used by all of its children may be viewed in the pipeline view. The content of this buffer may change due to non-determinism of the GPU generated results if refreshed.

## ExecuteIndirect on prior platforms

On consoles prior to Xbox Series X&#124;S, PIX does not support the "lock" for ExecuteIndirect arguments described above.  For these platforms, an option exists in PIX Setting to "force collapse" all ExecuteIndirect calls, effectively treating the ExecuteIndirect as a single draw/dispatch.

## See also

[Pipeline View](../tools\tools-console\pix/pix.md)

[PIX Counters](../tools\tools-console\pix/pix.md)

[Dr. PIX](../tools\tools-console\pix/pix.md)


