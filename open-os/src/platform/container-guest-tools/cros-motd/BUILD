# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

load("@rules_pkg//:pkg.bzl", "pkg_deb", "pkg_tar")

pkg_tar(
    name = "motd-profiled",
    srcs = ["cros-motd.sh"],
    mode = "0644",
    package_dir = "/etc/profile.d",
    strip_prefix = "/cros-motd",
)

pkg_tar(
    name = "debian-data",
    extension = "tar.gz",
    deps = [
        ":motd-profiled",
    ],
)

pkg_deb(
    name = "deb",
    architecture = "all",
    data = ":debian-data",
    depends = [
        "bash",
    ],
    description_file = "deb-description",
    homepage = "https://chromium.googlesource.com/chromiumos/platform2/+/HEAD/vm_tools/",
    maintainer = "The ChromiumOS Authors <chromium-os-dev@chromium.org>",
    package = "cros-motd",
    recommends = [],
    section = "misc",
    version = "1.1",
    visibility = ["//cros-debs:__pkg__"],
)
