
/* Copyright 2021 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __TPM_STORAGE_H
#define __TPM_STORAGE_H

/*
 * Initialize TPM storage for the new owner.
 * Returns 0 on success.
 */
int pinweaver_eal_storage_initialize_owner();

#endif  /* __TPM_STORAGE_H */
