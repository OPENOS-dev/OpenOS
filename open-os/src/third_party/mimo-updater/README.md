MIMO-updater is a utility to upgrade the displaylink FW in the MIMO device.

## Requirements
The GNU C/C++ library and libusb 1.0 are required.

## Building
At the top level of the directory.
```
$ make
```
Alternatively at Chromium OS development environment,
```
$ emerge-${BOARD} mimo-updater
```

## How to use
```
$ mimo-updater $${FW file name}
```

