
/* Copyright 2021 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __TPM_STORAGE_INTERNAL_H
#define __TPM_STORAGE_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mini_trunks/tss.h"
#include "pinweaver_eal.h"
#include "pinweaver_eal_tpm.h"
#include "tss_serde.h"

typedef enum {
	TPMSS_NOT_STARTED = 0,		// initial state
	TPMSS_NOT_INITIALIZED = 1,	// NV indices not created after changing the owner
	TPMSS_READY = 2,		// ready for PinWeaver commands
} tpm_storage_state_t;

typedef struct tpm_session_t {
	TPM_HANDLE handle;
	TSS_AUTHORIZATION_DELEGATE delegate;
} tpm_session_t;

typedef struct PW_PACKED tpm_storage_tree_descriptor_t {
	uint16_t version;
	uint32_t restart_count;
	struct pw_long_term_storage_t descriptor;
} tpm_storage_tree_descriptor_t;

#define DEVICE_KEY_SIZE SHA256_DIGEST_SIZE
#define DATA_KEY_SIZE SHA256_DIGEST_SIZE

typedef struct PW_PACKED log_sensitive_data_t {
	struct pw_get_log_entry_t entry;
	uint8_t hash[SHA256_DIGEST_SIZE];
} log_sensitive_data_t;

#define LOG_ENTRY_IV_SIZE SHA256_DIGEST_SIZE
typedef struct PW_PACKED tpm_storage_log_entry_t {
	uint16_t version;
	uint32_t counter;
	uint8_t iv[LOG_ENTRY_IV_SIZE];
	uint8_t encrypted_data[sizeof(log_sensitive_data_t)];
} tpm_storage_log_entry_t;

typedef struct cached_log_entry_t {
	struct pw_get_log_entry_t data;
	uint32_t counter;
} cached_log_entry_t;

#define MAX_LOG_ENTRIES 5
#define LOG_ENTRY_PERSIST MAX_LOG_ENTRIES

#define PW_OBJ_CONST_SIZE 8
#define PW_NONCE_SIZE (128/8)

#define REQUIRED_SALTING_KEY_ATTR \
	(kFixedTPM | kFixedParent | kDecrypt | kSensitiveDataOrigin)

#if defined(TEST_TPM_STORAGE)
extern tpm_storage_state_t g_storage_state;

extern tpm_session_t g_auth_session;

extern struct tpm_storage_tree_descriptor_t g_tree_descriptor;
extern bool g_tree_descriptor_filled;

extern bool g_auth_value_obtained[2];
extern TPM2B_DIGEST g_auth_value[2];

extern uint8_t g_device_key[2][DEVICE_KEY_SIZE];
extern bool g_device_key_obtained[2];

extern uint8_t g_data_key[2][DATA_KEY_SIZE];
extern bool g_data_key_obtained[2];

extern cached_log_entry_t g_cached_log_entries[MAX_LOG_ENTRIES];
extern int g_cache_order[MAX_LOG_ENTRIES];
extern uint32_t g_max_counter;

void digest_set_sha256(TPM2B_DIGEST* digest,
		const uint8_t* value /* SHA256_DIGEST_SIZE */);
void name_set_handle(TPM2B_NAME* name, TPM_HANDLE handle);
cached_log_entry_t* get_first_cached_log_entry();

int get_last_cached_log_entry_num();
int ensure_device_key(int kind);

const TPM2B_DIGEST* get_auth_value(int kind);

const uint8_t* get_data_key(int kind);
int calc_sha256(const void* data, size_t size, void* hash);
void set_nv_public_area(TPM_HANDLE handle,
		size_t data_size,
		bool set_written,
		TPMS_NV_PUBLIC* public_area);
int calc_sha256_name(const TSS_DST_DATA_BUF* data, TPM2B_NAME* name);
int calc_nv_name(const TPMS_NV_PUBLIC* public_area,
		TPM2B_NAME* name);

