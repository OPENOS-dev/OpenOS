// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use anyhow::Result;
use closure::closure;
use dbus::blocking::Connection;
use dbus_crossroads::Crossroads;
use log::info;
use std::ops::DerefMut;
use std::sync::Arc;
use std::sync::Mutex;

use crate::mem_allocator::HostMemController;

struct DbusStat {
    // Placeholder for any future context data
    _called_count: u32,
}

pub(crate) fn process_dbus(host_controller: Arc<Mutex<HostMemController>>) -> Result<()> {
    let c = Connection::new_session()?;
    c.request_name("com.borealis.mem_stress", false, true, false)?;
    let mut cr = Crossroads::new();

    let iface_token = cr.register("com.borealis.mem_stress", |b| {
        b.method(
            "SetRateMs",
            ("rate",),
            ("reply",),
            closure!(clone host_controller, |_, _, (rate_ms,): (u32,)| {
                info!("New rate from dbus {rate_ms}");

                let mut binding = host_controller.lock().unwrap();
                binding.deref_mut().set_alloc_rate(rate_ms);

                let reply = format!("New rate set to {rate_ms}");
                Ok((reply,))
            }),
        );

        b.method(
            "SetBufferSize",
            ("size",),
            ("reply",),
            closure!(clone host_controller, |_, _, (size_mb,): (u32,)| {
                info!("New size from dbus {size_mb}");

                let mut binding = host_controller.lock().unwrap();
                binding.deref_mut().set_alloc_size_mb(size_mb);

                let reply = format!("New size set to {size_mb}");
                Ok((reply,))
            }),
        );

        b.method(
            "SetMaxBuffersNum",
            ("size",),
            ("reply",),
            closure!(clone host_controller, |_, _, (n_buffers,): (u32,)| {
                info!("New max buffer num from dbus {n_buffers}");

                let mut binding = host_controller.lock().unwrap();
                binding.deref_mut().set_num_total_buffers(n_buffers);

                let reply = format!("New max buffers set to {n_buffers}");
                Ok((reply,))
            }),
        );

        b.method(
            "SetActiveBuffersNum",
            ("size",),
            ("reply",),
            closure!(clone host_controller, |_, _, (n_buffers,): (u32,)| {
                info!("New active buffer num from dbus {n_buffers}");

                let mut binding = host_controller.lock().unwrap();
                binding.deref_mut().set_num_active_buffers(n_buffers);

                let reply = format!("New active buffers set to {n_buffers}");
                Ok((reply,))
            }),
        );

        b.method(
            "SetAllMemConfig",
            ("rate_ms", "size_mb", "num_buffers", "num_active"),
            ("reply",),
            closure!(clone host_controller,
                |_, _, (rate, size, n_buffers, n_active): (u32, u32, u32, u32)| {

                let mut binding = host_controller.lock().unwrap();
                let config : &mut HostMemController= binding.deref_mut();
                config.set_alloc_rate(rate);
                config.set_alloc_size_mb(size);
                config.set_num_total_buffers(n_buffers);
                config.set_num_active_buffers(n_active);

                info!("All config update to: {:?}", config);
                let reply = format!("Memory request config updated to: {:?}", config);
                Ok((reply,))
            }),
        );
    });

    // Add the "/mem_stress" path, which implements the com.borealis.mem_stress interface,
    // to the crossroads instance.
    cr.insert("/mem_stress", &[iface_token], DbusStat { _called_count: 0 });

    // Serve clients forever.
    cr.serve(&c)?;
    unreachable!()
}
