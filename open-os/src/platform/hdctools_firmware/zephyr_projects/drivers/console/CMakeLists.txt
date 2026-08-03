# Copyright 2023 The ChromiumOS Authors.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

add_subdirectory_ifdef(CONFIG_ARG_PARSER arg_parser)
add_subdirectory_ifdef(CONFIG_REBOOT_CMD reboot_cmd)
