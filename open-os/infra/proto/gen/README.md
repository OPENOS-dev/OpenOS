# Copyright 2021 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

Contains generated files that need to be version controlled:

descriptors.json - FileDescriptorSet output by buf containing last-known good
 version of proto descriptors.  Committing this lets us check for breaking
 changes in presubmits.
