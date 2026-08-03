// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <arpa/inet.h>
#include <memory>

#include "fuzzer_provider.h"

extern "C" {
#include "pinweaver_eal.h"
#include "pinweaver_eal_tpm.h"
#include "tss.h"
#include "tss_serde.h"
} // extern "C"

namespace {

// TPM message header size.
constexpr size_t kHeaderSize = 10;
// Probability in % of generating a pure random value or byte stream.
constexpr uint32_t kPureRandomProb = 5;
// Probability in % of generating an error response.
constexpr uint32_t kErrorResponseProb = 20;
// Probability in % of generating a response of minimal size.
constexpr uint32_t kMinimalResponseProb = 80;
// Max fuzzed message length.
constexpr size_t kMaxMessageLength = 512;

void Serialize_uint16_t(uint16_t value, std::string* buffer) {
  uint16_t value_net = htons(value);
  const char* value_bytes = reinterpret_cast<const char*>(&value_net);
  buffer->append(value_bytes, sizeof(uint16_t));
}

void Serialize_uint32_t(uint32_t value, std::string* buffer) {
  uint32_t value_net = htonl(value);
  const char* value_bytes = reinterpret_cast<const char*>(&value_net);
  buffer->append(value_bytes, sizeof(uint32_t));
}

int Parse_uint16_t(std::string* buffer, uint16_t* value) {
  if (buffer->size() < sizeof(uint16_t))
    return -1;
  uint16_t value_net = 0;
  memcpy(&value_net, buffer->data(), sizeof(uint16_t));
  *value = ntohs(value_net);
  buffer->erase(0, sizeof(uint16_t));
  return 0;
}

int Parse_uint32_t(std::string* buffer, uint32_t* value) {
  if (buffer->size() < sizeof(uint32_t))
    return -1;
  uint32_t value_net = 0;
  memcpy(&value_net, buffer->data(), sizeof(uint32_t));
  *value = ntohl(value_net);
  buffer->erase(0, sizeof(uint32_t));
  return 0;
}

std::string BuildHeader(uint16_t tag, uint32_t size, uint32_t code) {
  std::string header;
  Serialize_uint16_t(tag, &header);
  Serialize_uint32_t(size, &header);
  Serialize_uint32_t(code, &header);
  return header;
}

bool ParseHeader(const std::string& command,
                 uint16_t* tag,
                 uint32_t* size,
                 uint32_t* code) {
  std::string header(command, 0, kHeaderSize);
  if (Parse_uint16_t(&header, tag)) {
    return false;
  }
  if (Parse_uint32_t(&header, size)) {
    return false;
  }
  if (Parse_uint32_t(&header, code)) {
    return false;
  }
  return true;
}

bool ConsumeBoolWithProbability(uint32_t probability) {
  return g_data_provider->ConsumeIntegralInRange<uint32_t>(0, 99) < probability;
}

std::string ConsumeRandomMessage() {
  return g_data_provider->ConsumeRandomLengthString(kMaxMessageLength);
}

uint32_t ConsumeUint32() {
  return g_data_provider->ConsumeIntegralInRange<uint32_t>(0, 0xFFFFFFFFu);
}

uint32_t ConsumeResponseCode() {
  // Decide between realistic and purely random value.
  if (ConsumeBoolWithProbability(kPureRandomProb)) {
    return ConsumeUint32();
  }
  uint32_t rc = g_data_provider->ConsumeIntegralInRange<uint32_t>(0, 0xFFF);
  if (g_data_provider->ConsumeBool()) {
    // Generate WARN or FMT0 error RC.
    rc &= 0x97F;
  } else {
    // Generate FMT1 error RC.
    rc |= RC_FMT1;
  }
  return rc;
}

uint32_t ConsumeHandle() {
  uint32_t handle = ConsumeUint32();
  // Decide between realistic and purely random value.
  if (!ConsumeBoolWithProbability(kPureRandomProb)) {
    handle &= 0xC3000003u;
  }
  return handle;
}

std::string ConsumeHandles(size_t qnt_handles) {
  std::string handles;
  for (; qnt_handles > 0; --qnt_handles) {
    Serialize_uint32_t(ConsumeHandle(), &handles);
  }
  return handles;
}

std::string ConsumePayload(size_t pre_payload_size) {
  if (kMaxMessageLength <= pre_payload_size) {
    return std::string();
  }
  return g_data_provider->ConsumeRandomLengthString(kMaxMessageLength -
                                                    pre_payload_size);
}

std::string ConsumeResponseForCommand(const std::string& command) {
  // With low probability return a completely random message.
  if (ConsumeBoolWithProbability(kPureRandomProb)) {
    return ConsumeRandomMessage();
  }

  // Parse command, use defaults in case of parsing errors.
  uint16_t cmd_tag = TPM_ST_NO_SESSIONS;
  uint32_t cmd_code = TPM_CC_FIRST;
  uint32_t cmd_size = 0;
  ParseHeader(command, &cmd_tag, &cmd_size, &cmd_code);

  // Decide if we want to return an error or success.
  uint32_t resp_code;
  std::string handles;
  if (ConsumeBoolWithProbability(kErrorResponseProb)) {
    resp_code = ConsumeResponseCode();
  } else {
    resp_code = TPM_RC_SUCCESS;
    handles = ConsumeHandles(tss_GetNumberOfResponseHandles(cmd_code));
  }

  // Error or success, with high probability return response of minimal size.
  std::string payload;
  if (!ConsumeBoolWithProbability(kMinimalResponseProb)) {
    payload = ConsumePayload(kHeaderSize + handles.size());
  }

  return BuildHeader(cmd_tag, kHeaderSize + handles.size() + payload.size(),
                     resp_code) +
         handles + payload;
}

}  // namespace

