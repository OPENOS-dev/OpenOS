/* Copyright 2021 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#if defined(__STDC_LIB_EXT1__)
#define __STDC_WANT_LIB_EXT1__ 1
#define HAS_MEMCPY_S
#endif

#include <stdbool.h>
#include <string.h>

#include "tpm_storage_internal.h"

#if defined(TEST_TPM_STORAGE)
#define TESTABLE_STATIC
#else
#define TESTABLE_STATIC static
#endif

#if defined(HAS_MEMCPY_S)
int pinweaver_eal_memcpy_s(
    void * dest,
    size_t destsz,
    const void * src,
    size_t count
) {
	return memcpy_s(dest, destsz, src, count);
}
#else
#include <errno.h>

int pinweaver_eal_memcpy_s(
    void * dest,
    size_t destsz,
    const void * src,
    size_t count
)
{
	if (count == 0)
		return 0;

	if (dest == NULL)
		return EINVAL;

	if (src == NULL) {
		memset(dest, 0, destsz);
		return EINVAL;
	}

	if (destsz < count) {
		memset(dest, 0, destsz);
		return ERANGE;
	}

    memcpy(dest, src, count);
    return 0;
}
#endif

/* Make sure the largest possible message would fit in 2K
 */
BUILD_ASSERT(PW_MAX_MESSAGE_SIZE + 10 <= PINWEAVER_TSS_MAX_MESSAGE_SIZE);

TESTABLE_STATIC tpm_storage_state_t g_storage_state = TPMSS_NOT_STARTED;

TESTABLE_STATIC tpm_session_t g_auth_session = { 0, };

TESTABLE_STATIC struct tpm_storage_tree_descriptor_t g_tree_descriptor;
TESTABLE_STATIC bool g_tree_descriptor_filled = false;

TESTABLE_STATIC bool g_auth_value_obtained[2] = {false, false};
TESTABLE_STATIC TPM2B_DIGEST g_auth_value[2];

TESTABLE_STATIC uint8_t g_device_key[2][DEVICE_KEY_SIZE];
TESTABLE_STATIC bool g_device_key_obtained[2] = {false, false};

TESTABLE_STATIC uint8_t g_data_key[2][DATA_KEY_SIZE];
TESTABLE_STATIC bool g_data_key_obtained[2] = {false, false};

TESTABLE_STATIC cached_log_entry_t g_cached_log_entries[MAX_LOG_ENTRIES];
TESTABLE_STATIC int g_cache_order[MAX_LOG_ENTRIES];
TESTABLE_STATIC uint32_t g_max_counter = 0;

TESTABLE_STATIC void digest_set_sha256(TPM2B_DIGEST* digest,
		const uint8_t* value /* SHA256_DIGEST_SIZE */)
{
	pinweaver_eal_memcpy_s(digest->buffer, sizeof(digest->buffer),
			value, SHA256_DIGEST_SIZE);
	digest->size = SHA256_DIGEST_SIZE;
}

TESTABLE_STATIC void name_set_handle(TPM2B_NAME* name, TPM_HANDLE handle)
{
	uint32_t serialized = htobe32(handle);
	pinweaver_eal_memcpy_s(name->name, sizeof(name->name),
			&serialized, sizeof(serialized));
	name->size = sizeof(serialized);
}

TESTABLE_STATIC cached_log_entry_t* get_first_cached_log_entry()
{
	int num = g_cache_order[0];

	if (num < 0 || num >= MAX_LOG_ENTRIES)
		return NULL;
	return &g_cached_log_entries[num];
}

TESTABLE_STATIC int get_last_cached_log_entry_num()
{
	int num = g_cache_order[MAX_LOG_ENTRIES - 1];

	if (num < 0 || num >= MAX_LOG_ENTRIES)
		return -1;
	return num;
}

TESTABLE_STATIC int ensure_device_key(int kind) {
	kind = kind ? 1 : 0;
	if (g_device_key_obtained[kind])
		return 0;
	int res = pinweaver_eal_get_device_key(kind, g_device_key[kind]);
	if (res) {
		PINWEAVER_EAL_INFO("pinweaver_eal_get_device_key %d failed: %d", kind, res);
		return res;
	}
	g_device_key_obtained[kind] = true;
	return 0;
}

TESTABLE_STATIC const TPM2B_DIGEST* get_auth_value(int kind) {
	const size_t kAuthValueLabelSize = 6;
	const char* kAuthValueLabel = "PW_ATH";

	kind = kind ? 1 : 0;
	if (g_auth_value_obtained[kind])
		return &g_auth_value[kind];
	if (ensure_device_key(kind))
		return NULL;

	pinweaver_eal_hmac_sha256_ctx_t hash;
	if (pinweaver_eal_hmac_sha256_init(&hash, g_device_key[kind], DEVICE_KEY_SIZE))
		return NULL;
	if (pinweaver_eal_hmac_sha256_update(&hash, kAuthValueLabel, kAuthValueLabelSize)) {
		pinweaver_eal_hmac_sha256_final(&hash, g_auth_value[kind].buffer);
		return NULL;
	}
	if (pinweaver_eal_hmac_sha256_final(&hash, g_auth_value[kind].buffer))
		return NULL;
	g_auth_value[kind].size = SHA256_DIGEST_SIZE;

	g_auth_value_obtained[kind] = true;
	return &g_auth_value[kind];
}

TESTABLE_STATIC const uint8_t* get_data_key(int kind) {
	kind = kind ? 1 : 0;
	if (g_data_key_obtained[kind])
		return g_data_key[kind];
	if (!g_tree_descriptor_filled)
		return NULL;
	if (ensure_device_key(kind))
		return NULL;

	pinweaver_eal_hmac_sha256_ctx_t hash;
	if (pinweaver_eal_hmac_sha256_init(&hash, g_device_key[kind], DEVICE_KEY_SIZE))
		return NULL;
	if (pinweaver_eal_hmac_sha256_update(&hash,
			g_tree_descriptor.descriptor.key_derivation_nonce,
			sizeof(g_tree_descriptor.descriptor.key_derivation_nonce))) {
		pinweaver_eal_hmac_sha256_final(&hash, g_data_key[kind]);
		return NULL;
	}
	if (pinweaver_eal_hmac_sha256_final(&hash, g_data_key[kind]))
		return NULL;

	g_data_key_obtained[kind] = true;
	return g_data_key[kind];
}

TESTABLE_STATIC int calc_sha256(const void* data, size_t size, void* hash)
{
	pinweaver_eal_sha256_ctx_t ctx;
	if (pinweaver_eal_sha256_init(&ctx))
		return -1;
	if (pinweaver_eal_sha256_update(&ctx, data, size)) {
		pinweaver_eal_sha256_final(&ctx, hash);
		return -1;
	}
	return pinweaver_eal_sha256_final(&ctx, hash);
}

