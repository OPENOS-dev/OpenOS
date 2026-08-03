// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// A high(er)-overhead libcxx hardening assertion handler. Useful for
// e.g., dev/sanitizer builds, when binary size & performance matter less than
// getting a high-quality error message from a crash.
#ifndef SYS_LIBS_LIBCXX_FILES_CROS_LIBCXX_DEBUG_ASSERTION_HANDLER_H_
#define SYS_LIBS_LIBCXX_FILES_CROS_LIBCXX_DEBUG_ASSERTION_HANDLER_H_

#include <__verbose_abort>

#pragma GCC system_header

// Simply use _LIBCPP_VERBOSE_ABORT, since that provides solid detail.
#define _LIBCPP_ASSERTION_HANDLER(message) _LIBCPP_VERBOSE_ABORT("%s", message)

#endif // SYS_LIBS_LIBCXX_FILES_CROS_LIBCXX_DEBUG_ASSERTION_HANDLER_H_