int calc_expected_name(TPM_HANDLE handle, size_t size,
		bool expect_written, TPM2B_NAME* name);
int check_nv_space(TPM_HANDLE handle,
		TPM2B_NAME* name,
		size_t expected_size,
		const TPMS_NV_PUBLIC* public_area);
int set_session_auth_value(tpm_session_t* session,
		const TPM2B_DIGEST* auth_value);
int set_future_session_auth_value(tpm_session_t* session,
		const TPM2B_DIGEST* auth_value);
void close_session(tpm_session_t* session);
int get_salting_key_public(TPM2B_PUBLIC* pub);
int calc_salting_key_hash(TPM2B_PUBLIC* pub,
		uint8_t* hash /* SHA256_DIGEST_SIZE */);
void dump_hash(const char* prefix, const uint8_t* hash);
int verify_salting_key_public(TPM2B_PUBLIC* pub);
int serialize_tpm_ec_point(
		const TPMS_ECC_POINT* point,
		TPM2B_ENCRYPTED_SECRET* serialized);
int generate_session_salt(TPM2B_DIGEST* salt,
		TPM2B_ENCRYPTED_SECRET* encrypted_salt);
int start_session(tpm_session_t* session,
		TPM_SE session_type,
		TPMI_DH_ENTITY bind_entity,
		const TPM2B_DIGEST* bind_authorization_value,
		bool salted,
		bool enable_encryption,
		bool force_restart);
int start_auth_session(int kind, bool force_restart);
int start_change_auth_session(
		tpm_session_t* change_auth_session);
int calc_log_sensitive_data_hash(log_sensitive_data_t* data,
		uint8_t* res /* SHA256_DIGEST_SIZE */);
int verify_log_sensitive_data(log_sensitive_data_t* data);
int decrypt_log_entry(
		const tpm_storage_log_entry_t* tpm_entry, int kind,
		cached_log_entry_t* entry);
int encrypt_log_entry(
		const struct pw_get_log_entry_t* log_entry,
		uint32_t counter, tpm_storage_log_entry_t* tpm_entry);
int cache_order_ins(int pos, int entry_num);
int cache_order_ins_entry_num(cached_log_entry_t* entry,
		int entry_num);
int cache_order_del(int pos);
int cache_order_del_entry_num(int entry_num);
int get_nv_handle_info(TPM_HANDLE handle,
		bool must_exist,
		size_t expected_size,
		TPM2B_NAME* name,
		bool* exists,
		bool* written);
int read_log_entry(TPM_HANDLE handle,
		cached_log_entry_t* entry,
		int* is_old_key);
int read_and_insert_log_entry(int entry_num, int* need_writing);
int write_log_entry_by_handle(TPM_HANDLE handle,
		tpm_storage_log_entry_t* tpm_entry);
int write_log_entry(int entry_num,
		tpm_storage_log_entry_t* tpm_entry);
int clear_persist_entry();
int write_back_log_entry(int entry_num, bool use_persist);
int read_log_entries();
void set_initial_log_entries();
void set_initial_tree_descr();
void set_initial_state();
int change_auth_handle(TPM_HANDLE handle, size_t size);
int change_auth();
int read_tree_descr();
int write_tree_descr(const tpm_storage_tree_descriptor_t *data);
int derive_pw_key(
		const uint8_t* device_key /* DEVICE_KEY_SIZE=256-bit */,
		const uint8_t* object_const /* PW_OBJ_CONST_SIZE */,
		const uint8_t* nonce /* PW_NONCE_SIZE */,
		uint8_t* result /* SHA256_DIGEST_SIZE */);
void *secure_memset(void *ptr, int value, size_t num);
void log_entry_set_default(
		struct pw_get_log_entry_t* entry);
int define_space(tpm_session_t* session,
		TPM_HANDLE handle, size_t data_size);
int create_nvmem_spaces();

#endif  // defined(TEST_TPM_STORAGE)

#ifdef __cplusplus
}
#endif

#endif  /* __TPM_STORAGE_INTERNAL_H */