TESTABLE_STATIC void set_nv_public_area(TPM_HANDLE handle,
		size_t data_size,
		bool set_written,
		TPMS_NV_PUBLIC* public_area)
{
	// Policy digest for nvmem spaces:
	// - Initial state
	//   0000000000000000000000000000000000000000000000000000000000000000
	// - PolicyAuthValue:
	//     <new> = sha256(<previous> || 0000016B)
	//   8fcd2169ab92694e0c633f1ab772842b8241bbc20288981fc7ac1eddc1fddb0e
	// - PolicyCommandCode(NV_ChangeAuth):
	//     <new> = sha256(<previous> || 0000016C || 0000013B)
	//   363ac945b6457c47c31f3355dba0db27de8db213d6250c6bf79685003f9fe7ab
	const uint8_t kNvPolicy[SHA256_DIGEST_SIZE] =
	{
		0x36, 0x3a, 0xc9, 0x45, 0xb6, 0x45, 0x7c, 0x47,
		0xc3, 0x1f, 0x33, 0x55, 0xdb, 0xa0, 0xdb, 0x27,
		0xde, 0x8d, 0xb2, 0x13, 0xd6, 0x25, 0x0c, 0x6b,
		0xf7, 0x96, 0x85, 0x00, 0x3f, 0x9f, 0xe7, 0xab,
	};
	memset(public_area, 0, sizeof(TPMS_NV_PUBLIC));
	public_area->nv_index = handle;
	public_area->name_alg = TPM_ALG_SHA256;
	public_area->attributes =
			TPMA_NV_AUTHREAD | TPMA_NV_AUTHWRITE | TPMA_NV_NO_DA;
	if (set_written)
		public_area->attributes |= TPMA_NV_WRITTEN;
	digest_set_sha256(&public_area->auth_policy, kNvPolicy);
	public_area->data_size = data_size;
}

TESTABLE_STATIC int calc_sha256_name(const TSS_DST_DATA_BUF* data,
		TPM2B_NAME* name)
{
	/* Serialized TPM_ALG_SHA256 */
	const uint8_t kSerializedAlgSha256[2] = {0x00, 0x0B};

	if (pinweaver_eal_memcpy_s(name->name, sizeof(name->name),
			kSerializedAlgSha256, sizeof(kSerializedAlgSha256)))
		return -1;
	name->size = sizeof(kSerializedAlgSha256) + SHA256_DIGEST_SIZE;
	uint8_t* hash = name->name + sizeof(kSerializedAlgSha256);
	return calc_sha256(data->buffer, data->size, hash);
}

TESTABLE_STATIC int calc_nv_name(const TPMS_NV_PUBLIC* public_area,
		TPM2B_NAME* name)
{
	TPMS_NV_PUBLIC serialized;
	TSS_DST_DATA_BUF buffer = {sizeof(serialized), 0,
			(uint8_t *)&serialized};
	if (tss_Serialize_TPMS_NV_PUBLIC(public_area, &buffer))
		return -1;
	return calc_sha256_name(&buffer, name);
}

TESTABLE_STATIC int calc_expected_name(TPM_HANDLE handle, size_t size,
		bool expect_written, TPM2B_NAME* name)
{
	TPMS_NV_PUBLIC public_area;
	set_nv_public_area(handle,
		size, expect_written, &public_area);
	return calc_nv_name(&public_area, name);
}

TESTABLE_STATIC int check_nv_space(TPM_HANDLE handle,
		TPM2B_NAME* name,
		size_t expected_size,
		const TPMS_NV_PUBLIC* public_area)
{
	TPM2B_NAME expected_name;

	const bool written = (public_area->attributes & TPMA_NV_WRITTEN) != 0;
	if (calc_expected_name(handle, expected_size, written, &expected_name))
		return -1;
	if (name->size != expected_name.size)
		return -1;
	return memcmp(name->name, expected_name.name, name->size);
}

TESTABLE_STATIC int set_session_auth_value(tpm_session_t* session,
		const TPM2B_DIGEST* auth_value)
{
	if (auth_value->size >
			sizeof(session->delegate.hmac.entity_authorization_value_.buffer)) {
		return -1;
	}
	session->delegate.hmac.entity_authorization_value_.size = auth_value->size;
	pinweaver_eal_memcpy_s(
			session->delegate.hmac.entity_authorization_value_.buffer,
			sizeof(session->delegate.hmac.entity_authorization_value_.buffer),
			auth_value->buffer, auth_value->size);
	return 0;
}

TESTABLE_STATIC int set_future_session_auth_value(tpm_session_t* session,
		const TPM2B_DIGEST* auth_value)
{
	if (auth_value->size >
			sizeof(session->delegate.hmac.future_authorization_value_.buffer)) {
		return -1;
	}
	session->delegate.hmac.future_authorization_value_.size = auth_value->size;
	pinweaver_eal_memcpy_s(
			session->delegate.hmac.future_authorization_value_.buffer,
			sizeof(session->delegate.hmac.future_authorization_value_.buffer),
			auth_value->buffer, auth_value->size);
	session->delegate.hmac.future_authorization_value_set_ = true;
	return 0;
}

TESTABLE_STATIC void close_session(tpm_session_t* session)
{
	if (session->handle) {
		tss_FlushContext(session->handle, NULL);
		session->handle = 0;
	}
}

TESTABLE_STATIC int start_session(tpm_session_t* session,
		TPM_SE session_type,
		TPMI_DH_ENTITY bind_entity,
		const TPM2B_DIGEST* bind_authorization_value,
		bool salted,
		bool enable_encryption,
		bool force_restart);

TESTABLE_STATIC int get_salting_key_public(TPM2B_PUBLIC* public)
{
	TPM2B_NAME name;
	TPM2B_NAME qualified_name;
	TPM_RC rc = tss_ReadPublic(SALTING_KEY_HANDLE,
			NULL /* doesn't matter w/o auth */,
			public,
			&name,
			&qualified_name,
			/* delegate = */ NULL);
	return rc == TPM_RC_SUCCESS ? 0 : -1;
}

TESTABLE_STATIC int calc_salting_key_hash(TPM2B_PUBLIC* public,
		uint8_t* hash /* SHA256_DIGEST_SIZE */)
{
	TPMS_ECC_POINT* pub_key = &public->public_area.unique.ecc;
	pinweaver_eal_sha256_ctx_t ctx;

	if (pub_key->x.size > sizeof(pub_key->x.buffer) ||
			pub_key->y.size > sizeof(pub_key->y.buffer)) {
		return -1;
	}
	if (pinweaver_eal_sha256_init(&ctx))
		return -1;
	if (pinweaver_eal_sha256_update(&ctx, pub_key->x.buffer, pub_key->x.size)) {
		pinweaver_eal_sha256_final(&ctx, hash);
		return -1;
	}
	if (pinweaver_eal_sha256_update(&ctx, pub_key->y.buffer, pub_key->y.size)) {
		pinweaver_eal_sha256_final(&ctx, hash);
		return -1;
	}
	return pinweaver_eal_sha256_final(&ctx, hash);
}

