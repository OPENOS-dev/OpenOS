// Copyright 2022 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use dbus_ipc::process_dbus;
use gpu_mem_allocator::allocate_gpu_mem;
use log::info;
use mem_allocator::ContinuousHeapAllocator;
use simple_logger::SimpleLogger;
use std::{
    sync::{mpsc::channel, Arc, Mutex},
    thread,
};

use crate::mem_allocator::HostMemController;

mod dbus_ipc;
mod gpu_mem_allocator;
mod mem_allocator;
mod procfs_util;

fn main() {
    SimpleLogger::new().init().unwrap();

    let host_controller_base = HostMemController::new();
    let host_controller_arc = Arc::new(Mutex::new(host_controller_base));
    let hc_arc_clone = host_controller_arc.clone();

    let _ = thread::Builder::new()
        .name("DBUS_POLLING".to_owned())
        .spawn(move || {
            let _ = process_dbus(hc_arc_clone);
        });

    let (run_alloc_tx, run_alloc_rx) = channel();
    let thread_handle = ContinuousHeapAllocator::run(run_alloc_rx, host_controller_arc);

    let _ = allocate_gpu_mem(2);
    info!("GFX allocator done");
    run_alloc_tx.send(false).unwrap();
    thread_handle.join().unwrap();
    info!("Mem allocator done");
}
