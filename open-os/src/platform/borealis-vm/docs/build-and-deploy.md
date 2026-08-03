# Build and Deploy

(For Googlers: see also [go/borealis-care](http://go/borealis-care).)

[TOC]

## Boards which already include Borealis

Many devices include Borealis out of the box; if your board is listed in [borealis_hardware_checker.cc](https://chromium.googlesource.com/chromium/src/+/main/chrome/browser/ash/borealis/borealis_hardware_checker.cc) and meets the minimum hardware requirements, see [the public documentation](https://g.co/SteamOnChromeOS) to get started.

All other boards with `USE=borealis_host` can run Borealis by enabling the `#borealis-enable-unsupported-hardware` Chrome flag using `chrome://flags`.

## Run the VM

This can be done by running `Steam` from the Launcher, or using `vmc` via `crosh` or over SSH:

```
vmc launch borealis
```

## Building the VM Image

Ensure you have docker installed on your system.

Invoke the build commands:

```
cd <cros>/src/platform/borealis
./tools/borealis_kernel.py
./tools/build_full.py
./tools/convert_docker_image.py
```

This should create `vm_kernel` and `vm_rootfs.img` in your current directory.

## Deploying a Locally-Built Image

```
ssh <dut> -- mkdir -p /mnt/stateful_partition/img_borealis
rsync --compress --progress vm_* <dut>:/mnt/stateful_partition/img_borealis
ssh <dut> -- chmod -R a+rX /mnt/stateful_partition/img_borealis
```

Additionally, once per restart, you need to trick dlcservice into using your images by running the following on your device:

```
dlcservice_util --id=borealis-dlc --install
mount --bind /mnt/stateful_partition/img_borealis /run/imageloader/borealis-dlc/package/root/
```
