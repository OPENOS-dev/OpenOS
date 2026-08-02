# Copyright 2023 The ChromiumOS Authors.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set(APPLICATION_DIR_INCLUDE_GENERATED ${APPLICATION_BINARY_DIR}/include/generated)
file(MAKE_DIRECTORY ${APPLICATION_DIR_INCLUDE_GENERATED})
zephyr_include_directories(${APPLICATION_DIR_INCLUDE_GENERATED})
set(APPLICATION_DIR_INCLUDE_GENERATED_PATH ${APPLICATION_DIR_INCLUDE_GENERATED}/ver_hdc.h)

add_custom_target(
    build_info_header
    VERBATIM COMMAND
    ${PYTHON_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/generate_build_info.py"
    "${APPLICATION_DIR_INCLUDE_GENERATED_PATH}"
)
add_dependencies(app build_info_header)