TESTABLE_STATIC void dump_hash(const char* prefix, const uint8_t* hash)
{
	char buffer[SHA256_DIGEST_SIZE*3 + 1];
	const size_t size = sizeof(buffer);
	char* ptr = buffer;
	int n;

	for(n = 0; n < SHA256_DIGEST_SIZE; ++n) {
		snprintf(ptr, size - 3*n, "%02x ", hash[n]);
		ptr += strlen(ptr);
	}
	*ptr = 0;
	PINWEAVER_EAL_INFO("%s: %s", prefix, buffer);
}

TESTABLE_STATIC int verify_salting_key_public(TPM2B_PUBLIC* public)
{
	uint8_t key_hash[SHA256_DIGEST_SIZE];
	uint8_t provisioned_hash[SHA256_DIGEST_SIZE];
	bool committed;

	if (public->public_area.type != TPM_ALG_ECC) {
		PINWEAVER_EAL_INFO("verify_salting_key_public: non-ECC key");
		return -1;
	}
	if (public->public_area.parameters.ecc_detail.curve_id !=
			TPM_ECC_NIST_P256) {
		PINWEAVER_EAL_INFO("verify_salting_key_public: wrong key curve %#x",
				public->public_area.parameters.ecc_detail.curve_id);
		return -1;
	}
	if ((public->public_area.object_attributes & REQUIRED_SALTING_KEY_ATTR) !=
			REQUIRED_SALTING_KEY_ATTR) {
		PINWEAVER_EAL_INFO("verify_salting_key_public: bad key attr %#x",
				public->public_area.object_attributes);
		return -1;
	}

	if (pinweaver_eal_get_tpm_key_hash(provisioned_hash, &committed)) {
		PINWEAVER_EAL_INFO("verify_salting_key_public: failed to get provisioned hash");
		return -1;
	}
	dump_hash("Provisioned hash", provisioned_hash);
	if (calc_salting_key_hash(public, key_hash)) {
		PINWEAVER_EAL_INFO("verify_salting_key_public: failed to calculate key hash");
		return -1;
	}
	dump_hash("Salting key hash", key_hash);
	return pinweaver_eal_safe_memcmp(key_hash, provisioned_hash,
			SHA256_DIGEST_SIZE);
}

TESTABLE_STATIC int serialize_tpm_ec_point(
		const TPMS_ECC_POINT* point,
		TPM2B_ENCRYPTED_SECRET* serialized)
{
	TSS_DST_DATA_BUF buffer =
			{sizeof(serialized->secret), 0, serialized->secret};
			
	TPM_RC rc = tss_Serialize_TPMS_ECC_POINT(point, &buffer);
	if (rc != TPM_RC_SUCCESS)
		return -1;
	serialized->size = buffer.size;
	return 0;
}

TESTABLE_STATIC int generate_session_salt(TPM2B_DIGEST* salt,
		TPM2B_ENCRYPTED_SECRET* encrypted_salt)
{
	TPM2B_PUBLIC salting_key_public;
	if (get_salting_key_public(&salting_key_public)) {
		PINWEAVER_EAL_INFO("generate_session_salt: failed to get salting key public");
		return -1;
	}
	if (verify_salting_key_public(&salting_key_public)) {
		PINWEAVER_EAL_INFO("generate_session_salt: bad salting key public");
		return -1;
	}

	TPMS_ECC_POINT* pub_key = &salting_key_public.public_area.unique.ecc;
	TPMS_ECC_POINT ephemeral_point;
	TPMS_ECC_POINT z_point;

	if (pinweaver_eal_generate_ecdh_points(pub_key,
			&ephemeral_point, &z_point)) {
		PINWEAVER_EAL_INFO("generate_session_salt: failed to generate ECDH points");
		return -1;
	}
	if (ephemeral_point.x.size > sizeof(ephemeral_point.x.buffer) ||
			ephemeral_point.y.size > sizeof(ephemeral_point.y.buffer) ||
			z_point.x.size > sizeof(z_point.x.buffer) ||
			z_point.y.size > sizeof(z_point.y.buffer)
			) {
		PINWEAVER_EAL_INFO("generate_session_salt: bad ECDH coord size(s)");
		return -1;
	}

	if (serialize_tpm_ec_point(&ephemeral_point, encrypted_salt)) {
		PINWEAVER_EAL_INFO("generate_session_salt: failed to serialize ECDH points");
		return -1;
	}

	const uint8_t kMarshaledCounter[4] = {0, 0, 0, 1}; /* big-endian 1 */
	const uint8_t kSessionKeyLabel[7] = {'S', 'E', 'C', 'R', 'E', 'T', 0};

	const TPM2B_ECC_PARAMETER* z_value = &z_point.x;
  	const TPM2B_ECC_PARAMETER* party_u_info = &ephemeral_point.x;
  	const TPM2B_ECC_PARAMETER* party_v_info = &pub_key->x;

	pinweaver_eal_sha256_ctx_t ctx;
	if (pinweaver_eal_sha256_init(&ctx))
		return -1;
	if (pinweaver_eal_sha256_update(&ctx, kMarshaledCounter, sizeof(kMarshaledCounter))) {
		pinweaver_eal_sha256_final(&ctx, salt->buffer);
		return -1;
	}
	if (pinweaver_eal_sha256_update(&ctx, z_value->buffer, z_value->size)) {
		pinweaver_eal_sha256_final(&ctx, salt->buffer);
		return -1;
	}
	if (pinweaver_eal_sha256_update(&ctx, kSessionKeyLabel, sizeof(kSessionKeyLabel))) {
		pinweaver_eal_sha256_final(&ctx, salt->buffer);
		return -1;
	}
	if (pinweaver_eal_sha256_update(&ctx, party_u_info->buffer, party_u_info->size)) {
		pinweaver_eal_sha256_final(&ctx, salt->buffer);
		return -1;
	}
	if (pinweaver_eal_sha256_update(&ctx, party_v_info->buffer, party_v_info->size)) {
		pinweaver_eal_sha256_final(&ctx, salt->buffer);
		return -1;
	}

	if (pinweaver_eal_sha256_final(&ctx, salt->buffer))
		return -1;
	salt->size = SHA256_DIGEST_SIZE;
	return 0;
}

