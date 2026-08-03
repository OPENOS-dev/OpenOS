# SiS Firmware Updater

## Description
The code was proviced by SiS and modified by Google. Modifications include:
 - Cleaning the code according to Google C++ coding style. Eliminating
compiling errors.
 - Adding a feature that the updater can take the device name from input
parameters. Blinded search is only required when input device name is invalid.
 - Adding a feature that the updater will compare FW version before actual
update. If the FW versions on device and in Chrome OS are same, the updater will
terminate.
 - Adding a input flag specifying whether the device is in recovery mode (FW
update fails last time). The flag value is determined by VID from udev rule.


## Requirements
On Linux, should run as root.

## Build
At the root of the repository: `make`.

Under Chrome OS development enviroment: `emerge-${BOARD} sis-updater`.

## How to use
Just run `./sis-updater`.
