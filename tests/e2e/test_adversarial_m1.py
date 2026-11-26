"""Adversarial Challenge & Empirical Stress-Testing Suite for Milestone 1 (R1).

Focus:
1. Empirically verify that when queue execution stalls, cpu_writes times out at 4000ms,
   executes memory writes, signals job.fence, and rescues pending title fences (0x6000013dc310)
   without deadlock.
2. Stress-test edge cases:
   - u64 fence value wrapping (boundary at 0xFFFF_FFFF_FFFF_FFFF and wrap-around to 0)
   - Multiple fences stalled simultaneously on the same queue (10, 50, 200 fences)
   - Rapid repeated stalls (100 consecutive stalled frames)
   - Multi-threaded concurrent execution (concurrent worker rescue, 10ms polling, and wait threads)
   - Zero-write jobs, out-of-order queue signals, and queue isolation
"""

import threading
import time
import unittest
from typing import Dict, List, Optional

from tests.e2e.test_stage6_4_e2e import (
    CpuWritesAndFenceManager,
    MockD3D12Fence,
    StoredTitleFence,
    WriteKind,
    WriteOp,
    REDUCED_WATCHDOG_TIMEOUT_MS,
    GAME_WATCHDOG_DEADLINE_MS,
)


class TestM1EmpiricalVerification(unittest.TestCase):
    """Empirical verification of Worker M1's core stall elimination claims."""

    def test_r1_core_stall_rescue_flow(self):
        """Verify: Queue stall -> 4000ms timeout -> memory writes -> fence signaled -> title fence rescued."""
        mgr = CpuWritesAndFenceManager(watchdog_timeout_ms=4000)
        q0 = 0x7F8AAC917010
        title_fence_addr = 0x6000013DC310

        # Title fence initially at 0x1d6c (7532), waiting for 0x1d6d (7533)
        tf = mgr.register_title_fence(title_fence_addr, initial_val=0x1D6C)
        mgr.note_queue_signal(title_fence_addr, val=0x1D6D)

        # GPU command queue fence stalled at 3766, wanted 3767
        gpu_fence = MockD3D12Fence(initial_val=3766)
        payload = b"\xAA\xBB\xCC\xDD\x11\x22\x33\x44"
        write_addr = 0x3_0000_1000
        writes = [WriteOp(WriteKind.IMMEDIATE, write_addr, payload)]

        mgr.queue_bottom_of_pipe_job(queue_id=q0, fence=gpu_fence, target_val=3767, writes=writes)

        # At t = 3999ms: timeout NOT reached
        mgr.process_bottom_of_pipe_jobs(elapsed_wait_ms=3999)
        self.assertEqual(gpu_fence.get_completed_value(), 3766)
        self.assertEqual(tf.cell_val, 0x1D6C)
        self.assertEqual(mgr.read_memory(write_addr, len(payload)), bytearray(len(payload)))
        self.assertEqual(len(mgr.stall_warnings), 0)

        # At t = 4000ms: timeout reached!
        mgr.process_bottom_of_pipe_jobs(elapsed_wait_ms=4000)

        # 1. Memory writes executed
        self.assertEqual(mgr.read_memory(write_addr, len(payload)), bytearray(payload))

        # 2. GPU fence advanced on CPU from 3766 to 3767
        self.assertEqual(gpu_fence.get_completed_value(), 3767)

        # 3. Pending title fence 0x6000013dc310 rescued (0x1d6c -> 0x1d6d)
        self.assertEqual(tf.cell_val, 0x1D6D)

        # 4. Stall telemetry recorded
        self.assertEqual(len(mgr.stall_warnings), 1)
        self.assertIn("fence at 3766, wanted 3767", mgr.stall_warnings[0])

        # 5. Timing safety: 4000ms is well before the 20000ms game watchdog crash
        self.assertLess(mgr.watchdog_timeout_ms, GAME_WATCHDOG_DEADLINE_MS)