TESTABLE_STATIC int start_session(tpm_session_t* session,
		TPM_SE session_type,
		TPMI_DH_ENTITY bind_entity,
		const TPM2B_DIGEST* bind_authorization_value,
		bool salted,
		bool enable_encryption,
		bool force_restart)
{
	TPM2B_DIGEST salt = {0,};
	TPM2B_ENCRYPTED_SECRET encrypted_salt = {0,};
  	TPMI_DH_OBJECT tpm_key = TPM_RH_NULL;

	if (session->handle) {
		if (!force_restart)
			return 0;
		close_session(session);
	}

	if (salted) {
		tpm_key = SALTING_KEY_HANDLE;
		int res = generate_session_salt(&salt, &encrypted_salt);
		if (res) {
			PINWEAVER_EAL_INFO("start_session: failed to create salt");
			return res;
		}
	}

	TPMT_SYM_DEF symmetric;
	if (enable_encryption) {
		symmetric.algorithm = TPM_ALG_AES;
		symmetric.key_bits.aes = 128;
		symmetric.mode.aes = TPM_ALG_CFB;
	} else {
		symmetric.algorithm = TPM_ALG_NULL;
	}

	TPM2B_NONCE nonce_caller;
	nonce_caller.size = SHA256_DIGEST_SIZE;
	int res = pinweaver_eal_rand_bytes(nonce_caller.buffer, nonce_caller.size);
	if (res) {
		PINWEAVER_EAL_INFO("start_session: failed to generate nonce");
		return res;
	}

	TPM2B_NONCE nonce_tpm;

	TPM_RC rc = tss_StartAuthSession(
			tpm_key, NULL /* doesn't matter w/o auth */,
			bind_entity, NULL /* doesn't matter w/o auth */,
			&nonce_caller, &encrypted_salt,
			session_type, &symmetric,
			TPM_ALG_SHA256,
			&session->handle,
			&nonce_tpm,
			NULL);
	if (rc != TPM_RC_SUCCESS) {
		PINWEAVER_EAL_INFO("StartAuthSession failed: %#x", rc);
		return -1;
	}

	if (tss_InitSessionHmac(&session->delegate.hmac,
			session->handle,
			&nonce_tpm, &nonce_caller,
			&salt, bind_authorization_value,
			enable_encryption)) {
		PINWEAVER_EAL_INFO("InitSession failed");
		tss_FlushContext(session->handle, NULL);
		session->handle = 0;
		return -1;
	}

	return 0;
}

TESTABLE_STATIC int start_auth_session(int kind, bool force_restart)
{
	const TPM2B_DIGEST* auth_value = get_auth_value(kind);
	if (!auth_value) {
		PINWEAVER_EAL_INFO("start_auth_session: no auth value");
		return -1;
	}
	int res = start_session(&g_auth_session, TPM_SE_HMAC, TPM_RH_NULL,
			/* bind_auth_value = */ NULL,
			/* salted = */ false, 
			/* enable_encryption = */ false,
			force_restart);
	if (res)
		return res;

	return set_session_auth_value(&g_auth_session, auth_value);
}

TESTABLE_STATIC int start_change_auth_session(
		tpm_session_t* change_auth_session)
{
	if (!change_auth_session)
		return -1;

	const TPM2B_DIGEST* auth_value = get_auth_value(PINWEAVER_EAL_AUTH_PREVIOUS);
	if (!auth_value) {
		PINWEAVER_EAL_INFO("start_change_auth_session: no auth value");
		return -1;
	}

	// Note: using policy session with enable_encryption = true doesn't work
	// for cr50 until https://crrev.com/c/3048088, but should work for TPM2.0
	// rev 1.38+. We do need enable_encryption = true here to avoid exposing the
	// new auth value.
	int res = start_session(change_auth_session, TPM_SE_POLICY,
			TPM_RH_NULL,
			/* bind_auth_value = */ NULL,
			/* salted = */ true,
			/* enable_encryption = */ true,
			/* force_restart = */ true);
	if (res)
		return res;

	TPM_RC rc = tss_PolicyAuthValue(change_auth_session->handle, NULL, NULL);
	if (rc != TPM_RC_SUCCESS) {
		PINWEAVER_EAL_INFO(
				"start_change_auth_session: PolicyAuthValue error %#x", rc);
		close_session(change_auth_session);
		return -1;
	}
	if (set_session_auth_value(change_auth_session, auth_value)) {
		PINWEAVER_EAL_INFO(
				"start_change_auth_session: failed to set authValue");
		close_session(change_auth_session);
		return -1;
	}

	rc = tss_PolicyCommandCode(change_auth_session->handle, NULL,
			TPM_CC_NV_ChangeAuth, NULL);
	if (rc != TPM_RC_SUCCESS) {
		PINWEAVER_EAL_INFO(
				"start_change_auth_session: PolicyCommandCode error %#x", rc);
		close_session(change_auth_session);
		return -1;
	}

	return 0;
}

TESTABLE_STATIC int calc_log_sensitive_data_hash(log_sensitive_data_t* data,
		uint8_t* res /* SHA256_DIGEST_SIZE */)
{
	return calc_sha256(&data->entry, sizeof(data->entry), res);
}

TESTABLE_STATIC int verify_log_sensitive_data(log_sensitive_data_t* data)
{
	uint8_t res[SHA256_DIGEST_SIZE];
	if (calc_log_sensitive_data_hash(data, res))
		return -1;
	if (pinweaver_eal_safe_memcmp(res, data->hash, SHA256_DIGEST_SIZE))
		return -1;
	return 0;
}

TESTABLE_STATIC int decrypt_log_entry(
		const tpm_storage_log_entry_t* tpm_entry, int kind,
		cached_log_entry_t* entry)
{
	const uint8_t* key = get_data_key(kind);
	if (key == NULL)
		return -1;

	log_sensitive_data_t plaintext;
	if (pinweaver_eal_aes256_ctr(key, DATA_KEY_SIZE, tpm_entry->iv,
			tpm_entry->encrypted_data, sizeof(log_sensitive_data_t),
			&plaintext)) {
		PINWEAVER_EAL_INFO("decrypt_log_entry: AES failed, kind=%d", kind);
		return -1;
	}
	if (verify_log_sensitive_data(&plaintext) != 0) {
		PINWEAVER_EAL_INFO("decrypt_log_entry: verification failed, kind=%d",
				kind);
		return -1;
	}

	return pinweaver_eal_memcpy_s(&entry->data, sizeof(entry->data),
			&plaintext.entry, sizeof(struct pw_get_log_entry_t));
}

TESTABLE_STATIC int encrypt_log_entry(
		const struct pw_get_log_entry_t* log_entry,
		uint32_t counter, tpm_storage_log_entry_t* tpm_entry)
{
	log_sensitive_data_t plaintext;
	const uint8_t* key = get_data_key(PINWEAVER_EAL_AUTH_CURRENT);
	if (key == NULL)
		return -1;

	if (pinweaver_eal_memcpy_s(&plaintext.entry, sizeof(plaintext.entry),
			log_entry, sizeof(struct pw_get_log_entry_t))) {
		PINWEAVER_EAL_INFO("encrypt_log_entry: entry copy failed");
		return -1;
	}
	if (calc_log_sensitive_data_hash(&plaintext, plaintext.hash)) {
		PINWEAVER_EAL_INFO("encrypt_log_entry: hash failed");
		return -1;
	}

	tpm_entry->counter = counter;
	tpm_entry->version = 0;
	pinweaver_eal_rand_bytes(tpm_entry->iv,
				 sizeof(tpm_entry->iv));
	if (pinweaver_eal_aes256_ctr(key, DATA_KEY_SIZE, tpm_entry->iv,
			&plaintext, sizeof(log_sensitive_data_t),
			tpm_entry->encrypted_data)) {
		PINWEAVER_EAL_INFO("encrypt_log_entry: AES failed");
		return -1;
	}

	return 0;
}

