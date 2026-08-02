# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Portage's sys-fs/f2fs-tools implicitly depends on brillo-base/libsparse
# which results in build errors if the sparse header exists.
export ac_cv_header_sparse_sparse_h=no
