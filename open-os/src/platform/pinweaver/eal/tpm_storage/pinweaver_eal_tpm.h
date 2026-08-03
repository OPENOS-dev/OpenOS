
/* Copyright 2021 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __PINWEAVER_EAL_TPM_H
#define __PINWEAVER_EAL_TPM_H

#include <stdbool.h>
#include <stddef.h>

#include "pinweaver_eal_types.h"
#include "pinweaver.h"
#include "mini_trunks/tss_types.h"

#define PINWEAVER_TSS_MAX_MESSAGE_SIZE 4096

#define SALTING_KEY_HANDLE (PERSISTENT_FIRST + 4)
#define TREE_DESCRIPTOR_HANDLE (NV_INDEX_FIRST + 0x00800000 + 7)
#define LOG_ENTRY_PERSIST_HANDLE (TREE_DESCRIPTOR_HANDLE + 1)
#define LOG_ENTRY_FIRST_HANDLE (LOG_ENTRY_PERSIST_HANDLE + 1)

/*
 * Initialize TPM tunnel.
 * Returns 0 on success.
 */
int pinweaver_eal_tpm_initialize();

/*
 * Returns true if TPM tunnel client is connected.
 */
bool pinweaver_eal_tpm_connected();

/*
 * Waits until TPM tunnel client is connected.
 */
int pinweaver_eal_tpm_wait_for_connection();

/*
 * Sends a TPM command synchronously and returns the response. If a transmission
 * error occurs the response will be populated with a well-formed error
 * response.
 * Size/capacity of request is determined from the TPM command header. It must
 * not exceed PINWEAVER_TSS_MAX_MESSAGE_SIZE bytes.
 * Capacity of response must be at least PINWEAVER_TSS_MAX_MESSAGE_SIZE bytes.
 * request and response are allowed to point to the same buffer.
 */
void pinweaver_eal_send_command_and_wait(const char *request, char *response);

/*
 * Perform AES-128 CFB encrypt/decrypt.
 * Only 128 bit key size is used.
 * Returns 0 on success.
 */
#define PINWEAVER_EAL_DECRYPT 0
#define PINWEAVER_EAL_ENCRYPT 1
int pinweaver_eal_aes128_cfb(const void *key,
			     size_t key_size, /* in bytes */
			     const void *iv,
			     const void *data,
			     size_t size,
			     int op_type, /* PINWEAVER_EAL_{DE|EN}CRYPT */
			     void *res);

#define PINWEAVER_EAL_AUTH_CURRENT 0
#define PINWEAVER_EAL_AUTH_PREVIOUS 1
#define PINWEAVER_EAL_CONST 2
/*
 * Gets device key - current, previous or const.
 * Returns 0 on success.
 */
int pinweaver_eal_get_device_key(int kind,
				 void* key /* 256-bit */);

/*
 * Gets Salting key hash from storage.
 * `*committed` is set to true if it is committed to write-once memory,
 * and false if it comes from SRAM.
 * Returns 0 on success.
 */
int pinweaver_eal_get_tpm_key_hash(uint8_t tpm_key_hash[32], bool* committed);

/*
 * Sets Salting key hash in *SRAM*.
 * Returns non-zero if a hash already exists in write-once memory.
 * Otherwise overwrites the SRAM hash and returns zero.
 */
int pinweaver_eal_set_tpm_key_hash(const uint8_t tpm_key_hash[32]);

/*
 * Commits Salting key hash in SRAM to write-once storage.
 * Returns non-zero if a hash already exists in write-once memory, if
 * the hash is uninitialized (via pinweaver_eal_set_tpm_key_hash) in
 * SRAM, or in case of an error in committing the hash.
 */
int pinweaver_eal_commit_tpm_key_hash();

/*
 * Generates ECDH ephemeral point and z point based on EC public point
 * for NIST P256 curve.
 * Returns 0 on success.
 */
int pinweaver_eal_generate_ecdh_points(const TPMS_ECC_POINT* pub_key,
		TPMS_ECC_POINT* ephemeral_point,
		TPMS_ECC_POINT* z_point);

#endif  /* __PINWEAVER_EAL_H */