TESTABLE_STATIC int cache_order_ins(int pos, int entry_num)
{
	if (pos < 0 || pos >= MAX_LOG_ENTRIES)
		return -1;
	memmove(g_cache_order + pos + 1, g_cache_order + pos,
			(MAX_LOG_ENTRIES - pos - 1) * sizeof(g_cache_order[0]));
	g_cache_order[pos] = entry_num;
	return 0;
}

TESTABLE_STATIC int cache_order_ins_entry_num(cached_log_entry_t* entry,
		int entry_num)
{
	int pos;
	for (pos = 0; pos < MAX_LOG_ENTRIES; ++pos) {
		int other_entry_num = g_cache_order[pos];
		if (other_entry_num < 0)
			break;
		if (other_entry_num >= MAX_LOG_ENTRIES)
			return -1;
		cached_log_entry_t* other_entry =
				&g_cached_log_entries[other_entry_num];
		if (other_entry->counter < entry->counter) {
			break;
		} else if (other_entry->counter == entry->counter) {
			if (entry->counter != 0)
				return -1;
			break;
		}
	}
	return cache_order_ins(pos, entry_num);
}

TESTABLE_STATIC int cache_order_del(int pos)
{
	if (pos < 0 || pos >= MAX_LOG_ENTRIES)
		return -1;
	memmove(g_cache_order + pos, g_cache_order + pos + 1,
		(MAX_LOG_ENTRIES - pos - 1) * sizeof(g_cache_order[0]));
	g_cache_order[MAX_LOG_ENTRIES - 1] = -1;
	return 0;
}

TESTABLE_STATIC int cache_order_del_entry_num(int entry_num)
{
	int pos;
	for (pos = 0; pos < MAX_LOG_ENTRIES; ++pos) {
		if (g_cache_order[pos] == entry_num)
			return cache_order_del(pos);
	}
	return -1;
}

TESTABLE_STATIC int get_nv_handle_info(TPM_HANDLE handle,
		bool must_exist,
		size_t expected_size,
		TPM2B_NAME* name,
		bool* exists,
		bool* written)
{
	TPM2B_NV_PUBLIC public_area;
	TPM_RC rc = tss_NV_ReadPublic(handle, NULL /* doesn't matter w/o auth */,
		&public_area, name, /* authorization_delegate = */ NULL);
	if (rc != TPM_RC_SUCCESS) {
		if (rc == TPM_RC_HANDLE + TPM_RC_H + TPM_RC_1) {
			PINWEAVER_EAL_INFO("get_nv_handle_info %#x: doesn't exist", handle);
			if (exists)
				*exists = false;
			if (written)
				*written = false;
			return must_exist ? -1 : 0;
		}
		PINWEAVER_EAL_INFO("get_nv_handle_info %#x: error %#x", handle, rc);
		return -1;
	}
	if (exists)
		*exists = true;
	if (written)
		*written = (public_area.nv_public.attributes & TPMA_NV_WRITTEN) != 0;
	if (check_nv_space(handle, name, expected_size, &public_area.nv_public)) {
		PINWEAVER_EAL_INFO("get_nv_handle_info %#x: unexpected space name",
				handle);
		return -1;
	}
	return 0;
}

TESTABLE_STATIC int read_log_entry(TPM_HANDLE handle,
		cached_log_entry_t* entry,
		int* is_old_key)
{
	TPM2B_NAME name;
	bool written;
	const size_t size = sizeof(tpm_storage_log_entry_t);
	if (get_nv_handle_info(handle, /* must_exist = */ true, size,
			&name, NULL, &written))
		return -1;
	if (!written) {
		PINWEAVER_EAL_INFO("read_log_entry %#x: uninitialized", handle);
		entry->counter = 0;
		return 0;
	}

	TPM2B_MAX_NV_BUFFER buffer;
	tpm_storage_log_entry_t* tpm_entry =
			(tpm_storage_log_entry_t*)buffer.buffer;

	TPM_RC rc = tss_NV_Read(handle,
			&name,
			handle,
			&name,
			size,
			0,
			&buffer,
			&g_auth_session.delegate);

	if (rc != TPM_RC_SUCCESS) {
		PINWEAVER_EAL_INFO("read_log_entry %#x: error %#x", handle, rc);
		return -1;
	}
	if (buffer.size != size) {
		PINWEAVER_EAL_INFO("read_log_entry %#x: unexpected size (%u)",
				handle, buffer.size);
		return -1;
	}
	entry->counter = tpm_entry->counter;
	if (tpm_entry->counter > g_max_counter)
		g_max_counter = tpm_entry->counter;

	if (entry->counter) {
		if (decrypt_log_entry(tpm_entry, PINWEAVER_EAL_AUTH_CURRENT, entry)) {
			PINWEAVER_EAL_INFO("read_log_entry %#x: trying old key", handle);
			if (decrypt_log_entry(tpm_entry, PINWEAVER_EAL_AUTH_PREVIOUS, entry)) {
				PINWEAVER_EAL_INFO("read_log_entry %#x: decrypt failed", handle);
				entry->counter = 0;
				return -1;
			}
			PINWEAVER_EAL_INFO("read_log_entry %#x: decrypted with old key", handle);
			if (is_old_key)
				*is_old_key = 1;
		}
	}
	return 0;
}

TESTABLE_STATIC int read_and_insert_log_entry(int entry_num, int* need_writing)
{
	if (entry_num < 0 || entry_num >= MAX_LOG_ENTRIES)
		return -1;
	if (!need_writing)
		return -1;

	TPM_HANDLE handle = LOG_ENTRY_FIRST_HANDLE + entry_num;
	cached_log_entry_t* entry = g_cached_log_entries + entry_num;
	int is_old_key = 0;

	if (read_log_entry(handle, entry, &is_old_key))
		return -1;

	need_writing[entry_num] = is_old_key;
	return cache_order_ins_entry_num(entry, entry_num);
}

TESTABLE_STATIC int write_log_entry_by_handle(TPM_HANDLE handle,
		tpm_storage_log_entry_t* tpm_entry)
{
	TPM2B_MAX_NV_BUFFER tpm_data;
	if (pinweaver_eal_memcpy_s(tpm_data.buffer, sizeof(tpm_data.buffer),
			tpm_entry, sizeof(tpm_storage_log_entry_t)))
		return -1;
	tpm_data.size = sizeof(tpm_storage_log_entry_t);

	TPM2B_NAME name;
	const size_t size = sizeof(tpm_storage_log_entry_t);

	const int kMaxRetries = 3;
	int retries;
	for (retries = 0; retries < kMaxRetries; ++retries) {
		if (get_nv_handle_info(handle, /* must_exist = */ true, size,
				&name, NULL, NULL))
			return -1;

		if (start_auth_session(PINWEAVER_EAL_AUTH_CURRENT, retries > 0))
			return -1;

		TPM_RC rc = tss_NV_Write(handle,
				&name,
				handle,
				&name,
				&tpm_data,
				/* offset = */ 0,
				&g_auth_session.delegate);
		if (rc == TPM_RC_SUCCESS)
			return 0;
		PINWEAVER_EAL_INFO("write_log_entry: write %#x attempt %d/%d failed %#x",
			handle, retries+1, kMaxRetries, rc);
	}
	return -1;
}

