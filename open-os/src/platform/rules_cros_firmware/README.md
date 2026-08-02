# `rules_cros_firmware`

This repository contains Bazel rules for building AP and EC firmware.
It's the core build logic for the Firmware SDK project.

## Getting Started

At the moment, we have a separate repo manifest in this repository
(see `default.xml`).  This *will* change in the future, and is only
for early experimentation.

To setup the checkout:

``` shellsession
$ mkdir ~/fwsdk
$ cd ~/fwsdk
$ repo init -u https://chromium.googlesource.com/chromiumos/platform/rules_cros_firmware
$ repo sync
```

## Formatting files

All files should be formatted using `cros format`.  Pre-upload checks
should validate you did this.

## Submitting changes

At the moment, we do not have CI setup in this repository, and it's
not part of the main ChromeOS build, so changes should be chumped.

At a later point we'll have some CI going.
