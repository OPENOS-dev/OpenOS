# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

load("@rules_pkg//:pkg.bzl", "pkg_deb", "pkg_tar")

pkg_tar(
    name = "systemd",
    srcs = ["cros-port-listener.service"],
    mode = "0644",
    package_dir = "/usr/lib/systemd/system",
    strip_prefix = "/cros-port-listener",
)

pkg_tar(
    name = "udev",
    srcs = ["10-cros-port-listener.rules"],
    mode = "0644",
    package_dir = "/usr/lib/udev/rules.d",
    strip_prefix = "/cros-port-listener",
)

pkg_tar(
    name = "debian-data",
    extension = "tar.gz",
    deps = [
        ":systemd",
        ":udev",
    ],
)

pkg_deb(
    name = "deb",
    architecture = "all",
    data = ":debian-data",
    depends = [
        "init-system-helpers",
    ],
    description_file = "deb-description",
    homepage = "https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/vm_tools/port_listener/",
    maintainer = "The ChromiumOS Authors <chromium-os-dev@chromium.org>",
    package = "cros-port-listener",
    postinst = "postinst",
    postrm = "postrm",
    prerm = "prerm",
    section = "misc",
    version = "0.2",
    visibility = ["//cros-debs:__pkg__"],
)