TESTABLE_STATIC int write_log_entry(int entry_num,
		tpm_storage_log_entry_t* tpm_entry)
{
	return write_log_entry_by_handle(LOG_ENTRY_FIRST_HANDLE + entry_num,
			tpm_entry);
}

TESTABLE_STATIC int clear_persist_entry()
{
	tpm_storage_log_entry_t tpm_entry = { 0, };
	return write_log_entry_by_handle(LOG_ENTRY_PERSIST_HANDLE, &tpm_entry);
}

TESTABLE_STATIC int write_back_log_entry(int entry_num, bool use_persist)
{
	PINWEAVER_EAL_INFO("write_back_log_entry %d", entry_num);
	tpm_storage_log_entry_t tpm_entry;
	cached_log_entry_t* entry = &g_cached_log_entries[entry_num];
	if (encrypt_log_entry(&entry->data, entry->counter, &tpm_entry))
		return -1;
	if (use_persist) {
		if (write_log_entry_by_handle(LOG_ENTRY_PERSIST_HANDLE, &tpm_entry))
			return -1;
	}
	return write_log_entry(entry_num, &tpm_entry);
}

TESTABLE_STATIC int read_log_entries()
{
	int entry_num;
	int need_writing[MAX_LOG_ENTRIES] = {0, };
	cached_log_entry_t* empty_entry = NULL;
	int empty_entry_num = -1;
	bool do_clear_persist_entry = false;

	if (g_storage_state == TPMSS_READY)
		return 0;
	
	for (entry_num = 0; entry_num < MAX_LOG_ENTRIES; ++entry_num) {
		g_cache_order[entry_num] = -1;
	}

	for (entry_num = 0; entry_num < MAX_LOG_ENTRIES; ++entry_num) {
		if (read_and_insert_log_entry(entry_num, need_writing))
			return -1;
		cached_log_entry_t* entry = &g_cached_log_entries[entry_num];
		if (empty_entry == NULL && entry->counter == 0) {
			empty_entry_num = entry_num;
			empty_entry = entry;
		}
	}

	if (empty_entry) {
		cached_log_entry_t persist_entry;

		if (read_log_entry(LOG_ENTRY_PERSIST_HANDLE, &persist_entry, NULL))
			return -1;
		if (persist_entry.counter) {
			for (entry_num = 0; entry_num < MAX_LOG_ENTRIES; ++entry_num) {
				if (g_cached_log_entries[entry_num].counter ==
						persist_entry.counter)
					break;
			}
			if (entry_num >= MAX_LOG_ENTRIES) {
				if (cache_order_del_entry_num(empty_entry_num))
					return -1;
				empty_entry->counter = persist_entry.counter;
				if (pinweaver_eal_memcpy_s(&empty_entry->data,
						sizeof(empty_entry->data),
						&persist_entry.data,
						sizeof(struct pw_get_log_entry_t)))
					return -1;
				if (cache_order_ins_entry_num(empty_entry, empty_entry_num))
					return -1;
				if (write_back_log_entry(empty_entry_num,
						/* use_persist = */ false)) {
					return -1;
				}
				do_clear_persist_entry = true;
			}
		}
	}

	for (entry_num = 0; entry_num < MAX_LOG_ENTRIES; ++entry_num) {
		if (need_writing[entry_num]) {
			do_clear_persist_entry = true;
			if (write_back_log_entry(entry_num, /* use_persist = */ true))
				return -1;
		}
	}

	if (do_clear_persist_entry) {
		clear_persist_entry();  // ignore errors
	}

	return 0;
}

TESTABLE_STATIC void set_initial_log_entries()
{
	int entry_num;
	for (entry_num = 0; entry_num < MAX_LOG_ENTRIES; ++entry_num) {
		memset(&g_cached_log_entries[entry_num], 0, sizeof(cached_log_entry_t));
		g_cache_order[entry_num] = MAX_LOG_ENTRIES - entry_num - 1;
	}
}

TESTABLE_STATIC void set_initial_tree_descr()
{
	memset(&g_tree_descriptor, 0, sizeof(g_tree_descriptor));
	g_tree_descriptor_filled = false;
}

TESTABLE_STATIC void set_initial_state()
{
	set_initial_log_entries();
	set_initial_tree_descr();
}

TESTABLE_STATIC int change_auth_handle(TPM_HANDLE handle, size_t size)
{
	TPM2B_NAME name;
	const TPM2B_DIGEST* new_auth_value = get_auth_value(PINWEAVER_EAL_AUTH_CURRENT);

	if (!new_auth_value) {
		PINWEAVER_EAL_INFO("change_auth_handle: no new auth value");
		return -1;
	}

	bool exists, written;
	if (get_nv_handle_info(handle,
			/* must_exist = */ false, size,
			&name, &exists, &written))
		return -1;
	PINWEAVER_EAL_INFO("change_auth_handle %#x: exists=%d written=%d\n",
			handle, exists, written);

	tpm_session_t change_auth_session = { 0, };
	if (start_change_auth_session(&change_auth_session))
		return -1;
	if (set_future_session_auth_value(&change_auth_session, new_auth_value)) {
		PINWEAVER_EAL_INFO("change_auth_handle %#x: failed to set authValue",
				handle);
		close_session(&change_auth_session);
		return -1;
	}
	TPM_RC rc = tss_NV_ChangeAuth(handle, &name, new_auth_value,
			&change_auth_session.delegate);
	if (rc != TPM_RC_SUCCESS) {
		PINWEAVER_EAL_INFO("change_auth_handle %#x: error %#x - ignoring",
				handle, rc);
	}
	close_session(&change_auth_session);
	return 0;
}

TESTABLE_STATIC int change_auth()
{
	int entry_num;

	for (entry_num = 0; entry_num < MAX_LOG_ENTRIES; ++entry_num) {
		if (change_auth_handle(LOG_ENTRY_FIRST_HANDLE + entry_num,
				sizeof(tpm_storage_log_entry_t)))
			return -1;
	}
	if (change_auth_handle(LOG_ENTRY_PERSIST_HANDLE,
			sizeof(tpm_storage_log_entry_t)))
		return -1;
	if (change_auth_handle(TREE_DESCRIPTOR_HANDLE,
			sizeof(g_tree_descriptor)))
		return -1;

	if (start_auth_session(PINWEAVER_EAL_AUTH_CURRENT, true))
		return -1;
	return 0;
}

