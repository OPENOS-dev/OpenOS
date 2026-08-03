// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use log::{info, warn};
use rand::Rng;
use std::alloc::{alloc, Layout};
use std::ops::Deref;
use std::sync::mpsc::Receiver;
use std::sync::{Arc, Mutex};
use std::thread;
use std::thread::JoinHandle;
use std::time::Duration;

use crate::procfs_util::{get_memory_stat, get_self_pid, MemPageData};

const THREAD_NAME: &str = "heap_allocator";
const IDLE_SLEEP_TIME: u64 = 1000;

#[derive(Default, Debug)]
pub struct HostMemController {
    alloc_rate_ms: u32,
    mem_alloc_size_mb: u32,
    num_total_buffers: u32,
    num_active_buffers: u32,
    _state: ControllerState,
}

#[derive(Default, Debug, PartialEq)]
pub enum ControllerState {
    #[default]
    TargetReached,
    Updating,
    _Paused, //TODO: implement freeze/unfreeze from dbus message.
}

// TODO: implement memory deallocation when requested totalbuffers < current.
// TODO: use getset crate instead of typing out. Add a builder?
impl HostMemController {
    pub fn new() -> HostMemController {
        HostMemController {
            // Defaults
            alloc_rate_ms: 50,
            mem_alloc_size_mb: 512 * 2,
            num_total_buffers: 10,
            num_active_buffers: 20,
            _state: ControllerState::TargetReached,
        }
    }

    pub fn get_alloc_rate(&self) -> u32 {
        self.alloc_rate_ms
    }

    pub fn set_alloc_rate(&mut self, rate: u32) {
        self.alloc_rate_ms = rate;
    }

    pub fn get_alloc_size_mb(&self) -> u32 {
        self.mem_alloc_size_mb
    }

    pub fn set_alloc_size_mb(&mut self, size: u32) {
        self.mem_alloc_size_mb = size;
    }

    pub fn get_num_total_buffers(&self) -> u32 {
        self.num_total_buffers
    }

    pub fn set_num_total_buffers(&mut self, num: u32) {
        if num < self.num_total_buffers {
            warn!("Memory deallocation not implemented yet :(");
        } else {
            self.num_total_buffers = num;
        }
    }

    pub fn get_num_active_buffers(&self) -> u32 {
        self.num_active_buffers
    }

    pub fn set_num_active_buffers(&mut self, num: u32) {
        self.num_active_buffers = num;
    }
}

#[derive(Default)]
pub struct ContinuousHeapAllocator {
    thread_pid: i32,
    buffer: Vec<Vec<u8>>,
    host_controller: Arc<Mutex<HostMemController>>,
}

// TODO: occasional 'clear_ref' to dump cache and rebuild RSS?
impl ContinuousHeapAllocator {
    fn new(
        thread_pid: i32,
        host_controller: Arc<Mutex<HostMemController>>,
    ) -> ContinuousHeapAllocator {
        ContinuousHeapAllocator {
            thread_pid,
            host_controller,
            ..Default::default()
        }
    }

    // Helper functions to deref the arc-mutex.  TODO: Turn them into a macro.
    fn get_rate_ms(&self) -> u64 {
        self.host_controller
            .lock()
            .unwrap()
            .deref()
            .get_alloc_rate() as u64
    }

    fn get_alloc_size_mb(&self) -> usize {
        self.host_controller
            .lock()
            .unwrap()
            .deref()
            .get_alloc_size_mb() as usize
    }

    fn get_num_total_buffers(&self) -> usize {
        self.host_controller
            .lock()
            .unwrap()
            .deref()
            .get_num_total_buffers() as usize
    }

    fn get_num_active_buffers(&self) -> usize {
        self.host_controller
            .lock()
            .unwrap()
            .deref()
            .get_num_active_buffers() as usize
    }

