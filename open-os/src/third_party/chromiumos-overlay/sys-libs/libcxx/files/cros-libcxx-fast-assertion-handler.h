// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// A low-overhead libcxx hardening assertion handler.
#ifndef SYS_LIBS_LIBCXX_FILES_CROS_LIBCXX_FAST_ASSERTION_HANDLER_H_
#define SYS_LIBS_LIBCXX_FILES_CROS_LIBCXX_FAST_ASSERTION_HANDLER_H_

#pragma GCC system_header

#ifdef __clang__
// Use of clang::nomerge is expected to have a very small binary size penalty,
// but makes postmortem debugging substantially easier (b/383757253#comment2).
#define _LIBCPP_ASSERTION_HANDLER(message)                                     \
  ((void)message, ({ [[clang::nomerge]] __builtin_trap(); }))
#else
#define _LIBCPP_ASSERTION_HANDLER(message) ((void)message, __builtin_trap())
#endif

#endif // SYS_LIBS_LIBCXX_FILES_CROS_LIBCXX_FAST_ASSERTION_HANDLER_H_
