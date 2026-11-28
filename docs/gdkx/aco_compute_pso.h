/*
 * Copyright © 2025 Microsoft Corporation
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef ACO_COMPUTE_PSO_H
#define ACO_COMPUTE_PSO_H

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

#endif

#ifndef ACO_DXBC_HEADER_SIZE
#define ACO_DXBC_HEADER_SIZE 32u
#endif

#define ACO_XPSF_MAGIC        0x58505346u
#define ACO_XPSF_HEADER_SIZE  16u
#define ACO_XPSF_ENTRY_SIZE   16u

#define ACO_XPSF_PACKET    0u
#define ACO_XPSF_METADATA  1u
#define ACO_XPSF_CS_ISA    6u
#define ACO_XPSF_CS_META   7u

#define ACO_CSO_CS_ISA_ALIGN 256u

#ifdef __cplusplus
extern "C" {
#endif

typedef struct aco_blob_span {
   const void *data;
   uint32_t    size;
} aco_blob_span;

typedef struct aco_compute_pso_view {
   aco_blob_span packet;
   aco_blob_span pso_metadata;
   aco_blob_span cs_isa;
   aco_blob_span cs_meta;
} aco_compute_pso_view;

typedef enum aco_cso_result {
   ACO_CSO_OK = 0,
   ACO_CSO_E_NULL,
   ACO_CSO_E_NOT_CONTAINER,
   ACO_CSO_E_NO_ACCC,
   ACO_CSO_E_BAD_XPSF,
   ACO_CSO_E_MISSING_ENTRY
} aco_cso_result;

static inline uint32_t
aco__cso_rd_u32(const uint8_t *p)
{
   return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
          ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static inline void
aco__cso_clear_view(aco_compute_pso_view *v)
{
   if (!v)
      return;
   v->packet.data = NULL;       v->packet.size = 0;
   v->pso_metadata.data = NULL; v->pso_metadata.size = 0;
   v->cs_isa.data = NULL;       v->cs_isa.size = 0;
   v->cs_meta.data = NULL;      v->cs_meta.size = 0;
}

static inline aco_cso_result
aco_cso_find_dxbc_part(const void *blob, uint32_t size, uint32_t fourcc,
                       const void **out_body, uint32_t *out_body_size)
{
   const uint8_t *p;
   uint32_t total, part_count, i;

   if (out_body)
      *out_body = NULL;
   if (out_body_size)
      *out_body_size = 0;

   if (!blob || size < ACO_DXBC_HEADER_SIZE)
      return ACO_CSO_E_NOT_CONTAINER;

   p = (const uint8_t *)blob;
   if (aco__cso_rd_u32(p) != ACO_DXBC_FOURCC_DXBC)
      return ACO_CSO_E_NOT_CONTAINER;

   total      = aco__cso_rd_u32(p + 24);
   part_count = aco__cso_rd_u32(p + 28);

   if (total > size)
      total = size;

   if ((uint64_t)ACO_DXBC_HEADER_SIZE + (uint64_t)part_count * 4u > (uint64_t)total)
      return ACO_CSO_E_NOT_CONTAINER;

   for (i = 0; i < part_count; i++) {
      uint32_t off = aco__cso_rd_u32(p + ACO_DXBC_HEADER_SIZE + i * 4u);
      uint32_t pf, ps;
      if ((uint64_t)off + 8u > (uint64_t)total)
         continue;
      pf = aco__cso_rd_u32(p + off);
      ps = aco__cso_rd_u32(p + off + 4u);
      if ((uint64_t)off + 8u + (uint64_t)ps > (uint64_t)total)
         continue;
      if (pf != fourcc)
         continue;
      if (out_body)
         *out_body = p + off + 8u;
      if (out_body_size)
         *out_body_size = ps;
      return ACO_CSO_OK;
   }
   return ACO_CSO_E_NO_ACCC;
}

static inline aco_cso_result
aco_compute_pso_view_from_xpsf(const void *xpsf, uint32_t size,
                               aco_compute_pso_view *out)
{
   const uint8_t *p;
   uint32_t num_entries, total, i;

   aco__cso_clear_view(out);
   if (!out)
      return ACO_CSO_E_NULL;
   if (!xpsf || size < ACO_XPSF_HEADER_SIZE)
      return ACO_CSO_E_NULL;

   p = (const uint8_t *)xpsf;
   if (aco__cso_rd_u32(p) != ACO_XPSF_MAGIC)
      return ACO_CSO_E_NOT_CONTAINER;

   num_entries = aco__cso_rd_u32(p + 8);
   total       = aco__cso_rd_u32(p + 12);
   if (total > size)
      total = size;

   {
      uint64_t payload_base = (uint64_t)ACO_XPSF_HEADER_SIZE +
                              (uint64_t)num_entries * ACO_XPSF_ENTRY_SIZE;
      if (payload_base > (uint64_t)total)
         return ACO_CSO_E_BAD_XPSF;

      for (i = 0; i < num_entries; i++) {
         const uint8_t *e = p + ACO_XPSF_HEADER_SIZE + (uint64_t)i * ACO_XPSF_ENTRY_SIZE;
         uint32_t type = aco__cso_rd_u32(e);
         uint32_t off  = aco__cso_rd_u32(e + 4);
         uint32_t esz  = aco__cso_rd_u32(e + 8);
         aco_blob_span span;

         if ((uint64_t)off < payload_base ||
             (uint64_t)off + (uint64_t)esz > (uint64_t)total)
            continue;

         span.data = p + off;
         span.size = esz;

         switch (type) {
         case ACO_XPSF_PACKET:   out->packet = span;       break;
         case ACO_XPSF_METADATA: out->pso_metadata = span; break;
         case ACO_XPSF_CS_ISA:   out->cs_isa = span;       break;
         case ACO_XPSF_CS_META:  out->cs_meta = span;      break;
         default:                 break;
         }
      }
   }

   if (!out->packet.data || out->packet.size == 0 ||
       !out->cs_isa.data || out->cs_isa.size == 0)
      return ACO_CSO_E_MISSING_ENTRY;
   return ACO_CSO_OK;
}

static inline aco_cso_result
aco_compute_pso_view_from_cso(const void *blob, uint32_t size,
                              aco_compute_pso_view *out)
{
   uint32_t magic;

   aco__cso_clear_view(out);
   if (!out)
      return ACO_CSO_E_NULL;
   if (!blob || size < ACO_XPSF_HEADER_SIZE)
      return ACO_CSO_E_NULL;

   magic = aco__cso_rd_u32((const uint8_t *)blob);

   if (magic == ACO_XPSF_MAGIC)
      return aco_compute_pso_view_from_xpsf(blob, size, out);

   if (magic == ACO_DXBC_FOURCC_DXBC) {
      const void *xpsf = NULL;
      uint32_t xpsf_size = 0;
      aco_cso_result r =
         aco_cso_find_dxbc_part(blob, size, ACO_DXBC_FOURCC_ACCC,
                                &xpsf, &xpsf_size);
      if (r != ACO_CSO_OK)
         return r;
      return aco_compute_pso_view_from_xpsf(xpsf, xpsf_size, out);
   }

   return ACO_CSO_E_NOT_CONTAINER;
}

static inline const char *
aco_cso_result_str(aco_cso_result r)
{
   switch (r) {
   case ACO_CSO_OK:              return "ok";
   case ACO_CSO_E_NULL:          return "null or undersized input";
   case ACO_CSO_E_NOT_CONTAINER: return "not a DXBC container or XPSF blob";
   case ACO_CSO_E_NO_ACCC:       return "no 'ACCC' compute cook part in container";
   case ACO_CSO_E_BAD_XPSF:      return "malformed XPSF (out-of-bounds directory)";
   case ACO_CSO_E_MISSING_ENTRY: return "XPSF missing a required compute entry";
   default:                      return "unknown";
   }
}

#ifdef ACO_CSO_WITH_D3D12

static inline void
aco_cso_fill_deserialize_compute(const aco_compute_pso_view *v,
                                 D3D12XBOX_DESERIALIZE_COMPUTE_PIPELINE_STATE *ds)
{
   ds->pPacket             = (void *)v->packet.data;
   ds->PacketSize          = v->packet.size;
   ds->pMetaData           = (void *)v->pso_metadata.data;
   ds->MetaDataSize        = v->pso_metadata.size;
   ds->CS.pMetaData        = (void *)v->cs_meta.data;
   ds->CS.MetaDataSize     = v->cs_meta.size;
   ds->CS.GpuInstructionsSize = v->cs_isa.size;
}
#endif

#ifdef __cplusplus
}
#endif

#endif