    pub fn run(
        cancel_alloc: Receiver<bool>,
        host_controller: Arc<Mutex<HostMemController>>,
    ) -> JoinHandle<()> {
        thread::Builder::new()
            .name(THREAD_NAME.to_owned())
            .spawn(move || {
                info!("Creating memory allocation background thread");

                let thread_pid = get_self_pid().unwrap();
                let mut allocator = ContinuousHeapAllocator::new(thread_pid, host_controller);

                allocator.loop_allocation(cancel_alloc);
            })
            .unwrap()
    }

    fn loop_allocation(&mut self, cancel_alloc: Receiver<bool>) {
        let start = self.get_curr_stat();
        info!("Mem stat:*{:?}", start);

        let mut polling_interval = IDLE_SLEEP_TIME;
        let mut state = ControllerState::TargetReached;

        loop {
            if self.buffer.len() < self.get_num_total_buffers() {
                // need to create more buffers at the given rate
                state = ControllerState::Updating;
                polling_interval = self.get_rate_ms();

                let new_alloc = self.allocate_heap();
                self.buffer.push(new_alloc);
            } else if state != ControllerState::TargetReached {
                // One shot when buffers fully allocated
                state = ControllerState::TargetReached;
                polling_interval = IDLE_SLEEP_TIME;
                info!("mem allocation target reached.  No new allocations");
                // TODO: send dbus signal
            }

            self.update_buffers();

            if state == ControllerState::Updating {
                // Print out the alloc size while updating
                // (might be too spammy at higher allocation rates)
                let stat = get_memory_stat(self.thread_pid).unwrap();
                info!("Mem stat: {:?}", stat);
            }

            // does an implicit thread sleep.
            if cancel_alloc
                .recv_timeout(Duration::from_millis(polling_interval))
                .is_ok()
            {
                // Don't need to check the value of the bool; any message on the channel cancels this.
                break;
            }
        }
    }

    pub fn get_curr_stat(&self) -> MemPageData {
        get_memory_stat(self.thread_pid).unwrap()
    }

    fn allocate_heap(&self) -> Vec<u8> {
        // Only for cargo debug builds, print on update
        #[cfg(debug_assertions)]
        {
            static mut LAST_SZ: usize = 0;

            if self.get_alloc_size_mb() != unsafe { LAST_SZ } {
                info!(
                    "---> Buffer size changed: {} -> {}",
                    unsafe { LAST_SZ },
                    self.get_alloc_size_mb()
                );
                unsafe { LAST_SZ = self.get_alloc_size_mb() };
            }
        }

        // TODO: mutex unlocking might be expensive,
        // only check for dbus updates periodically
        vec![5u8; 1024 * 1024 * self.get_alloc_size_mb()]
    }

    fn update_buffers(&mut self) {
        // Only touch the last n windows (helps to force page out of earlier memory)
        // This can effectively control the WSS.
        let latest_idx = self.buffer.len();
        let num_active_buffers = self.get_num_active_buffers();

        let start_idx = if latest_idx > num_active_buffers {
            latest_idx - num_active_buffers
        } else {
            0
        };

        for row in start_idx..latest_idx {
            let mut rng = rand::thread_rng();
            let idx = rng.gen_range(0..self.buffer[row].len());

            self.buffer[row][idx] = rng.gen()
        }
    }

    fn _allocate_manual(num_mbytes: usize) -> *mut u8 {
        let size = std::mem::size_of::<u32>() * 1000 * 1000 * num_mbytes;
        let align = std::mem::align_of::<u32>();
        let layout = Layout::from_size_align(size, align).unwrap();

        let ptr = unsafe { alloc(layout) };

        if !ptr.is_null() {
            unsafe {
                // Use the allocated memory
                *(ptr as *mut u32) = 42;
                info!("Allocated value: {}", *ptr);

                // Deallocate the memory
                // info!("Deallocating");
                // dealloc(ptr, layout);
            }
        }
        ptr
    }

    fn _touch_mem(&self, mem: &mut Vec<u8>) {
        let mut rng = rand::thread_rng();
        let idx = rng.gen_range(0..mem.len());

        mem[idx] = rng.gen();
    }
}
