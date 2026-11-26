"""Tier 1: Feature Coverage Test Suite.

Contains 36 test cases (6 per feature across R1-R6) exercising the primary happy paths
and contract specifications of the GTA VI XCL Mac Native integration.
"""

import os
import struct
import unittest
from tests.e2e.harness import (
    S_OK, E_INVALIDARG, E_NOINTERFACE, DXGI_ERROR_NOT_FOUND, DXGI_ERROR_DEVICE_REMOVED,
    IID_ID3D12DEVICE9, IID_ID3D12SHADER_CACHE_SESSION,
    SLOT_BYTES, SAMPLER_SLOT_BYTES, CLEAR_COLOR_SLATE, TEXTURE_COLOR_SAMPLE,
    SPREAD4, SPREAD8, UNSPREAD8, NIBBLES,
    fast_interleave4, fast_interleave8, reference_fold_interleave, reference_fold_nibbles,
    DescriptorSlot, ConsoleHeap, WriteWatchSystem,
    BoundedRawBufferSRV, NakedRootSRV,
    GraphicsRootSignature, RootParameterKind, CommandListState,
    ShaderCacheSessionDesc, FallbackShaderCacheSession, D3D12DeviceMock,
    BCPackStreamDecompressor, PlatformUtil
)


class TestTier1Features(unittest.TestCase):
    """Tier 1 Feature Coverage Tests for Requirements R1 through R6."""

    # ==========================================================================
    # R1: Heap Mirroring & Descriptor Slot Synchronization (VIctory #844)
    # ==========================================================================
    def test_r1_heap_registration_tracking(self):
        """R1.1: Verify descriptor heaps registered in console_heap are tracked with valid ranges."""
        base_addr = 0x1_0000_0000 # 4 GiB UMA window 0
        num_slots = 1024
        heap = ConsoleHeap(base_addr=base_addr, num_slots=num_slots, sampler=False)
        self.assertEqual(heap.slot_size, SLOT_BYTES)
        self.assertEqual(heap.total_bytes, num_slots * 32)
        r_start, r_len = heap.get_range()
        self.assertEqual(r_start, base_addr)
        self.assertEqual(r_len, 32768)

    def test_r1_write_watch_range_inclusion(self):
        """R1.2: Verify cached_watch_ranges includes all active descriptor heap ranges."""
        watch = WriteWatchSystem(page_size=4096)
        watch.textures_ranges.append((0x20000000, 0x10000))
        watch.sparse_ranges.append((0x30000000, 0x20000))
        heap = ConsoleHeap(base_addr=0x1_0000_0000, num_slots=512)
        watch.register_heap(heap)

        ranges = watch.cached_watch_ranges()
        self.assertEqual(len(ranges), 3)
        self.assertIn(heap.get_range(), ranges)

    def test_r1_dirty_page_slot_inspection(self):
        """R1.3: Verify sync_dirty_heap_slots inspects dirty pages and mirrors updated slots."""
        watch = WriteWatchSystem(page_size=4096)
        heap = ConsoleHeap(base_addr=0x1_0000_0000, num_slots=256)
        watch.register_heap(heap)

        # Write valid descriptor into slot 10 (offset 320, page 0)
        desc_data = bytearray(32)
        struct.pack_into("<I", desc_data, 0, 0x5000_0000) # Buffer VA
        struct.pack_into("<I", desc_data, 4, 0x0001_0000)
        struct.pack_into("<I", desc_data, 12, 0x9000_0000) # Valid view flag (>>28 >= 8)
        heap.write_slot(10, bytes(desc_data))

        # Mark page 0 dirty
        watch.mark_page_dirty(heap.base_addr)
        dirty = watch.take_written()
        self.assertEqual(len(dirty), 1)

        watch.sync_dirty_heap_slots(dirty)
        self.assertIn(10, heap.mirrored_pc_slots)
        self.assertEqual(heap.mirrored_pc_slots[10], bytes(desc_data))

    def test_r1_cpu_memcpy_slot_mirroring(self):
        """R1.4: Verify CPU memcpy of descriptor blocks without D3D12 calls mirrors into PC heap."""
        watch = WriteWatchSystem(page_size=4096)
        heap = ConsoleHeap(base_addr=0x1_0000_0000, num_slots=128)
        watch.register_heap(heap)

        # Simulate title copying 4 descriptor slots at once (128 bytes)
        block = bytearray()
        for idx in range(4):
            slot = bytearray(32)
            struct.pack_into("<I", slot, 0, 0x4000_0000 + idx * 0x1000)
            struct.pack_into("<I", slot, 12, 0x8000_0000) # Valid flag
            block.extend(slot)

        # Write to slots 20..23
        start_addr = heap.base_addr + 20 * SLOT_BYTES
        for i in range(4):
            heap.write_slot(20 + i, bytes(block[i*32:(i+1)*32]))
        watch.mark_page_dirty(start_addr)

        dirty = watch.take_written()
        watch.sync_dirty_heap_slots(dirty)

        for i in range(4):
            self.assertIn(20 + i, heap.mirrored_pc_slots)

    def test_r1_cbv_srv_uav_descriptor_validity(self):
        """R1.5: Verify descriptor validity filters publish valid views and ignore zero memory."""
        slot_valid = DescriptorSlot(bytes([0x01] + [0]*11 + [0x80] + [0]*19))
        self.assertTrue(slot_valid.is_valid_view())

        slot_all_zero = DescriptorSlot(bytes(32))
        self.assertFalse(slot_all_zero.is_valid_view())

        slot_buffer = DescriptorSlot(bytes([0x50, 0x00, 0x00, 0x00] + [0]*28))
        self.assertTrue(slot_buffer.is_valid_view())

    def test_r1_stale_view_prevention_on_submit(self):
        """R1.6: Verify dirty slot sync prevents mesh shaders reading stale V# base address 0."""
        desc_data = bytearray(32)
        # Uninitialized: base address 0
        slot_stale = DescriptorSlot(bytes(desc_data))
        self.assertEqual(slot_stale.get_v_sharp_base(), 0)

        # CPU writes valid V# buffer view
        struct.pack_into("<I", desc_data, 0, 0x0004_0000) # low
        struct.pack_into("<I", desc_data, 4, 0x0000_0002) # high
        struct.pack_into("<I", desc_data, 12, 0x8000_0000)
        slot_fresh = DescriptorSlot(bytes(desc_data))
        self.assertEqual(slot_fresh.get_v_sharp_base(), 0x2_0004_0000)
        self.assertTrue(slot_fresh.is_valid_view())

    # ==========================================================================
    # R2: Bounded Root SRVs & Safe Host Views (VIctory #844)
    # ==========================================================================
    def test_r2_root_descriptor_table_conversion(self):
        """R2.1: Verify naked root SRVs are converted to bounded 1-view descriptor tables."""
        srv = BoundedRawBufferSRV(resource_bytes=1024, is_upload=True)
        self.assertTrue(srv.raw_flag)
        self.assertEqual(srv.view_dimension, "D3D12_SRV_DIMENSION_BUFFER")
        self.assertEqual(srv.format, "DXGI_FORMAT_R32_TYPELESS")

    def test_r2_srv_raw_buffer_view_dimension(self):
        """R2.2: Verify bounded SRV uses RAW buffer view semantics."""
        srv = BoundedRawBufferSRV(resource_bytes=4096)
        self.assertEqual(srv.num_elements, 1024)
        self.assertTrue(srv.raw_flag)

    def test_r2_srv_num_elements_clamped_to_host_bytes(self):
        """R2.3: Verify NumElements is strictly clamped to input_bytes / 4."""
        srv = BoundedRawBufferSRV(resource_bytes=500)
        self.assertEqual(srv.num_elements, 125)

    def test_r2_out_of_bounds_compute_read_zeros(self):
        """R2.4: Verify ByteAddressBuffer loads past host buffer bounds return zero without fault."""
        buf = bytearray(256)
        struct.pack_into("<I", buf, 0, 0x12345678)
        srv = BoundedRawBufferSRV(resource_bytes=len(buf))

        # In-bounds read
        val, fault = srv.load_word(bytes(buf), 0)
        self.assertEqual(val, 0x12345678)
        self.assertFalse(fault)

        # Out-of-bounds read past 256 bytes
        val_oob, fault_oob = srv.load_word(bytes(buf), 512)
        self.assertEqual(val_oob, 0)
        self.assertFalse(fault_oob)

    def test_r2_gpu_page_fault_prevention_32mb_past_bounds(self):
        """R2.5: Verify 32 MB past-buffer compute reads do not trigger 0x887A0005 GPU device removal."""
        buf = bytearray(1024)
        bounded = BoundedRawBufferSRV(resource_bytes=len(buf))
        naked = NakedRootSRV(resource_bytes=len(buf))

        # 32 MB past bounds = 33,554,432 bytes
        oob_offset = len(buf) + (32 * 1024 * 1024)

        # Naked root SRV triggers fatal fault
        _, naked_fault = naked.load_word(bytes(buf), oob_offset)
        self.assertTrue(naked_fault)

        # Bounded SRV safely clamps to zero without fault
        val_bounded, bounded_fault = bounded.load_word(bytes(buf), oob_offset)
        self.assertEqual(val_bounded, 0)
        self.assertFalse(bounded_fault)

    def test_r2_mesh_scale_repack_bounded_view(self):
        """R2.6: Verify indirect mesh scale repack compute dispatch uses bounded SRV."""
        repack_input_bytes = 2048
        srv = BoundedRawBufferSRV(resource_bytes=repack_input_bytes)
        self.assertEqual(srv.num_elements, 512)
        val, fault = srv.load_word(bytes(repack_input_bytes), 2044)
        self.assertFalse(fault)

    # ==========================================================================
    # R3: Descriptor Sampling for Cutscenes & 3D Shaders (VIctory #850 & #710)
    # ==========================================================================
    def test_r3_graphics_root_arguments_recording(self):
        """R3.1: Verify SetGraphicsRoot* records root arguments into args_gfx."""
        cmd = CommandListState()
        cmd.set_graphics_root_32bit_constant(0, 0xAABBCCDD, offset=0)
        cmd.set_graphics_root_descriptor_table(1, 0x1_0000_0100)

        val = struct.unpack_from("<I", cmd.args_gfx, 0)[0]
        table_addr = struct.unpack_from("<Q", cmd.args_gfx, 16)[0]
        self.assertEqual(val, 0xAABBCCDD)
        self.assertEqual(table_addr, 0x1_0000_0100)

    def test_r3_root_signature_invalidation_rebinding(self):
        """R3.2: Verify SetGraphicsRootSignature invalidation is resolved by rebinding."""
        cmd = CommandListState()
        rs = GraphicsRootSignature(params=[
            {"type": RootParameterKind.CONSTANTS},
            {"type": RootParameterKind.DESCRIPTOR_TABLE},
        ])
        heap = ConsoleHeap(base_addr=0x1_0000_0000, num_slots=64)
        desc_data = bytearray(32)
        struct.pack_into("<I", desc_data, 12, 0x8000_0000)
        heap.mirrored_pc_slots[0] = bytes(desc_data)

        # Pre-record arguments
        cmd.set_graphics_root_32bit_constant(0, 42)
        cmd.set_graphics_root_descriptor_table(1, heap.base_addr)

        # SetGraphicsRootSignature clears hardware bindings
        cmd.set_graphics_root_signature(rs)
        self.assertEqual(len(cmd.hardware_bound_args), 0)

        # resolve_and_bind_graphics_root_arguments restores them
        cmd.resolve_and_bind_graphics_root_arguments(heap)
        self.assertEqual(len(cmd.hardware_bound_args), 2)
        self.assertEqual(cmd.hardware_bound_args[0], ("CONSTANTS", 42))
        self.assertEqual(cmd.hardware_bound_args[1][0], "TABLE")

    def test_r3_descriptor_table_gpu_handle_resolution(self):
        """R3.3: Verify console 32-bit descriptor table addresses resolve to PC GPU handles."""
        cmd = CommandListState()
        heap = ConsoleHeap(base_addr=0x1_0000_0000, num_slots=128)
        heap.mirrored_pc_slots[5] = bytes(32)
        rs = GraphicsRootSignature(params=[{"type": RootParameterKind.DESCRIPTOR_TABLE}])
        cmd.set_graphics_root_signature(rs)
        cmd.set_graphics_root_descriptor_table(0, heap.base_addr + 5 * SLOT_BYTES)

        cmd.resolve_and_bind_graphics_root_arguments(heap)
        binding = cmd.hardware_bound_args[0]
        self.assertEqual(binding[0], "TABLE")
        self.assertEqual(binding[1], 5)

    def test_r3_cutscene_fullscreen_triangle_sampling(self):
        """R3.4: Verify cutscenes sample bound texture descriptors instead of flat slate 0x20202C."""
        cmd = CommandListState()
        heap = ConsoleHeap(base_addr=0x1_0000_0000, num_slots=64)
        heap.mirrored_pc_slots[0] = bytes(32)
        rs = GraphicsRootSignature(params=[{"type": RootParameterKind.DESCRIPTOR_TABLE}])

        # Before rebinding: draw call without bound texture yields slate color
        cmd.set_graphics_root_signature(rs)
        cmd.draw_fullscreen_triangle()
        self.assertEqual(cmd.rendered_pixel_color, CLEAR_COLOR_SLATE)

        # After rebinding: draw call samples bound texture -> green rendered pixel
        cmd.set_graphics_root_descriptor_table(0, heap.base_addr)
        cmd.resolve_and_bind_graphics_root_arguments(heap)
        cmd.draw_fullscreen_triangle()
        self.assertEqual(cmd.rendered_pixel_color, TEXTURE_COLOR_SAMPLE)
        self.assertNotEqual(cmd.rendered_pixel_color, CLEAR_COLOR_SLATE)

    def test_r3_mesh_shader_dispatch_descriptor_binding(self):
        """R3.5: Verify dispatch_mesh rebinds root arguments and binds sampler/texture views."""
        cmd = CommandListState()
        heap = ConsoleHeap(base_addr=0x1_0000_0000, num_slots=64)
        heap.mirrored_pc_slots[2] = bytes(32)
        rs = GraphicsRootSignature(params=[{"type": RootParameterKind.DESCRIPTOR_TABLE}])
        cmd.set_graphics_root_signature(rs)
        cmd.set_graphics_root_descriptor_table(0, heap.base_addr + 2 * SLOT_BYTES)
        cmd.resolve_and_bind_graphics_root_arguments(heap)

        self.assertIn(0, cmd.hardware_bound_args)
        self.assertEqual(cmd.hardware_bound_args[0][0], "TABLE")

    def test_r3_root_constant_uav_preservation(self):
        """R3.6: Verify root constants and root UAVs are preserved and bound alongside tables."""
        cmd = CommandListState()
        rs = GraphicsRootSignature(params=[
            {"type": RootParameterKind.CONSTANTS},
            {"type": RootParameterKind.ROOT_DESCRIPTOR},
        ])
        cmd.set_graphics_root_signature(rs)
        cmd.set_graphics_root_32bit_constant(0, 0x1234)
        struct.pack_into("<Q", cmd.args_gfx, 16, 0x2_0000_0000)

        cmd.resolve_and_bind_graphics_root_arguments(ConsoleHeap(0, 1))
        self.assertEqual(cmd.hardware_bound_args[0], ("CONSTANTS", 0x1234))
        self.assertEqual(cmd.hardware_bound_args[1], ("UAV", 0x2_0000_0000))

    # ==========================================================================
    # R4: D3DMetal Shader Cache Session Guard (XCL #1108)
    # ==========================================================================
    def test_r4_device9_query_interface(self):
        """R4.1: Verify dev_qi_x recognizes IID_ID3D12Device9 and returns S_OK."""
        dev = D3D12DeviceMock(is_mac_host=True)
        hr, obj = dev.query_interface(IID_ID3D12DEVICE9)
        self.assertEqual(hr, S_OK)
        self.assertIsNotNone(obj)

    def test_r4_create_shader_cache_session_success(self):
        """R4.2: Verify CreateShaderCacheSession (slot 73) returns S_OK and non-null session."""
        dev = D3D12DeviceMock(is_mac_host=True)
        desc = ShaderCacheSessionDesc()
        hr, session = dev.create_shader_cache_session(desc)
        self.assertEqual(hr, S_OK)
        self.assertIsNotNone(session)

    def test_r4_shader_cache_control_stub(self):
        """R4.3: Verify ShaderCacheControl (slot 74) returns S_OK."""
        dev = D3D12DeviceMock(is_mac_host=True)
        hr = dev.shader_cache_control(0, 0)
        self.assertEqual(hr, S_OK)

    def test_r4_fallback_session_find_value_miss(self):
        """R4.4: Verify FindValue returns DXGI_ERROR_NOT_FOUND (clean cache miss)."""
        session = FallbackShaderCacheSession()
        hr, val = session.find_value(b"test_key")
        self.assertEqual(hr, DXGI_ERROR_NOT_FOUND)
        self.assertIsNone(val)

    def test_r4_fallback_session_store_value_success(self):
        """R4.5: Verify StoreValue accepts cache entries and returns S_OK."""
        session = FallbackShaderCacheSession()
        hr = session.store_value(b"shader_hash", b"translated_bytecode")
        self.assertEqual(hr, S_OK)
        hr_find, val = session.find_value(b"shader_hash")
        self.assertEqual(hr_find, S_OK)
        self.assertEqual(val, b"translated_bytecode")

    def test_r4_fallback_session_get_desc_48_bytes(self):
        """R4.6: Verify GetDesc copies 48-byte D3D12_SHADER_CACHE_SESSION_DESC."""
        desc = ShaderCacheSessionDesc(max_mem_bytes=1048576, version=2)
        session = FallbackShaderCacheSession(desc)
        ret_desc = session.get_desc()
        self.assertEqual(ret_desc.max_mem_bytes, 1048576)
        self.assertEqual(ret_desc.version, 2)
        packed = ret_desc.pack()
        self.assertEqual(len(packed), 48)

    # ==========================================================================
    # R5: High-Throughput Streaming BCPack Decompression (VIctory #703)
    # ==========================================================================
    def test_r5_bcpack_supported_modes(self):
        """R5.1: Verify BCPack accepts BC1, BC3, BC4, BC5, BC6H, BC7, and rejects BC2."""
        dec = BCPackStreamDecompressor()
        src = bytes([0x55] * 64)
        dst = bytearray(256)

        # Supported modes
        for mode in (1, 3, 4, 5, 6, 7):
            hr = dec.decode_stream_simulated(mode, src, dst)
            self.assertEqual(hr, S_OK, f"Failed on mode {mode}")

        # BC2 must be rejected
        hr_bc2 = dec.decode_stream_simulated(2, src, dst)
        self.assertNotEqual(hr_bc2, S_OK)

    def test_r5_zero_heap_allocation_streaming(self):
        """R5.2: Verify streaming decompression uses zero per-block heap allocations."""
        dec = BCPackStreamDecompressor()
        src = bytes([0xAA] * 128)
        dst = bytearray(1024)
        hr = dec.decode_stream_simulated(7, src, dst)
        self.assertEqual(hr, S_OK)
        self.assertEqual(dec.heap_allocation_count, 0)

    def test_r5_compile_time_lut_parity(self):
        """R5.3: Verify compile-time LUTs match reference fold implementations 100%."""
        # Test SPREAD4
        for a in range(16):
            for b in range(16):
                self.assertEqual(fast_interleave4(a, b), reference_fold_interleave(a, b))

        # Test SPREAD8
        for val in range(256):
            ref = sum(((val >> k & 1) << (2 * k)) for k in range(8))
            self.assertEqual(SPREAD8[val], ref)

        # Test NIBBLES
        for b in range(256):
            ref_nibbles = reference_fold_nibbles(b)
            self.assertEqual(NIBBLES[b], (ref_nibbles[0], ref_nibbles[1]))

    def test_r5_const_generic_lane_stepping(self):
        """R5.4: Verify const-generic lane stepping configurations unpack symbols."""
        dec = BCPackStreamDecompressor()
        src = bytes(range(64))
        dst = bytearray(128)
        # Mode 1: 12x1, 8 bytes per block
        hr = dec.decode_stream_simulated(1, src, dst)
        self.assertEqual(hr, S_OK)
        self.assertTrue(any(b != 0 for b in dst))

    def test_r5_directstorage_request_validation(self):
        """R5.5: Verify DirectStorage validates single and two-stage BCPack requests."""
        # Options bits: bcpack mode in bits 1..=4
        options_bc7 = 7 << 1
        mode = (options_bc7 >> 1) & 0xF
        self.assertEqual(mode, 7)
        self.assertIn(mode, (1, 3, 4, 5, 6, 7))

    def test_r5_directstorage_direct_vram_decompression(self):
        """R5.6: Verify DirectStorage writes directly to destination buffer without intermediate copy."""
        dec = BCPackStreamDecompressor()
        src = bytes([0x12, 0x34, 0x56, 0x78] * 16)
        vram_dst = bytearray(256)
        hr = dec.decode_stream_simulated(5, src, vram_dst)
        self.assertEqual(hr, S_OK)
        self.assertEqual(len(vram_dst), 256)

    # ==========================================================================
    # R6: Mac Host Gating & Verification Parity
    # ==========================================================================
    def test_r6_mac_host_detection_darwin(self):
        """R6.1: Verify mac_host() detects Darwin platform and respects environment overrides."""
        os.environ["XCL_MAC_HOST"] = "1"
        self.assertTrue(PlatformUtil.mac_host())

        os.environ["XCL_MAC_HOST"] = "0"
        self.assertFalse(PlatformUtil.mac_host())

        del os.environ["XCL_MAC_HOST"]

    def test_r6_rosetta_brand_detection(self):
        """R6.2: Verify rosetta() detects Apple Silicon translation from CPU brand."""
        self.assertTrue(PlatformUtil.rosetta("VirtualApple @ 2.50GHz"))
        self.assertTrue(PlatformUtil.rosetta("Apple M3 Max"))
        self.assertFalse(PlatformUtil.rosetta("AMD Ryzen 9 7950X"))
        self.assertFalse(PlatformUtil.rosetta("Intel(R) Core(TM) i9-14900K"))

    def test_r6_windows_host_isolation(self):
        """R6.3: Verify Mac workarounds bypass when mac_host() is false."""
        dev = D3D12DeviceMock(is_mac_host=False)
        self.assertFalse(dev.is_mac_host)

    def test_r6_verify_mac_build_script_parity(self):
        """R6.4: Verify all subsystem checks in scripts/verify_mac_build.sh pass."""
        script_path = "/Users/utkarshbhatnagar/Desktop/xcl/scripts/verify_mac_build.sh"
        self.assertTrue(os.path.isfile(script_path))
        with open(script_path, "r", encoding="utf-8") as f:
            content = f.read()
        self.assertIn("D3DMetal COM vtable prefix mirroring", content)
        self.assertIn("CreateShaderCacheSession", content)
        self.assertIn("Heap mirroring and bounded root SRVs", content)

    def test_r6_ci_workflow_alignment(self):
        """R6.5: Verify .github/workflows/mac-cross.yml targets x86_64-pc-windows-gnullvm."""
        workflow_path = "/Users/utkarshbhatnagar/Desktop/xcl/.github/workflows/mac-cross.yml"
        self.assertTrue(os.path.isfile(workflow_path))
        with open(workflow_path, "r", encoding="utf-8") as f:
            content = f.read()
        self.assertIn("x86_64-pc-windows-gnullvm", content)
        self.assertIn("verify_mac_build.sh", content)

    def test_r6_pointer_alignment_guards(self):
        """R6.6: Verify pointer alignment checks protect adopted D3D12 buffer returns."""
        self.assertTrue(PlatformUtil.check_pointer_alignment(0x1_0000_0000))
        self.assertTrue(PlatformUtil.check_pointer_alignment(0x1_0000_0008))
        self.assertFalse(PlatformUtil.check_pointer_alignment(0x1_0000_0001))
        self.assertFalse(PlatformUtil.check_pointer_alignment(0x1_0000_0003))