TESTABLE_STATIC int read_tree_descr()
{
	TPM2B_MAX_NV_BUFFER buffer;
	TPM2B_NAME name;
	bool exists;
	bool written;
	const size_t size = sizeof(g_tree_descriptor);

	if (g_tree_descriptor_filled)
		return 0;

	PINWEAVER_EAL_INFO("read_tree_descr: read from TPM");
	if (get_nv_handle_info(TREE_DESCRIPTOR_HANDLE,
			/* must_exist = */ false, size,
			&name, &exists, &written))
		return -1;
	if (!exists) {
		PINWEAVER_EAL_INFO("read_tree_descr: no tree descr");
		g_storage_state = TPMSS_NOT_INITIALIZED;
		return -1;
	}

	TPM_RC rc = tss_NV_Read(TREE_DESCRIPTOR_HANDLE,
			&name,
			TREE_DESCRIPTOR_HANDLE,
			&name,
			size,
			0,
			&buffer,
			&g_auth_session.delegate);
	if (rc != TPM_RC_SUCCESS) {
		if ((rc == TPM_RC_BAD_AUTH + TPM_RC_S + TPM_RC_1) ||
		    (rc == TPM_RC_AUTH_FAIL + TPM_RC_S + TPM_RC_1)) {
			PINWEAVER_EAL_INFO("read_tree_descr: change auth (%#x)", rc);
			if (change_auth())
				return -1;
			PINWEAVER_EAL_INFO("read_tree_descr: change auth done");
			rc = tss_NV_Read(TREE_DESCRIPTOR_HANDLE,
					&name,
					TREE_DESCRIPTOR_HANDLE,
					&name,
					size,
					0,
					&buffer,
					&g_auth_session.delegate);
		}
		if (rc == TPM_RC_NV_UNINITIALIZED) {
			PINWEAVER_EAL_INFO("read_tree_descr: empty");
			set_initial_state();
			g_storage_state = TPMSS_READY;
			return 0;
		}
		if (rc != TPM_RC_SUCCESS) {
			PINWEAVER_EAL_INFO("read_tree_descr: error %#x", rc);
			return -1;
		}
	}
	if (buffer.size != size) {
		PINWEAVER_EAL_INFO("read_tree_descr: unexpected size (%u)", buffer.size);
		return -1;
	}
	if (pinweaver_eal_memcpy_s(&g_tree_descriptor, sizeof(g_tree_descriptor),
			buffer.buffer, sizeof(g_tree_descriptor))) {
		PINWEAVER_EAL_INFO("read_tree_descr: failed to copy descriptor");
		return -1;
	}

	g_tree_descriptor_filled = true;
	return 0;
}

TESTABLE_STATIC int write_tree_descr(const tpm_storage_tree_descriptor_t *data)
{
	TPM2B_MAX_NV_BUFFER tpm_data;
	const size_t size = sizeof(g_tree_descriptor);
	if (pinweaver_eal_memcpy_s(tpm_data.buffer, sizeof(tpm_data.buffer),
			data, size))
		return -1;
	tpm_data.size = size;

	TPM2B_NAME name;
	if (get_nv_handle_info(TREE_DESCRIPTOR_HANDLE,
			/* must_exist = */ true, size,
			&name, NULL, NULL))
		return -1;

	const int kMaxRetries = 3;
	int retries;
	for (retries = 0; retries < kMaxRetries; ++retries) {
		if (start_auth_session(PINWEAVER_EAL_AUTH_CURRENT, retries > 0))
			return -1;

		TPM_RC rc = tss_NV_Write(TREE_DESCRIPTOR_HANDLE,
				&name,
				TREE_DESCRIPTOR_HANDLE,
				&name,
				&tpm_data,
				/* offset = */ 0,
				&g_auth_session.delegate);
		if (rc == TPM_RC_SUCCESS)
			return 0;
		PINWEAVER_EAL_INFO("write_tree_descr: write %#x attempt %d/%d failed %#x",
			TREE_DESCRIPTOR_HANDLE, retries+1, kMaxRetries, rc);
	}
	return -1;
}

TESTABLE_STATIC int derive_pw_key(
		const uint8_t* device_key /* DEVICE_KEY_SIZE=256-bit */,
		const uint8_t* object_const /* PW_OBJ_CONST_SIZE */,
		const uint8_t* nonce /* PW_NONCE_SIZE */,
		uint8_t* result /* SHA256_DIGEST_SIZE */)
{
	pinweaver_eal_hmac_sha256_ctx_t hash;
	if (pinweaver_eal_hmac_sha256_init(&hash, device_key, DEVICE_KEY_SIZE))
		return -1;
	if (pinweaver_eal_hmac_sha256_update(&hash, object_const, PW_OBJ_CONST_SIZE)) {
		pinweaver_eal_hmac_sha256_final(&hash, result);
		return -1;
	}
	if (pinweaver_eal_hmac_sha256_update(&hash, nonce, PW_NONCE_SIZE)) {
		pinweaver_eal_hmac_sha256_final(&hash, result);
		return -1;
	}
	return pinweaver_eal_hmac_sha256_final(&hash, result);
}

TESTABLE_STATIC void *secure_memset(void *ptr, int value, size_t num)
{
	volatile unsigned char *v_ptr = ptr;
	while (num--)
		*(v_ptr++) = value;
	return ptr;
}

int pinweaver_eal_derive_keys(struct merkle_tree_t *merkle_tree)
{
	const uint8_t kWrapKeyConst[PW_OBJ_CONST_SIZE] = 
			{'W', 'R', 'A', 'P', 'W', 'R', 'A', 'P'};
	const uint8_t kHmacKeyConst[PW_OBJ_CONST_SIZE] =
			{'H', 'M', 'A', 'C', 'H', 'M', 'A', 'C'};
	uint8_t device_key[DEVICE_KEY_SIZE];
	if (pinweaver_eal_get_device_key(PINWEAVER_EAL_CONST, device_key))
		return -1;

	if (derive_pw_key(device_key, kWrapKeyConst,
			merkle_tree->key_derivation_nonce, merkle_tree->wrap_key))
		return -1;

	if (derive_pw_key(device_key, kHmacKeyConst,
			merkle_tree->key_derivation_nonce, merkle_tree->hmac_key))
		return -1;

        // Do not leave the content of the deivce key on the stack.
	secure_memset(device_key, 0, sizeof(device_key));

	return 0;
}

int pinweaver_eal_storage_start(void)
{
	const int kMaxRetries = 10;
	int retries;

	if (g_storage_state == TPMSS_READY)
		return 0;

	PINWEAVER_EAL_INFO("pinweaver_eal_storage_start: starting");
	set_initial_state();
	g_storage_state = TPMSS_NOT_STARTED;

	for (retries = 0; retries < kMaxRetries; ++retries) {
		if (start_auth_session(PINWEAVER_EAL_AUTH_CURRENT, true) != 0)
			return -1;
		if (read_tree_descr()) {
			if (g_storage_state == TPMSS_NOT_INITIALIZED)
				return -1;
			continue;
		}
		if (read_log_entries())
			continue;

		/* update restart count in the descriptor */
		++g_tree_descriptor.restart_count;
		if (write_tree_descr(&g_tree_descriptor))
			return -1;

		g_storage_state = TPMSS_READY;
		return 0;
	}

	return -1;
}

