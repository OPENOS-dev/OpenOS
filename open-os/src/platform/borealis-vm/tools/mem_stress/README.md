# Borealis Memory-Stress tool

This tool is designed to consume host and graphics memory at a variable rate.  The intended use is to run stress tests on host-dut and borealis VM while allocating variable amount of memory in each of the 4 regions (host GFX mem, host CPU mem, borealis GFX mem, and borealis CPU mem).

## Usage
The tool is developed in rust and can be compiled using
```shell
cargo build
```

### Additional packages
The executable can be run directly on any target machine (host dut, borealis VM, workstation) if the associated libraries and assets are present.  On a typical workstation this will require the following additional packages
```shell
apt-get install libsdl2-ttf-dev
apt-get install libsdl2-image-dev
```

### Additional assets
The asset provided in the `assets` directory also need to be available on the target device, (building and running with `cargo run` from the *project root* will take care of this on workstations).

## D-Bus APIs
Once the executable is running, it will respond to D-Bus method calls for any runtime parameter changes.  Once the program allocates the target memory, it will continue to hold that memory (and refresh the number of active memory buffers).  The following APIs are currently supported.

### SetRateMs
* Description: Sets the growth rate of memory allocations.  This parameter works in conjunction with `SetBufferSize` to control memory growth.  The mem_stress executable will request new memory of size `BufferSize` at this given internal.  The request rate will be `BufferSize`/`RateMs`.
* Parameter(s): `uint32`: rate
* Default value(s): `50` (ms)
* Usage example: sets rate to 1sec interval
```shell
dbus-send --print-reply --dest=com.borealis.mem_stress /mem_stress com.borealis.mem_stress.SetRateMs uint32:1000
```

### SetBufferSize
* Description: Sets the requested allocation size in each iteration in MB.
* Parameter(s): `uint32`: size
* Default value(s): `1024` MB
* Usage example: sets buffer size to 128 MB
```shell
dbus-send --print-reply --dest=com.borealis.mem_stress /mem_stress com.borealis.mem_stress.SetBufferSize uint32:128
```

### SetMaxBuffersNum
* Description: Total number of buffers to allocate.  The total memory allocation will be `BufferSize` * `NumBuffers`.
* Parameter(s): `uint32`: size
* Default value(s): `10`
* Usage example: sets total memory request to `BufferSize`*`100`
```shell
dbus-send --print-reply --dest=com.borealis.mem_stress /mem_stress com.borealis.mem_stress.SetMaxBuffersNum uint32:100
```

### SetActiveBuffersNum
* Description: Continuously refreshes last `n` number of buffers to keep them in memory.  This parameter effectively controls the WSS, but there is no guarantee that the buffers will not get swapped out (i.e: if the number of active buffer request is larger than what the system can handle).
* Parameter(s): `uint32`: size
* Default value(s): `20`
* Usage example: attempts to set WSS to `BufferSize`*`10`
```shell
dbus-send --print-reply --dest=com.borealis.mem_stress /mem_stress com.borealis.mem_stress.SetActiveBuffersNum uint32:10
```

### SetAllMemConfig
* Description: Updates all previously described parameters at once.
* Parameter(s):
    * `uint32`: rate_ms
    * `uint32`: size_mb
    * `uint32`: num_buffers
    * `uint32`: num_active
* Usage example:
```shell
dbus-send --print-reply --dest=com.borealis.mem_stress /mem_stress com.borealis.mem_stress.SetAllMemConfig uint32:100 uint32:1024 uint32:40 uint32:20
```

## TODO
* Implement memory deallocation
* Send d-bus signal when memory allocations complete (useful for externally controlling programs)
* Add/implement dbus control for GFX memory segments
* Integrate with mem_tracker
* Integrate with Docker to build in to borealis runtime_debug image
