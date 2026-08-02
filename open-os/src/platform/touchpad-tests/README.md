# Touch tests

[TOC]

## Introduction

This repository contains automated tests for [Chromium OS's Gestures library][gestures-lib].
Each test has a log of evdev events which are replayed, a properties file
containing [gesture properties] to set while the Gestures library runs, and a
Python function which verifies the output and returns a test score.

[gestures-lib]: https://chromium.googlesource.com/chromiumos/platform/gestures/+/HEAD/
[gesture properties]: https://chromium.googlesource.com/chromiumos/platform/gestures/+/HEAD/docs/gesture_properties.md

## Setting up

Assuming that you've followed the [developer guide], simply enter the SDK
chroot using the `cros_sdk` command, then run the following inside:

```sh
(inside)
$ cd /mnt/host/source/src/platform/touchpad-tests
$ sudo make setup-in-place
```

[developer guide]: https://www.chromium.org/chromium-os/developer-library/guides/development/developer-guide/

## Running tests

To run all tests, simply run `touchtests`. To run one or more specific tests,
you can pass a test name or a glob:

```sh
(inside)
$ touchtests atlas-1.0/fat-thumb-fail
$ touchtests atlas-1.0/palm-while-typing*
```

Each test will return a status, with the following meanings:

* **success**: the test succeeded, with the given score out of 1.
* **failure**: the test failed.
* **error**: an error occurred while running the test, so the behavior of the
  gestures library couldn't be evaluated.
* **incomplete**: (baseline tests only) the evdev log for this platform hasn't
  been created.

### Checking for regressions

The `--out` (or `-o`) switch creates a report file that future runs can be
compared against with the `--ref` (or `-r`) switch:

```sh
(inside)
$ touchtests --out baseline.json
# (cause some regressions)
$ touchtests --ref baseline.json
```

The output table will contain a delta column that indicates any regressions or
improvements, and an error message will be shown if regressions exist.
