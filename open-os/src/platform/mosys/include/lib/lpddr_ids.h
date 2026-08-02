/* Copyright 2022 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef LPDDR_IDS_H
#define LPDDR_IDS_H

#include "lib/nonspd.h"

extern const struct nonspd_mem_info *lpddr_info_from_fdt_ids(void);

#endif /* LPDDR_IDS_H */