extern "C" {

void pinweaver_eal_send_command_and_wait(const char *request, char *response) {
  uint32_t size_field = 0;
  memcpy(&size_field, request + 2, sizeof(uint32_t));
  const size_t request_size = ntohs(size_field);
  std::string request_str = std::string(request, request_size);
  std::string response_str = ConsumeResponseForCommand(request_str);
  memset(response, 0, 10);
  memcpy(response, response_str.data(), response_str.size());
}

int pinweaver_eal_sha256_init(pinweaver_eal_sha256_ctx_t *ctx) {
  if (g_data_provider->ConsumeBool())
    return -1;
  int rv = SHA256_Init(ctx);
  return rv == 1 ? 0 : -1;
}

int pinweaver_eal_sha256_update(pinweaver_eal_sha256_ctx_t *ctx,
                                const void *data, size_t size) {
  if (g_data_provider->ConsumeBool())
    return -1;
  int rv = SHA256_Update(ctx, data, size);
  return rv == 1 ? 0 : -1;
}

int pinweaver_eal_sha256_final(pinweaver_eal_sha256_ctx_t *ctx, void *res) {
  if (g_data_provider->ConsumeBool())
    return -1;
  int rv = SHA256_Final((unsigned char *)res, ctx);
  return rv == 1 ? 0 : -1;
}

int pinweaver_eal_hmac_sha256_init(pinweaver_eal_hmac_sha256_ctx_t *ctx,
                                   const void *key,
                                   size_t key_size /* in bytes */) {
  if (g_data_provider->ConsumeBool())
    return -1;
  *ctx = HMAC_CTX_new();
  if (!*ctx) {
    return -1;
  }
  int rv = HMAC_Init_ex(*ctx, key, key_size, EVP_sha256(), NULL);
  return rv == 1 ? 0 : -1;
}
int pinweaver_eal_hmac_sha256_update(pinweaver_eal_hmac_sha256_ctx_t *ctx,
                                     const void *data, size_t size) {
  if (g_data_provider->ConsumeBool())
    return -1;
  int rv = HMAC_Update(*ctx, (const unsigned char *)data, size);
  return rv == 1 ? 0 : -1;
}

int pinweaver_eal_hmac_sha256_final(pinweaver_eal_hmac_sha256_ctx_t *ctx,
                                    void *res) {
  unsigned int len;
  int rv = HMAC_Final(*ctx, (unsigned char *)res, &len);
  HMAC_CTX_free(*ctx);
  if (g_data_provider->ConsumeBool())
    return -1;
  *ctx = NULL;
  return rv == 1 ? 0 : -1;
}

int pinweaver_eal_aes128_cfb(const void *key, size_t key_size, /* in bytes */
                             const void *iv, const void *data, size_t size,
                             int op_type, /* PINWEAVER_EAL_{DE|EN}CRYPT */
                             void *res) {
  if (g_data_provider->ConsumeBool())
    return -1;

  EVP_CIPHER_CTX *ctx;
  int rv;
  int len, len_final;

  ctx = EVP_CIPHER_CTX_new();
  if (!ctx)
    return -1;
  switch (op_type) {
  case PINWEAVER_EAL_DECRYPT:
    rv = EVP_DecryptInit(ctx, EVP_aes_128_cfb(), (const unsigned char *)key,
                         (const unsigned char *)iv);
    if (rv != 1)
      break;
    rv = EVP_DecryptUpdate(ctx, (unsigned char *)res, &len,
                           (const unsigned char *)data, size);
    if (rv != 1)
      break;
    rv = EVP_DecryptFinal(ctx, ((unsigned char *)res) + len, &len_final);
    break;
  case PINWEAVER_EAL_ENCRYPT:
    rv = EVP_EncryptInit(ctx, EVP_aes_128_cfb(), (const unsigned char *)key,
                         (const unsigned char *)iv);
    if (rv != 1)
      break;
    rv = EVP_EncryptUpdate(ctx, (unsigned char *)res, &len,
                           (const unsigned char *)data, size);
    if (rv != 1)
      break;
    rv = EVP_EncryptFinal(ctx, ((unsigned char *)res) + len, &len_final);
    break;
  default:
    rv = 0;
  }
  EVP_CIPHER_CTX_free(ctx);
  return rv == 1 ? 0 : -1;
}

int pinweaver_eal_safe_memcmp(const void *s1, const void *s2, size_t len) {
  if (g_data_provider->ConsumeBool()) {
    return g_data_provider->ConsumeIntegralInRange(-1, 1);
  }
  return memcmp(s1, s2, len);
}

int pinweaver_eal_rand_bytes(void *buf, size_t size) {
  if (g_data_provider->ConsumeBool())
    return -1;
  std::string str = g_data_provider->ConsumeBytesAsString(size);
  memcpy(buf, str.data(), str.size());
  return 0;
}

int pinweaver_eal_memcpy_s(
    void * dest,
    size_t destsz,
    const void * src,
    size_t count
) {
	if (count == 0)
		return 0;

	if (dest == NULL) {
		abort();
  }

	if (src == NULL) {
		memset(dest, 0, destsz);
		abort();
	}

	if (destsz < count) {
		memset(dest, 0, destsz);
		abort();
	}

  memcpy(dest, src, count);
  return 0;
}

} // extern "C"