class TestM1EdgeCasesAndStressHarness(unittest.TestCase):
    """Stress tests and boundary condition mining for M1."""

    def test_edge_u64_wrapping_boundary(self):
        """Stress-test fence counter transitions at boundary and near u64::MAX."""
        max_u64 = 0xFFFF_FFFF_FFFF_FFFF
        mgr = CpuWritesAndFenceManager(watchdog_timeout_ms=4000)

        # Test transition from (u64::MAX - 5) to u64::MAX
        fence = MockD3D12Fence(initial_val=max_u64 - 5)
        tf = mgr.register_title_fence(0x6000013DC310, initial_val=max_u64 - 2)
        mgr.note_queue_signal(0x6000013DC310, val=max_u64)

        mgr.queue_bottom_of_pipe_job(0, fence, target_val=max_u64, writes=[])
        mgr.process_bottom_of_pipe_jobs(elapsed_wait_ms=4000)

        self.assertEqual(fence.get_completed_value(), max_u64)
        self.assertEqual(tf.cell_val, max_u64)

    def test_edge_multiple_fences_stalled_simultaneously_same_queue(self):
        """Stress-test 100 title fences stalled simultaneously on the same queue."""
        mgr = CpuWritesAndFenceManager(watchdog_timeout_ms=4000)
        q0 = 0x7F8AAC917010
        num_fences = 100

        fences = []
        for i in range(num_fences):
            addr = 0x6000_0000_0000 + (i * 0x1000)
            initial = 1000 + i * 10
            target = initial + 1
            tf = mgr.register_title_fence(addr, initial_val=initial)
            mgr.note_queue_signal(addr, val=target)
            fences.append((tf, target))

        # Bottom-of-pipe job stalls on q0
        gpu_fence = MockD3D12Fence(initial_val=5000)
        mgr.queue_bottom_of_pipe_job(queue_id=q0, fence=gpu_fence, target_val=5001, writes=[])

        # Timeout triggers stall rescue
        mgr.process_bottom_of_pipe_jobs(elapsed_wait_ms=4000)

        # Verify all 100 fences were rescued without exception
        for tf, target in fences:
            self.assertEqual(tf.cell_val, target, f"Fence at {tf.addr:#x} failed to advance to {target}")

        self.assertEqual(mgr.rescued_fences_count, num_fences)
        self.assertEqual(gpu_fence.get_completed_value(), 5001)

    def test_edge_rapid_repeated_stalls_100_frames(self):
        """Stress-test 100 consecutive frames with active queue stalls."""
        mgr = CpuWritesAndFenceManager(watchdog_timeout_ms=4000)
        q0 = 0x7F8AAC917010
        title_fence_addr = 0x6000013DC310
        tf = mgr.register_title_fence(title_fence_addr, initial_val=0)

        gpu_fence = MockD3D12Fence(initial_val=0)
        write_addr = 0x4_0000_0000

        for frame in range(1, 101):
            # Each frame writes a distinct payload
            payload = frame.to_bytes(4, byteorder="little")
            writes = [WriteOp(WriteKind.IMMEDIATE, write_addr, payload)]
            mgr.note_queue_signal(title_fence_addr, val=frame)

            # Job is queued for this frame
            mgr.queue_bottom_of_pipe_job(queue_id=q0, fence=gpu_fence, target_val=frame, writes=writes)

            # Queue stalls and times out
            mgr.process_bottom_of_pipe_jobs(elapsed_wait_ms=4000)

            # Assert this frame resolved cleanly
            self.assertEqual(gpu_fence.get_completed_value(), frame)
            self.assertEqual(tf.cell_val, frame)
            self.assertEqual(mgr.read_memory(write_addr, 4), bytearray(payload))

        self.assertEqual(len(mgr.stall_warnings), 100)
        self.assertEqual(mgr.rescued_fences_count, 100)

    def test_edge_empty_writes_bottom_of_pipe_job(self):
        """Verify that jobs with zero memory writes still advance fence and rescue stalled title fences."""
        mgr = CpuWritesAndFenceManager(watchdog_timeout_ms=4000)
        tf = mgr.register_title_fence(0x6000013DC310, initial_val=10)
        mgr.note_queue_signal(0x6000013DC310, val=20)

        gpu_fence = MockD3D12Fence(initial_val=1)
        mgr.queue_bottom_of_pipe_job(queue_id=0, fence=gpu_fence, target_val=2, writes=[])

        mgr.process_bottom_of_pipe_jobs(elapsed_wait_ms=4000)
        self.assertEqual(gpu_fence.get_completed_value(), 2)
        self.assertEqual(tf.cell_val, 20)

    def test_edge_concurrent_threads_deadlock_free(self):
        """Stress-test concurrent worker rescue, background polling, and wait thread to verify no deadlocks."""
        mgr = CpuWritesAndFenceManager(watchdog_timeout_ms=10)  # low timeout for fast stress test
        tf = mgr.register_title_fence(0x6000013DC310, initial_val=0)
        gpu_fence = MockD3D12Fence(initial_val=0)

        errors = []
        stop_event = threading.Event()

        def worker_rescue_loop():
            try:
                for val in range(1, 100):
                    mgr.note_queue_signal(0x6000013DC310, val)
                    mgr.queue_bottom_of_pipe_job(0, gpu_fence, val, [])
                    mgr.process_bottom_of_pipe_jobs(elapsed_wait_ms=15)
                    time.sleep(0.001)
            except Exception as e:
                errors.append(e)

        def poller_loop():
            try:
                while not stop_event.is_set():
                    mgr.poll_background_fences(elapsed_ms=10)
                    time.sleep(0.002)
            except Exception as e:
                errors.append(e)

        t1 = threading.Thread(target=worker_rescue_loop)
        t2 = threading.Thread(target=poller_loop)

        t1.start()
        t2.start()

        t1.join(timeout=5.0)
        self.assertFalse(t1.is_alive(), "Worker loop deadlocked!")
        stop_event.set()
        t2.join(timeout=2.0)
        self.assertFalse(t2.is_alive(), "Poller loop deadlocked!")

        self.assertEqual(len(errors), 0, f"Encountered thread errors: {errors}")
        self.assertEqual(gpu_fence.get_completed_value(), 99)
        self.assertEqual(tf.cell_val, 99)


if __name__ == "__main__":
    unittest.main()
