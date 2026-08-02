# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

filegroup(
    name = "zmake_src",
    srcs = glob(["zephyr/zmake/**/*.py"]),
)

filegroup(
    name = "src",
    srcs = glob([
        "baseboard/**",
        "board/**",
        "builtin/**",
        "common/**",
        "driver/**",
        "include/**",
        "power/**",
        "zephyr/**",
        "fuzz/**",
        "test/**",
        "third_party/**",
        "util/check_zephyr_end_of_ram.py",
        "*",
    ], exclude = ["**/twister-out/**"]),
    visibility = ["//visibility:public"],
)

py_binary(
    name = "zmake",
    srcs = [":zmake_src"],
    data = [
        "@ec//:src",
    ],
    main = "//:zephyr/zmake/bazel_main.py",
    visibility = ["//visibility:public"],
    deps = ["@zephyr//:py_deps", "@u_boot//:binman"],
)

filegroup(
    name = "twister_src",
    srcs = glob(["util/twister_launcher.py"])
)

py_binary(
    name = "twister_binary",
    srcs = ["@ec//:twister_src"],
    data = [
        "@ec//:src",
    ],
    main = "//:util/twister_launcher.py",
    visibility = ["//visibility:public"],
    deps = ["@zephyr//:py_deps"],
)

exports_files(["zephyr"])