int pinweaver_eal_storage_init_state(uint8_t root_hash[PW_HASH_SIZE],
				     uint32_t *restart_count)
{
	if (pinweaver_eal_storage_start())
		return -1;

	struct cached_log_entry_t* first = get_first_cached_log_entry();

	if (!first)
		return -1;

	if (pinweaver_eal_memcpy_s(root_hash, PW_HASH_SIZE,
			first->data.root, PW_HASH_SIZE))
		return -1;

	*restart_count = g_tree_descriptor.restart_count;
	return 0;
}

TESTABLE_STATIC void log_entry_set_default(
		struct pw_get_log_entry_t* entry)
{
	memset(entry, 0, sizeof(struct pw_get_log_entry_t));
}

int pinweaver_eal_storage_get_log(struct pw_log_storage_t *dest)
{
	if (pinweaver_eal_storage_start())
		return -1;

	int entry_num;
	for (entry_num = 0;
			entry_num < PW_LOG_ENTRY_COUNT &&
			entry_num < MAX_LOG_ENTRIES;
			++entry_num) {
		int nv_num = g_cache_order[entry_num];
		if (g_cached_log_entries[nv_num].counter != 0) {
			if (pinweaver_eal_memcpy_s(&dest->entries[entry_num],
					sizeof(dest->entries[entry_num]),
					&g_cached_log_entries[nv_num].data,
					sizeof(struct pw_get_log_entry_t)))
				return -1;
		} else {
			log_entry_set_default(&dest->entries[entry_num]);
		}
	}
	return 0;
}

int pinweaver_eal_storage_set_log(const struct pw_log_storage_t *log)
{
	if (pinweaver_eal_storage_start())
		return -1;

	int entry_num = get_last_cached_log_entry_num();
	if (entry_num < 0)
		return -1;

	uint32_t counter = ++g_max_counter;

	tpm_storage_log_entry_t tpm_entry;
	if (encrypt_log_entry(&log->entries[0], counter, &tpm_entry))
		return -1;
	if (write_log_entry(entry_num, &tpm_entry))
		return -1;

	cached_log_entry_t* entry = &g_cached_log_entries[entry_num];
	if (pinweaver_eal_memcpy_s(&entry->data, sizeof(entry->data),
			&log->entries[0], sizeof(struct pw_get_log_entry_t)))
		return -1;
	entry->counter = counter;
	memmove(g_cache_order + 1, g_cache_order,
		(MAX_LOG_ENTRIES - 1) * sizeof(g_cache_order[0]));
	g_cache_order[0] = entry_num;

	return 0;
}

int pinweaver_eal_storage_get_tree_data(struct pw_long_term_storage_t *dest)
{
	pinweaver_eal_storage_start();
	if (pinweaver_eal_memcpy_s(dest, sizeof(struct pw_long_term_storage_t),
			&g_tree_descriptor.descriptor, sizeof(struct pw_long_term_storage_t)))
		return -1;
	return g_tree_descriptor_filled ? 0 : -1;
}

int pinweaver_eal_storage_set_tree_data(
		const struct pw_long_term_storage_t *data)
{
	if (pinweaver_eal_storage_start())
		return -1;

	set_initial_state();
	if (pinweaver_eal_memcpy_s(&g_tree_descriptor.descriptor,
			sizeof(g_tree_descriptor.descriptor), data,
			sizeof(struct pw_long_term_storage_t)))
		return -1;
	g_tree_descriptor_filled = true;
	g_data_key_obtained[PINWEAVER_EAL_AUTH_CURRENT] = false;
	g_data_key_obtained[PINWEAVER_EAL_AUTH_PREVIOUS] = false;

	tpm_storage_log_entry_t tpm_entry;
	memset(&tpm_entry, 0, sizeof(tpm_entry));
	int res = 0;
	int entry_num;
	for (entry_num = 0; entry_num < MAX_LOG_ENTRIES; ++entry_num) {
		if (write_log_entry(entry_num, &tpm_entry))
			res = -1;
	}
	if (write_tree_descr(&g_tree_descriptor))
		res = -1;

	return res;
}

TESTABLE_STATIC int define_space(tpm_session_t* session,
		TPM_HANDLE handle, size_t data_size)
{
	const TPM2B_DIGEST* auth_value = get_auth_value(0);
	if (!auth_value)
		return -1;

	TPM2B_NV_PUBLIC public_area;
	public_area.size = sizeof(TPMS_NV_PUBLIC);
	set_nv_public_area(handle, data_size, false, &public_area.nv_public);

	TPM2B_NAME owner_name;
	name_set_handle(&owner_name, TPM_RH_OWNER);
	TPM_RC rc = tss_NV_DefineSpace(TPM_RH_OWNER, &owner_name,
			auth_value, &public_area, &session->delegate);
	if (rc != TPM_RC_SUCCESS && rc != TPM_RC_NV_DEFINED) {
		PINWEAVER_EAL_INFO("define_space %#x failed %#x", handle, rc);
		return -1;
	}
	if (rc == TPM_RC_NV_DEFINED) {
		PINWEAVER_EAL_INFO("define_space %#x: already defined", handle);
	}
	return 0;
}

TESTABLE_STATIC int create_nvmem_spaces()
{
	tpm_session_t salted_session = { 0, };
	if (start_session(&salted_session, TPM_SE_HMAC, TPM_RH_NULL,
			/* bind_auth_value = */ NULL,
			/* salted = */ true, 
			/* enable_encryption = */ true,
			/* force_restart = */ true))
		return -1;

	int entry_num;
	int res = -1;
	do {
		for (entry_num = 0; entry_num < MAX_LOG_ENTRIES; ++entry_num) {
			if (define_space(&salted_session,
					LOG_ENTRY_FIRST_HANDLE + entry_num,
					sizeof(tpm_storage_log_entry_t)))
				break;
		}
		if (entry_num < MAX_LOG_ENTRIES)
			break;

		if (define_space(&salted_session,
				LOG_ENTRY_PERSIST_HANDLE,
				sizeof(tpm_storage_log_entry_t)))
			break;
		if (define_space(&salted_session,
				TREE_DESCRIPTOR_HANDLE,
				sizeof(g_tree_descriptor)))
			break;

		PINWEAVER_EAL_INFO("create_nvmem_spaces: success");
		res = 0;
	} while(0);
	close_session(&salted_session);

	return res;
}

int pinweaver_eal_storage_initialize_owner()
{
	PINWEAVER_EAL_INFO("pinweaver_eal_storage_initialize_owner");
	if (g_storage_state == TPMSS_NOT_STARTED)
		pinweaver_eal_storage_start();
	if (g_storage_state == TPMSS_READY)
		return 0;
	if (g_storage_state != TPMSS_NOT_INITIALIZED)
		return -1;

	PINWEAVER_EAL_INFO("pinweaver_eal_storage_initialize_owner: creating spaces");
	const int kMaxRetries = 10;
	int retries;
	for (retries = 0; retries < kMaxRetries; ++retries) {
		if (create_nvmem_spaces() == 0) {
			PINWEAVER_EAL_INFO("pinweaver_eal_storage_initialize_owner: done");
			g_storage_state = TPMSS_READY;
			return 0;
		}
	}
	return -1;
}
