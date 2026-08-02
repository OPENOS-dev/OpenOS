# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

TOOLS = [
    "ar",
    "as",
    "ld",
    "ld.bfd",
    "ld.gold",
    "nm",
    "objcopy",
    "objdump",
    "ranlib",
    "readelf",
    "strip",
]

ARCHES = [
    "aarch64-elf",
    "arm-eabi",
    "i386-elf",
    "nds32le-elf",
    "riscv64-elf",
    "x86_64-elf",
]

VERSION = "11.2.0"

# {arch}/bin/{TOOLS}
# {arch}/lib/*
# bin/{arch}-{TOOLS}
# lib/gcc/{arch}/{VERSION}/
#   include/*.h
#

def filemaps(name = ""):
    targets = []

    def _filegroup(name, srcs):
        targets.append(name)
        native.filegroup(name = name, srcs = srcs)

    for arch in ARCHES:
        _filegroup(
            "{name}{arch}_ldscripts".format(arch = arch, name = name),
            native.glob(["{arch}/lib/**/*".format(arch = arch)]),
        )

        _filegroup(
            "{name}{arch}_includes".format(arch = arch, name = name),
            native.glob(["lib/gcc/{arch}/{version}/include/*.h"
                .format(arch = arch, version = VERSION)]),
        )

        for tool in TOOLS:
            _filegroup(
                "{name}{arch}_{tool}"
                    .format(arch = arch, tool = tool, name = name),
                [
                    "bin/{arch}-{tool}".format(arch = arch, tool = tool),
                    "{arch}/bin/{tool}".format(arch = arch, tool = tool),
                ],
            )

    native.filegroup(
        name = "coreboot_sdk",
        srcs = targets,
        visibility = ["//visibility:public"],
    )
