// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <endian.h>
#include <string.h>

#include "pinweaver_eal.h"
#include "pinweaver_eal_tpm.h"
#include "tss.h"
#include "tss_serde.h"

#ifndef arraysize
#define arraysize(arr) (sizeof(arr)/sizeof(arr[0]))
#endif

#define MAX_AUTH_SECTION_SIZE (3 * sizeof(TPMS_AUTH_COMMAND))
#define SIZE_OFFSET 2

uint8_t g_message[PINWEAVER_TSS_MAX_MESSAGE_SIZE];

#if 0 /* debug helper */

static void dump_buf(const char* prefix, const void* data, size_t size)
{
  printf("%s:\n", prefix);
  const uint8_t* ptr = data;
  for(size_t n = 0; n < size; ++ptr) {
    printf("%02x ", *ptr);
    if ((++n % 16) == 0)
      printf("\n");
  }
  printf("\n");
}

#endif /* debug helper */

static TPM_RC dst_data_buf_append(TSS_DST_DATA_BUF* data_buf, const void* data,
                                  size_t size) {
  if (data_buf->size >= data_buf->max_size)
    return TPM_RC_INSUFFICIENT;
  if (data_buf->size + size > data_buf->max_size
      || size > data_buf->max_size) {
    return TPM_RC_INSUFFICIENT;
  }
  if (pinweaver_eal_memcpy_s(data_buf->buffer + data_buf->size,
      data_buf->max_size - data_buf->size, data, size)) {
    return TPM_RC_INSUFFICIENT;
  }
  data_buf->size += size;
  return TPM_RC_SUCCESS;
}

static TPM_RC dst_data_buf_skip(TSS_DST_DATA_BUF* data_buf,
                                size_t size,
                                TSS_DST_DATA_BUF* reserved) {
  if (data_buf->size + size > data_buf->max_size
      || size > data_buf->max_size) {
    return TPM_RC_INSUFFICIENT;
  }
  if (reserved) {
    reserved->buffer = data_buf->buffer + data_buf->size;
    reserved->max_size = size;
    reserved->size = 0;
  }
  data_buf->size += size;
  return TPM_RC_SUCCESS;
}

static void dst_data_buf_set_to_remainder(TSS_DST_DATA_BUF* to,
                                          TSS_DST_DATA_BUF* from) {
  to->max_size = from->max_size - from->size;
  to->buffer = from->buffer + from->size;
  to->size = 0;
}

static TPM_RC src_data_buf_move_start(TSS_SRC_DATA_BUF* data_buf,
    TSS_DST_DATA_BUF* value_bytes, size_t size) {
  if (value_bytes) {
    TPM_RC result = dst_data_buf_append(value_bytes, data_buf->buffer, size);
    if (result)
      return result;
  }
  if (size > data_buf->size)
    return TPM_RC_INSUFFICIENT;
  data_buf->buffer += size;
  data_buf->size -= size;
  return TPM_RC_SUCCESS;
}

static TPM_RC dst_data_buf_set_at_offset(TSS_DST_DATA_BUF* to,
    TSS_DST_DATA_BUF* from, size_t offset) {
  if (from->size + offset > from->max_size || offset > from->max_size)
    return TPM_RC_INSUFFICIENT;
  to->buffer = from->buffer;
  to->max_size = from->max_size;
  to->size = from->size + offset;
  return TPM_RC_SUCCESS;
}

static TPM_RC dst_data_buf_insert(TSS_DST_DATA_BUF* data_buf,
  TSS_DST_DATA_BUF* new_buf, size_t offset, size_t size) {
  if (data_buf->size + size > data_buf->max_size
      || size > data_buf->max_size
      || offset > data_buf->size) {
    return TPM_RC_INSUFFICIENT;
  }
  memmove(data_buf->buffer + offset + size, data_buf->buffer + offset,
          data_buf->size - offset);
  data_buf->size += size;
  new_buf->buffer = data_buf->buffer + offset;
  new_buf->max_size = size;
  new_buf->size = 0;
  return TPM_RC_SUCCESS;
}

static void SendCommandAndWait(const TSS_DST_DATA_BUF* command,
                               TSS_SRC_DATA_BUF* response) {
  pinweaver_eal_send_command_and_wait((const char *)(command->buffer), (char *)g_message);
  TSS_SRC_DATA_BUF buffer = {sizeof(UINT32), g_message + SIZE_OFFSET};
  UINT32 response_size;
  TPM_RC rc = tss_Parse_UINT32(&buffer, &response_size, NULL);
  response->buffer = g_message;
  if (rc != TPM_RC_SUCCESS || response_size > PINWEAVER_TSS_MAX_MESSAGE_SIZE) {
    response->size = 0;
  } else {
    response->size = response_size;
  }
}

size_t tss_GetNumberOfRequestHandles(TPM_CC command_code) {
  switch (command_code) {
    case TPM_CC_Startup:
      return 0;
    case TPM_CC_Shutdown:
      return 0;
    case TPM_CC_SelfTest:
      return 0;
    case TPM_CC_IncrementalSelfTest:
      return 0;
    case TPM_CC_GetTestResult:
      return 0;
    case TPM_CC_StartAuthSession:
      return 2;
    case TPM_CC_PolicyRestart:
      return 1;
    case TPM_CC_Create:
      return 1;
    case TPM_CC_Load:
      return 1;
    case TPM_CC_LoadExternal:
      return 0;
    case TPM_CC_ReadPublic:
      return 1;
    case TPM_CC_ActivateCredential:
      return 2;
    case TPM_CC_MakeCredential:
      return 1;
    case TPM_CC_Unseal:
      return 1;
    case TPM_CC_ObjectChangeAuth:
      return 2;
    case TPM_CC_Duplicate:
      return 2;
    case TPM_CC_Rewrap:
      return 2;
    case TPM_CC_Import:
      return 1;
    case TPM_CC_RSA_Encrypt:
      return 1;
    case TPM_CC_RSA_Decrypt:
      return 1;
    case TPM_CC_ECDH_KeyGen:
      return 1;
    case TPM_CC_ECDH_ZGen:
      return 1;
    case TPM_CC_ECC_Parameters:
      return 0;
    case TPM_CC_ZGen_2Phase:
      return 1;
    case TPM_CC_EncryptDecrypt:
      return 1;
    case TPM_CC_Hash:
      return 0;
    case TPM_CC_HMAC:
      return 1;
    case TPM_CC_GetRandom:
      return 0;
    case TPM_CC_StirRandom:
      return 0;
    case TPM_CC_HMAC_Start:
      return 1;
    case TPM_CC_HashSequenceStart:
      return 0;
    case TPM_CC_SequenceUpdate:
      return 1;
    case TPM_CC_SequenceComplete:
      return 1;
    case TPM_CC_EventSequenceComplete:
      return 2;
    case TPM_CC_Certify:
      return 2;
    case TPM_CC_CertifyCreation:
      return 2;
    case TPM_CC_Quote:
      return 1;
    case TPM_CC_GetSessionAuditDigest:
      return 3;
    case TPM_CC_GetCommandAuditDigest:
      return 2;
    case TPM_CC_GetTime:
      return 2;
    case TPM_CC_Commit:
      return 1;
    case TPM_CC_EC_Ephemeral:
      return 0;
    case TPM_CC_VerifySignature:
      return 1;
    case TPM_CC_Sign:
      return 1;
    case TPM_CC_SetCommandCodeAuditStatus:
      return 1;
    case TPM_CC_PCR_Extend:
      return 1;
    case TPM_CC_PCR_Event:
      return 1;
    case TPM_CC_PCR_Read:
      return 0;
    case TPM_CC_PCR_Allocate:
      return 1;
    case TPM_CC_PCR_SetAuthPolicy:
      return 2;
    case TPM_CC_PCR_SetAuthValue:
      return 1;
    case TPM_CC_PCR_Reset:
      return 1;
    case TPM_CC_PolicySigned:
      return 2;
    case TPM_CC_PolicySecret:
      return 2;
    case TPM_CC_PolicyTicket:
      return 1;
    case TPM_CC_PolicyOR:
      return 1;
    case TPM_CC_PolicyPCR:
      return 1;
    case TPM_CC_PolicyLocality:
      return 1;
    case TPM_CC_PolicyNV:
      return 3;
    case TPM_CC_PolicyCounterTimer:
      return 1;
    case TPM_CC_PolicyCommandCode:
      return 1;
    case TPM_CC_PolicyPhysicalPresence:
      return 1;
    case TPM_CC_PolicyCpHash:
      return 1;
    case TPM_CC_PolicyNameHash:
      return 1;
    case TPM_CC_PolicyDuplicationSelect:
      return 1;
    case TPM_CC_PolicyAuthorize:
      return 1;
    case TPM_CC_PolicyAuthValue:
      return 1;
    case TPM_CC_PolicyPassword:
      return 1;
    case TPM_CC_PolicyGetDigest:
      return 1;
    case TPM_CC_PolicyNvWritten:
      return 1;
    case TPM_CC_CreatePrimary:
      return 1;
    case TPM_CC_HierarchyControl:
      return 1;
    case TPM_CC_SetPrimaryPolicy:
      return 1;
    case TPM_CC_ChangePPS:
      return 1;
    case TPM_CC_ChangeEPS:
      return 1;
    case TPM_CC_Clear:
      return 1;
    case TPM_CC_ClearControl:
      return 1;
    case TPM_CC_HierarchyChangeAuth:
      return 1;
    case TPM_CC_DictionaryAttackLockReset:
      return 1;
    case TPM_CC_DictionaryAttackParameters:
      return 1;
    case TPM_CC_PP_Commands:
      return 1;
    case TPM_CC_SetAlgorithmSet:
      return 1;
    case TPM_CC_FieldUpgradeStart:
      return 2;
    case TPM_CC_FieldUpgradeData:
      return 0;
    case TPM_CC_FirmwareRead:
      return 0;
    case TPM_CC_ContextSave:
      return 1;
    case TPM_CC_ContextLoad:
      return 0;
    case TPM_CC_FlushContext:
      return 0;
    case TPM_CC_EvictControl:
      return 2;
    case TPM_CC_ReadClock:
      return 0;
    case TPM_CC_ClockSet:
      return 1;
    case TPM_CC_ClockRateAdjust:
      return 1;
    case TPM_CC_GetCapability:
      return 0;
    case TPM_CC_TestParms:
      return 0;
    case TPM_CC_NV_DefineSpace:
      return 1;
    case TPM_CC_NV_UndefineSpace:
      return 2;
    case TPM_CC_NV_UndefineSpaceSpecial:
      return 2;
    case TPM_CC_NV_ReadPublic:
      return 1;
    case TPM_CC_NV_Write:
      return 2;
    case TPM_CC_NV_Increment:
      return 2;
    case TPM_CC_NV_Extend:
      return 2;
    case TPM_CC_NV_SetBits:
      return 2;
    case TPM_CC_NV_WriteLock:
      return 2;
    case TPM_CC_NV_GlobalWriteLock:
      return 1;
    case TPM_CC_NV_Read:
      return 2;
    case TPM_CC_NV_ReadLock:
      return 2;
    case TPM_CC_NV_ChangeAuth:
      return 1;
    case TPM_CC_NV_Certify:
      return 3;
    case TPM_CCE_PolicyFidoSigned:
      return 2;
    default:
      break;
  }
  return 0;
}

size_t tss_GetNumberOfResponseHandles(TPM_CC command_code) {
  switch (command_code) {
    case TPM_CC_Startup:
      return 0;
    case TPM_CC_Shutdown:
      return 0;
    case TPM_CC_SelfTest:
      return 0;
    case TPM_CC_IncrementalSelfTest:
      return 0;
    case TPM_CC_GetTestResult:
      return 0;
    case TPM_CC_StartAuthSession:
      return 1;
    case TPM_CC_PolicyRestart:
      return 0;
    case TPM_CC_Create:
      return 0;
    case TPM_CC_Load:
      return 1;
    case TPM_CC_LoadExternal:
      return 1;
    case TPM_CC_ReadPublic:
      return 0;
    case TPM_CC_ActivateCredential:
      return 0;
    case TPM_CC_MakeCredential:
      return 0;
    case TPM_CC_Unseal:
      return 0;
    case TPM_CC_ObjectChangeAuth:
      return 0;
    case TPM_CC_Duplicate:
      return 0;
    case TPM_CC_Rewrap:
      return 0;
    case TPM_CC_Import:
      return 0;
    case TPM_CC_RSA_Encrypt:
      return 0;
    case TPM_CC_RSA_Decrypt:
      return 0;
    case TPM_CC_ECDH_KeyGen:
      return 0;
    case TPM_CC_ECDH_ZGen:
      return 0;
    case TPM_CC_ECC_Parameters:
      return 0;
    case TPM_CC_ZGen_2Phase:
      return 0;
    case TPM_CC_EncryptDecrypt:
      return 0;
    case TPM_CC_Hash:
      return 0;
    case TPM_CC_HMAC:
      return 0;
    case TPM_CC_GetRandom:
      return 0;
    case TPM_CC_StirRandom:
      return 0;
    case TPM_CC_HMAC_Start:
      return 1;
    case TPM_CC_HashSequenceStart:
      return 1;
    case TPM_CC_SequenceUpdate:
      return 0;
    case TPM_CC_SequenceComplete:
      return 0;
    case TPM_CC_EventSequenceComplete:
      return 0;
    case TPM_CC_Certify:
      return 0;
    case TPM_CC_CertifyCreation:
      return 0;
    case TPM_CC_Quote:
      return 0;
    case TPM_CC_GetSessionAuditDigest:
      return 0;
    case TPM_CC_GetCommandAuditDigest:
      return 0;
    case TPM_CC_GetTime:
      return 0;
    case TPM_CC_Commit:
      return 0;
    case TPM_CC_EC_Ephemeral:
      return 0;
    case TPM_CC_VerifySignature:
      return 0;
    case TPM_CC_Sign:
      return 0;
    case TPM_CC_SetCommandCodeAuditStatus:
      return 0;
    case TPM_CC_PCR_Extend:
      return 0;
    case TPM_CC_PCR_Event:
      return 0;
    case TPM_CC_PCR_Read:
      return 0;
    case TPM_CC_PCR_Allocate:
      return 0;
    case TPM_CC_PCR_SetAuthPolicy:
      return 0;
    case TPM_CC_PCR_SetAuthValue:
      return 0;
    case TPM_CC_PCR_Reset:
      return 0;
    case TPM_CC_PolicySigned:
      return 0;
    case TPM_CC_PolicySecret:
      return 0;
    case TPM_CC_PolicyTicket:
      return 0;
    case TPM_CC_PolicyOR:
      return 0;
    case TPM_CC_PolicyPCR:
      return 0;
    case TPM_CC_PolicyLocality:
      return 0;
    case TPM_CC_PolicyNV:
      return 0;
    case TPM_CC_PolicyCounterTimer:
      return 0;
    case TPM_CC_PolicyCommandCode:
      return 0;
    case TPM_CC_PolicyPhysicalPresence:
      return 0;
    case TPM_CC_PolicyCpHash:
      return 0;
    case TPM_CC_PolicyNameHash:
      return 0;
    case TPM_CC_PolicyDuplicationSelect:
      return 0;
    case TPM_CC_PolicyAuthorize:
      return 0;
    case TPM_CC_PolicyAuthValue:
      return 0;
    case TPM_CC_PolicyPassword:
      return 0;
    case TPM_CC_PolicyGetDigest:
      return 0;
    case TPM_CC_PolicyNvWritten:
      return 0;
    case TPM_CC_CreatePrimary:
      return 1;
    case TPM_CC_HierarchyControl:
      return 0;
    case TPM_CC_SetPrimaryPolicy:
      return 0;
    case TPM_CC_ChangePPS:
      return 0;
    case TPM_CC_ChangeEPS:
      return 0;
    case TPM_CC_Clear:
      return 0;
    case TPM_CC_ClearControl:
      return 0;
    case TPM_CC_HierarchyChangeAuth:
      return 0;
    case TPM_CC_DictionaryAttackLockReset:
      return 0;
    case TPM_CC_DictionaryAttackParameters:
      return 0;
    case TPM_CC_PP_Commands:
      return 0;
    case TPM_CC_SetAlgorithmSet:
      return 0;
    case TPM_CC_FieldUpgradeStart:
      return 0;
    case TPM_CC_FieldUpgradeData:
      return 0;
    case TPM_CC_FirmwareRead:
      return 0;
    case TPM_CC_ContextSave:
      return 0;
    case TPM_CC_ContextLoad:
      return 1;
    case TPM_CC_FlushContext:
      return 0;
    case TPM_CC_EvictControl:
      return 0;
    case TPM_CC_ReadClock:
      return 0;
    case TPM_CC_ClockSet:
      return 0;
    case TPM_CC_ClockRateAdjust:
      return 0;
    case TPM_CC_GetCapability:
      return 0;
    case TPM_CC_TestParms:
      return 0;
    case TPM_CC_NV_DefineSpace:
      return 0;
    case TPM_CC_NV_UndefineSpace:
      return 0;
    case TPM_CC_NV_UndefineSpaceSpecial:
      return 0;
    case TPM_CC_NV_ReadPublic:
      return 0;
    case TPM_CC_NV_Write:
      return 0;
    case TPM_CC_NV_Increment:
      return 0;
    case TPM_CC_NV_Extend:
      return 0;
    case TPM_CC_NV_SetBits:
      return 0;
    case TPM_CC_NV_WriteLock:
      return 0;
    case TPM_CC_NV_GlobalWriteLock:
      return 0;
    case TPM_CC_NV_Read:
      return 0;
    case TPM_CC_NV_ReadLock:
      return 0;
    case TPM_CC_NV_ChangeAuth:
      return 0;
    case TPM_CC_NV_Certify:
      return 0;
    case TPM_CCE_PolicyFidoSigned:
      return 0;
    default:
      break;
  }
  return 0;
}

TPM_RC tss_Serialize_uint8_t(const uint8_t* value, TSS_DST_DATA_BUF* buffer) {
  uint8_t value_net = *value;
  switch (sizeof(uint8_t)) {
    case 2:
      value_net = htobe16(*value);
      break;
    case 4:
      value_net = htobe32(*value);
      break;
    case 8:
      value_net = htobe64(*value);
      break;
    default:
      break;
  }
  return dst_data_buf_append(buffer, &value_net, sizeof(uint8_t));
}

TPM_RC tss_Parse_uint8_t(TSS_SRC_DATA_BUF* buffer,
                     uint8_t* value,
                     TSS_DST_DATA_BUF* value_bytes) {
  if (buffer->size < sizeof(uint8_t))
    return TPM_RC_INSUFFICIENT;
  uint8_t value_net = 0;
  if (pinweaver_eal_memcpy_s(&value_net, sizeof(value_net), buffer->buffer,
      sizeof(uint8_t))) {
    return TPM_RC_INSUFFICIENT;
  }
  switch (sizeof(uint8_t)) {
    case 2:
      *value = be16toh(value_net);
      break;
    case 4:
      *value = be32toh(value_net);
      break;
    case 8:
      *value = be64toh(value_net);
      break;
    default:
      *value = value_net;
  }
  return src_data_buf_move_start(buffer, value_bytes, sizeof(uint8_t));
}

TPM_RC tss_Serialize_int8_t(const int8_t* value, TSS_DST_DATA_BUF* buffer) {
  int8_t value_net = *value;
  switch (sizeof(int8_t)) {
    case 2:
      value_net = htobe16(*value);
      break;
    case 4:
      value_net = htobe32(*value);
      break;
    case 8:
      value_net = htobe64(*value);
      break;
    default:
      break;
  }
  return dst_data_buf_append(buffer, &value_net, sizeof(int8_t));
}

TPM_RC tss_Parse_int8_t(TSS_SRC_DATA_BUF* buffer,
                    int8_t* value,
                    TSS_DST_DATA_BUF* value_bytes) {
  if (buffer->size < sizeof(int8_t))
    return TPM_RC_INSUFFICIENT;
  int8_t value_net = 0;
  if (pinweaver_eal_memcpy_s(&value_net, sizeof(value_net), buffer->buffer,
      sizeof(int8_t))) {
    return TPM_RC_INSUFFICIENT;
  }
  switch (sizeof(int8_t)) {
    case 2:
      *value = be16toh(value_net);
      break;
    case 4:
      *value = be32toh(value_net);
      break;
    case 8:
      *value = be64toh(value_net);
      break;
    default:
      *value = value_net;
  }
  return src_data_buf_move_start(buffer, value_bytes, sizeof(int8_t));
}

TPM_RC tss_Serialize_int(const int* value, TSS_DST_DATA_BUF* buffer) {
  int value_net = *value;
  switch (sizeof(int)) {
    case 2:
      value_net = htobe16(*value);
      break;
    case 4:
      value_net = htobe32(*value);
      break;
    case 8:
      value_net = htobe64(*value);
      break;
    default:
      break;
  }
  return dst_data_buf_append(buffer, &value_net, sizeof(int));
}

TPM_RC tss_Parse_int(TSS_SRC_DATA_BUF* buffer, int* value, TSS_DST_DATA_BUF* value_bytes) {
  if (buffer->size < sizeof(int))
    return TPM_RC_INSUFFICIENT;
  int value_net = 0;
  if (pinweaver_eal_memcpy_s(&value_net, sizeof(value_net), buffer->buffer,
      sizeof(int))) {
    return TPM_RC_INSUFFICIENT;
  }
  switch (sizeof(int)) {
    case 2:
      *value = be16toh(value_net);
      break;
    case 4:
      *value = be32toh(value_net);
      break;
    case 8:
      *value = be64toh(value_net);
      break;
    default:
      *value = value_net;
  }
  return src_data_buf_move_start(buffer, value_bytes, sizeof(int));
}

TPM_RC tss_Serialize_uint16_t(const uint16_t* value, TSS_DST_DATA_BUF* buffer) {
  uint16_t value_net = *value;
  switch (sizeof(uint16_t)) {
    case 2:
      value_net = htobe16(*value);
      break;
    case 4:
      value_net = htobe32(*value);
      break;
    case 8:
      value_net = htobe64(*value);
      break;
    default:
      break;
  }
  return dst_data_buf_append(buffer, &value_net, sizeof(uint16_t));
}

TPM_RC tss_Parse_uint16_t(TSS_SRC_DATA_BUF* buffer,
                      uint16_t* value,
                      TSS_DST_DATA_BUF* value_bytes) {
  if (buffer->size < sizeof(uint16_t))
    return TPM_RC_INSUFFICIENT;
  uint16_t value_net = 0;
  if (pinweaver_eal_memcpy_s(&value_net, sizeof(value_net), buffer->buffer,
      sizeof(uint16_t))) {
    return TPM_RC_INSUFFICIENT;
  }
  switch (sizeof(uint16_t)) {
    case 2:
      *value = be16toh(value_net);
      break;
    case 4:
      *value = be32toh(value_net);
      break;
    case 8:
      *value = be64toh(value_net);
      break;
    default:
      *value = value_net;
  }
  return src_data_buf_move_start(buffer, value_bytes, sizeof(uint16_t));
}

TPM_RC tss_Serialize_int16_t(const int16_t* value, TSS_DST_DATA_BUF* buffer) {
  int16_t value_net = *value;
  switch (sizeof(int16_t)) {
    case 2:
      value_net = htobe16(*value);
      break;
    case 4:
      value_net = htobe32(*value);
      break;
    case 8:
      value_net = htobe64(*value);
      break;
    default:
      break;
  }
  return dst_data_buf_append(buffer, &value_net, sizeof(int16_t));
}

TPM_RC tss_Parse_int16_t(TSS_SRC_DATA_BUF* buffer,
                     int16_t* value,
                     TSS_DST_DATA_BUF* value_bytes) {
  if (buffer->size < sizeof(int16_t))
    return TPM_RC_INSUFFICIENT;
  int16_t value_net = 0;
  if (pinweaver_eal_memcpy_s(&value_net, sizeof(value_net), buffer->buffer,
      sizeof(int16_t))) {
    return TPM_RC_INSUFFICIENT;
  }
  switch (sizeof(int16_t)) {
    case 2:
      *value = be16toh(value_net);
      break;
    case 4:
      *value = be32toh(value_net);
      break;
    case 8:
      *value = be64toh(value_net);
      break;
    default:
      *value = value_net;
  }
  return src_data_buf_move_start(buffer, value_bytes, sizeof(int16_t));
}

TPM_RC tss_Serialize_uint32_t(const uint32_t* value, TSS_DST_DATA_BUF* buffer) {
  uint32_t value_net = *value;
  switch (sizeof(uint32_t)) {
    case 2:
      value_net = htobe16(*value);
      break;
    case 4:
      value_net = htobe32(*value);
      break;
    case 8:
      value_net = htobe64(*value);
      break;
    default:
      break;
  }
  return dst_data_buf_append(buffer, &value_net, sizeof(uint32_t));
}

TPM_RC tss_Parse_uint32_t(TSS_SRC_DATA_BUF* buffer,
                      uint32_t* value,
                      TSS_DST_DATA_BUF* value_bytes) {
  if (buffer->size < sizeof(uint32_t))
    return TPM_RC_INSUFFICIENT;
  uint32_t value_net = 0;
  if (pinweaver_eal_memcpy_s(&value_net, sizeof(value_net), buffer->buffer,
      sizeof(uint32_t))) {
    return TPM_RC_INSUFFICIENT;
  }
  switch (sizeof(uint32_t)) {
    case 2:
      *value = be16toh(value_net);
      break;
    case 4:
      *value = be32toh(value_net);
      break;
    case 8:
      *value = be64toh(value_net);
      break;
    default:
      *value = value_net;
  }
  return src_data_buf_move_start(buffer, value_bytes, sizeof(uint32_t));
}

TPM_RC tss_Serialize_int32_t(const int32_t* value, TSS_DST_DATA_BUF* buffer) {
  int32_t value_net = *value;
  switch (sizeof(int32_t)) {
    case 2:
      value_net = htobe16(*value);
      break;
    case 4:
      value_net = htobe32(*value);
      break;
    case 8:
      value_net = htobe64(*value);
      break;
    default:
      break;
  }
  return dst_data_buf_append(buffer, &value_net, sizeof(int32_t));
}

TPM_RC tss_Parse_int32_t(TSS_SRC_DATA_BUF* buffer,
                     int32_t* value,
                     TSS_DST_DATA_BUF* value_bytes) {
  if (buffer->size < sizeof(int32_t))
    return TPM_RC_INSUFFICIENT;
  int32_t value_net = 0;
  if (pinweaver_eal_memcpy_s(&value_net, sizeof(value_net), buffer->buffer,
      sizeof(int32_t))) {
    return TPM_RC_INSUFFICIENT;
  }
  switch (sizeof(int32_t)) {
    case 2:
      *value = be16toh(value_net);
      break;
    case 4:
      *value = be32toh(value_net);
      break;
    case 8:
      *value = be64toh(value_net);
      break;
    default:
      *value = value_net;
  }
  return src_data_buf_move_start(buffer, value_bytes, sizeof(int32_t));
}

TPM_RC tss_Serialize_uint64_t(const uint64_t* value, TSS_DST_DATA_BUF* buffer) {
  uint64_t value_net = *value;
  switch (sizeof(uint64_t)) {
    case 2:
      value_net = htobe16(*value);
      break;
    case 4:
      value_net = htobe32(*value);
      break;
    case 8:
      value_net = htobe64(*value);
      break;
    default:
      break;
  }
  return dst_data_buf_append(buffer, &value_net, sizeof(uint64_t));
}

TPM_RC tss_Parse_uint64_t(TSS_SRC_DATA_BUF* buffer,
                      uint64_t* value,
                      TSS_DST_DATA_BUF* value_bytes) {
  if (buffer->size < sizeof(uint64_t))
    return TPM_RC_INSUFFICIENT;
  uint64_t value_net = 0;
  if (pinweaver_eal_memcpy_s(&value_net, sizeof(value_net), buffer->buffer,
      sizeof(uint64_t))) {
    return TPM_RC_INSUFFICIENT;
  }
  switch (sizeof(uint64_t)) {
    case 2:
      *value = be16toh(value_net);
      break;
    case 4:
      *value = be32toh(value_net);
      break;
    case 8:
      *value = be64toh(value_net);
      break;
    default:
      *value = value_net;
  }
  return src_data_buf_move_start(buffer, value_bytes, sizeof(uint64_t));
}

TPM_RC tss_Serialize_int64_t(const int64_t* value, TSS_DST_DATA_BUF* buffer) {
  int64_t value_net = *value;
  switch (sizeof(int64_t)) {
    case 2:
      value_net = htobe16(*value);
      break;
    case 4:
      value_net = htobe32(*value);
      break;
    case 8:
      value_net = htobe64(*value);
      break;
    default:
      break;
  }
  return dst_data_buf_append(buffer, &value_net, sizeof(int64_t));
}

TPM_RC tss_Parse_int64_t(TSS_SRC_DATA_BUF* buffer,
                     int64_t* value,
                     TSS_DST_DATA_BUF* value_bytes) {
  if (buffer->size < sizeof(int64_t))
    return TPM_RC_INSUFFICIENT;
  int64_t value_net = 0;
  if (pinweaver_eal_memcpy_s(&value_net, sizeof(value_net), buffer->buffer,
      sizeof(int64_t))) {
    return TPM_RC_INSUFFICIENT;
  }
  switch (sizeof(int64_t)) {
    case 2:
      *value = be16toh(value_net);
      break;
    case 4:
      *value = be32toh(value_net);
      break;
    case 8:
      *value = be64toh(value_net);
      break;
    default:
      *value = value_net;
  }
  return src_data_buf_move_start(buffer, value_bytes, sizeof(int64_t));
}

TPM_RC tss_Serialize_UINT8(const UINT8* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_uint8_t(value, buffer);
}

TPM_RC tss_Parse_UINT8(TSS_SRC_DATA_BUF* buffer,
                   UINT8* value,
                   TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_uint8_t(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_BYTE(const BYTE* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_uint8_t(value, buffer);
}

TPM_RC tss_Parse_BYTE(TSS_SRC_DATA_BUF* buffer, BYTE* value, TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_uint8_t(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_INT8(const INT8* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_int8_t(value, buffer);
}

TPM_RC tss_Parse_INT8(TSS_SRC_DATA_BUF* buffer, INT8* value, TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_int8_t(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_BOOL(const BOOL* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_int(value, buffer);
}

TPM_RC tss_Parse_BOOL(TSS_SRC_DATA_BUF* buffer, BOOL* value, TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_int(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_UINT16(const UINT16* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_uint16_t(value, buffer);
}

TPM_RC tss_Parse_UINT16(TSS_SRC_DATA_BUF* buffer,
                    UINT16* value,
                    TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_uint16_t(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_INT16(const INT16* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_int16_t(value, buffer);
}

TPM_RC tss_Parse_INT16(TSS_SRC_DATA_BUF* buffer,
                   INT16* value,
                   TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_int16_t(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_UINT32(const UINT32* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_uint32_t(value, buffer);
}

TPM_RC tss_Parse_UINT32(TSS_SRC_DATA_BUF* buffer,
                    UINT32* value,
                    TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_uint32_t(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_INT32(const INT32* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_int32_t(value, buffer);
}

TPM_RC tss_Parse_INT32(TSS_SRC_DATA_BUF* buffer,
                   INT32* value,
                   TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_int32_t(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_UINT64(const UINT64* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_uint64_t(value, buffer);
}

TPM_RC tss_Parse_UINT64(TSS_SRC_DATA_BUF* buffer,
                    UINT64* value,
                    TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_uint64_t(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_INT64(const INT64* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_int64_t(value, buffer);
}

TPM_RC tss_Parse_INT64(TSS_SRC_DATA_BUF* buffer,
                   INT64* value,
                   TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_int64_t(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_ALGORITHM_ID(const TPM_ALGORITHM_ID* value,
                                  TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPM_ALGORITHM_ID(TSS_SRC_DATA_BUF* buffer,
                              TPM_ALGORITHM_ID* value,
                              TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_MODIFIER_INDICATOR(const TPM_MODIFIER_INDICATOR* value,
                                        TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPM_MODIFIER_INDICATOR(TSS_SRC_DATA_BUF* buffer,
                                    TPM_MODIFIER_INDICATOR* value,
                                    TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_AUTHORIZATION_SIZE(const TPM_AUTHORIZATION_SIZE* value,
                                        TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPM_AUTHORIZATION_SIZE(TSS_SRC_DATA_BUF* buffer,
                                    TPM_AUTHORIZATION_SIZE* value,
                                    TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_PARAMETER_SIZE(const TPM_PARAMETER_SIZE* value,
                                    TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPM_PARAMETER_SIZE(TSS_SRC_DATA_BUF* buffer,
                                TPM_PARAMETER_SIZE* value,
                                TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_KEY_SIZE(const TPM_KEY_SIZE* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT16(value, buffer);
}

TPM_RC tss_Parse_TPM_KEY_SIZE(TSS_SRC_DATA_BUF* buffer,
                          TPM_KEY_SIZE* value,
                          TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT16(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_KEY_BITS(const TPM_KEY_BITS* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT16(value, buffer);
}

TPM_RC tss_Parse_TPM_KEY_BITS(TSS_SRC_DATA_BUF* buffer,
                          TPM_KEY_BITS* value,
                          TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT16(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_HANDLE(const TPM_HANDLE* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPM_HANDLE(TSS_SRC_DATA_BUF* buffer,
                        TPM_HANDLE* value,
                        TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM2B_DIGEST(const TPM2B_DIGEST* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->buffer[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_DIGEST(TSS_SRC_DATA_BUF* buffer,
                          TPM2B_DIGEST* value,
                          TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->buffer[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_NONCE(const TPM2B_NONCE* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM2B_DIGEST(value, buffer);
}

TPM_RC tss_Parse_TPM2B_NONCE(TSS_SRC_DATA_BUF* buffer,
                         TPM2B_NONCE* value,
                         TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM2B_DIGEST(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM2B_AUTH(const TPM2B_AUTH* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM2B_DIGEST(value, buffer);
}

TPM_RC tss_Parse_TPM2B_AUTH(TSS_SRC_DATA_BUF* buffer,
                        TPM2B_AUTH* value,
                        TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM2B_DIGEST(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM2B_OPERAND(const TPM2B_OPERAND* value,
                               TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM2B_DIGEST(value, buffer);
}

TPM_RC tss_Parse_TPM2B_OPERAND(TSS_SRC_DATA_BUF* buffer,
                           TPM2B_OPERAND* value,
                           TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM2B_DIGEST(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_ALG_ID(const TPM_ALG_ID* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT16(value, buffer);
}

TPM_RC tss_Parse_TPM_ALG_ID(TSS_SRC_DATA_BUF* buffer,
                        TPM_ALG_ID* value,
                        TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT16(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_ALG_HASH(const TPMI_ALG_HASH* value,
                               TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_ALG_ID(value, buffer);
}

TPM_RC tss_Parse_TPMI_ALG_HASH(TSS_SRC_DATA_BUF* buffer,
                           TPMI_ALG_HASH* value,
                           TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_ALG_ID(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMS_SCHEME_SIGHASH(const TPMS_SCHEME_SIGHASH* value,
                                     TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_HASH(&value->hash_alg, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_SCHEME_SIGHASH(TSS_SRC_DATA_BUF* buffer,
                                 TPMS_SCHEME_SIGHASH* value,
                                 TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_HASH(buffer, &value->hash_alg, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_SCHEME_HMAC(const TPMS_SCHEME_HMAC* value,
                                  TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPMS_SCHEME_SIGHASH(value, buffer);
}

TPM_RC tss_Parse_TPMS_SCHEME_HMAC(TSS_SRC_DATA_BUF* buffer,
                              TPMS_SCHEME_HMAC* value,
                              TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPMS_SCHEME_SIGHASH(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMS_SCHEME_RSASSA(const TPMS_SCHEME_RSASSA* value,
                                    TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPMS_SCHEME_SIGHASH(value, buffer);
}

TPM_RC tss_Parse_TPMS_SCHEME_RSASSA(TSS_SRC_DATA_BUF* buffer,
                                TPMS_SCHEME_RSASSA* value,
                                TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPMS_SCHEME_SIGHASH(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMS_SCHEME_RSAPSS(const TPMS_SCHEME_RSAPSS* value,
                                    TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPMS_SCHEME_SIGHASH(value, buffer);
}

TPM_RC tss_Parse_TPMS_SCHEME_RSAPSS(TSS_SRC_DATA_BUF* buffer,
                                TPMS_SCHEME_RSAPSS* value,
                                TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPMS_SCHEME_SIGHASH(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMS_SCHEME_ECDSA(const TPMS_SCHEME_ECDSA* value,
                                   TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPMS_SCHEME_SIGHASH(value, buffer);
}

TPM_RC tss_Parse_TPMS_SCHEME_ECDSA(TSS_SRC_DATA_BUF* buffer,
                               TPMS_SCHEME_ECDSA* value,
                               TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPMS_SCHEME_SIGHASH(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMS_SCHEME_SM2(const TPMS_SCHEME_SM2* value,
                                 TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPMS_SCHEME_SIGHASH(value, buffer);
}

TPM_RC tss_Parse_TPMS_SCHEME_SM2(TSS_SRC_DATA_BUF* buffer,
                             TPMS_SCHEME_SM2* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPMS_SCHEME_SIGHASH(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMS_SCHEME_ECSCHNORR(const TPMS_SCHEME_ECSCHNORR* value,
                                       TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPMS_SCHEME_SIGHASH(value, buffer);
}

TPM_RC tss_Parse_TPMS_SCHEME_ECSCHNORR(TSS_SRC_DATA_BUF* buffer,
                                   TPMS_SCHEME_ECSCHNORR* value,
                                   TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPMS_SCHEME_SIGHASH(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_YES_NO(const TPMI_YES_NO* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_BYTE(value, buffer);
}

TPM_RC tss_Parse_TPMI_YES_NO(TSS_SRC_DATA_BUF* buffer,
                         TPMI_YES_NO* value,
                         TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_BYTE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_DH_OBJECT(const TPMI_DH_OBJECT* value,
                                TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_HANDLE(value, buffer);
}

TPM_RC tss_Parse_TPMI_DH_OBJECT(TSS_SRC_DATA_BUF* buffer,
                            TPMI_DH_OBJECT* value,
                            TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_HANDLE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_DH_PERSISTENT(const TPMI_DH_PERSISTENT* value,
                                    TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_HANDLE(value, buffer);
}

TPM_RC tss_Parse_TPMI_DH_PERSISTENT(TSS_SRC_DATA_BUF* buffer,
                                TPMI_DH_PERSISTENT* value,
                                TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_HANDLE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_DH_ENTITY(const TPMI_DH_ENTITY* value,
                                TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_HANDLE(value, buffer);
}

TPM_RC tss_Parse_TPMI_DH_ENTITY(TSS_SRC_DATA_BUF* buffer,
                            TPMI_DH_ENTITY* value,
                            TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_HANDLE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_DH_PCR(const TPMI_DH_PCR* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_HANDLE(value, buffer);
}

TPM_RC tss_Parse_TPMI_DH_PCR(TSS_SRC_DATA_BUF* buffer,
                         TPMI_DH_PCR* value,
                         TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_HANDLE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_SH_AUTH_SESSION(const TPMI_SH_AUTH_SESSION* value,
                                      TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_HANDLE(value, buffer);
}

TPM_RC tss_Parse_TPMI_SH_AUTH_SESSION(TSS_SRC_DATA_BUF* buffer,
                                  TPMI_SH_AUTH_SESSION* value,
                                  TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_HANDLE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_SH_HMAC(const TPMI_SH_HMAC* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_HANDLE(value, buffer);
}

TPM_RC tss_Parse_TPMI_SH_HMAC(TSS_SRC_DATA_BUF* buffer,
                          TPMI_SH_HMAC* value,
                          TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_HANDLE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_SH_POLICY(const TPMI_SH_POLICY* value,
                                TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_HANDLE(value, buffer);
}

TPM_RC tss_Parse_TPMI_SH_POLICY(TSS_SRC_DATA_BUF* buffer,
                            TPMI_SH_POLICY* value,
                            TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_HANDLE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_DH_CONTEXT(const TPMI_DH_CONTEXT* value,
                                 TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_HANDLE(value, buffer);
}

TPM_RC tss_Parse_TPMI_DH_CONTEXT(TSS_SRC_DATA_BUF* buffer,
                             TPMI_DH_CONTEXT* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_HANDLE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_RH_HIERARCHY(const TPMI_RH_HIERARCHY* value,
                                   TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_HANDLE(value, buffer);
}

TPM_RC tss_Parse_TPMI_RH_HIERARCHY(TSS_SRC_DATA_BUF* buffer,
                               TPMI_RH_HIERARCHY* value,
                               TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_HANDLE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_RH_ENABLES(const TPMI_RH_ENABLES* value,
                                 TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_HANDLE(value, buffer);
}

TPM_RC tss_Parse_TPMI_RH_ENABLES(TSS_SRC_DATA_BUF* buffer,
                             TPMI_RH_ENABLES* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_HANDLE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_RH_HIERARCHY_AUTH(const TPMI_RH_HIERARCHY_AUTH* value,
                                        TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_HANDLE(value, buffer);
}

TPM_RC tss_Parse_TPMI_RH_HIERARCHY_AUTH(TSS_SRC_DATA_BUF* buffer,
                                    TPMI_RH_HIERARCHY_AUTH* value,
                                    TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_HANDLE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_RH_PLATFORM(const TPMI_RH_PLATFORM* value,
                                  TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_HANDLE(value, buffer);
}

TPM_RC tss_Parse_TPMI_RH_PLATFORM(TSS_SRC_DATA_BUF* buffer,
                              TPMI_RH_PLATFORM* value,
                              TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_HANDLE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_RH_OWNER(const TPMI_RH_OWNER* value,
                               TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_HANDLE(value, buffer);
}

TPM_RC tss_Parse_TPMI_RH_OWNER(TSS_SRC_DATA_BUF* buffer,
                           TPMI_RH_OWNER* value,
                           TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_HANDLE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_RH_ENDORSEMENT(const TPMI_RH_ENDORSEMENT* value,
                                     TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_HANDLE(value, buffer);
}

TPM_RC tss_Parse_TPMI_RH_ENDORSEMENT(TSS_SRC_DATA_BUF* buffer,
                                 TPMI_RH_ENDORSEMENT* value,
                                 TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_HANDLE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_RH_PROVISION(const TPMI_RH_PROVISION* value,
                                   TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_HANDLE(value, buffer);
}

TPM_RC tss_Parse_TPMI_RH_PROVISION(TSS_SRC_DATA_BUF* buffer,
                               TPMI_RH_PROVISION* value,
                               TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_HANDLE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_RH_CLEAR(const TPMI_RH_CLEAR* value,
                               TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_HANDLE(value, buffer);
}

TPM_RC tss_Parse_TPMI_RH_CLEAR(TSS_SRC_DATA_BUF* buffer,
                           TPMI_RH_CLEAR* value,
                           TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_HANDLE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_RH_NV_AUTH(const TPMI_RH_NV_AUTH* value,
                                 TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_HANDLE(value, buffer);
}

TPM_RC tss_Parse_TPMI_RH_NV_AUTH(TSS_SRC_DATA_BUF* buffer,
                             TPMI_RH_NV_AUTH* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_HANDLE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_RH_LOCKOUT(const TPMI_RH_LOCKOUT* value,
                                 TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_HANDLE(value, buffer);
}

TPM_RC tss_Parse_TPMI_RH_LOCKOUT(TSS_SRC_DATA_BUF* buffer,
                             TPMI_RH_LOCKOUT* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_HANDLE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_RH_NV_INDEX(const TPMI_RH_NV_INDEX* value,
                                  TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_HANDLE(value, buffer);
}

TPM_RC tss_Parse_TPMI_RH_NV_INDEX(TSS_SRC_DATA_BUF* buffer,
                              TPMI_RH_NV_INDEX* value,
                              TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_HANDLE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_ALG_ASYM(const TPMI_ALG_ASYM* value,
                               TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_ALG_ID(value, buffer);
}

TPM_RC tss_Parse_TPMI_ALG_ASYM(TSS_SRC_DATA_BUF* buffer,
                           TPMI_ALG_ASYM* value,
                           TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_ALG_ID(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_ALG_SYM(const TPMI_ALG_SYM* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_ALG_ID(value, buffer);
}

TPM_RC tss_Parse_TPMI_ALG_SYM(TSS_SRC_DATA_BUF* buffer,
                          TPMI_ALG_SYM* value,
                          TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_ALG_ID(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_ALG_SYM_OBJECT(const TPMI_ALG_SYM_OBJECT* value,
                                     TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_ALG_ID(value, buffer);
}

TPM_RC tss_Parse_TPMI_ALG_SYM_OBJECT(TSS_SRC_DATA_BUF* buffer,
                                 TPMI_ALG_SYM_OBJECT* value,
                                 TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_ALG_ID(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_ALG_SYM_MODE(const TPMI_ALG_SYM_MODE* value,
                                   TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_ALG_ID(value, buffer);
}

TPM_RC tss_Parse_TPMI_ALG_SYM_MODE(TSS_SRC_DATA_BUF* buffer,
                               TPMI_ALG_SYM_MODE* value,
                               TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_ALG_ID(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_ALG_KDF(const TPMI_ALG_KDF* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_ALG_ID(value, buffer);
}

TPM_RC tss_Parse_TPMI_ALG_KDF(TSS_SRC_DATA_BUF* buffer,
                          TPMI_ALG_KDF* value,
                          TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_ALG_ID(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_ALG_SIG_SCHEME(const TPMI_ALG_SIG_SCHEME* value,
                                     TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_ALG_ID(value, buffer);
}

TPM_RC tss_Parse_TPMI_ALG_SIG_SCHEME(TSS_SRC_DATA_BUF* buffer,
                                 TPMI_ALG_SIG_SCHEME* value,
                                 TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_ALG_ID(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_ECC_KEY_EXCHANGE(const TPMI_ECC_KEY_EXCHANGE* value,
                                       TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_ALG_ID(value, buffer);
}

TPM_RC tss_Parse_TPMI_ECC_KEY_EXCHANGE(TSS_SRC_DATA_BUF* buffer,
                                   TPMI_ECC_KEY_EXCHANGE* value,
                                   TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_ALG_ID(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_ST(const TPM_ST* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT16(value, buffer);
}

TPM_RC tss_Parse_TPM_ST(TSS_SRC_DATA_BUF* buffer,
                    TPM_ST* value,
                    TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT16(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_ST_COMMAND_TAG(const TPMI_ST_COMMAND_TAG* value,
                                     TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_ST(value, buffer);
}

TPM_RC tss_Parse_TPMI_ST_COMMAND_TAG(TSS_SRC_DATA_BUF* buffer,
                                 TPMI_ST_COMMAND_TAG* value,
                                 TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_ST(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_ST_ATTEST(const TPMI_ST_ATTEST* value,
                                TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_ST(value, buffer);
}

TPM_RC tss_Parse_TPMI_ST_ATTEST(TSS_SRC_DATA_BUF* buffer,
                            TPMI_ST_ATTEST* value,
                            TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_ST(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_AES_KEY_BITS(const TPMI_AES_KEY_BITS* value,
                                   TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_KEY_BITS(value, buffer);
}

TPM_RC tss_Parse_TPMI_AES_KEY_BITS(TSS_SRC_DATA_BUF* buffer,
                               TPMI_AES_KEY_BITS* value,
                               TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_KEY_BITS(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_SM4_KEY_BITS(const TPMI_SM4_KEY_BITS* value,
                                   TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_KEY_BITS(value, buffer);
}

TPM_RC tss_Parse_TPMI_SM4_KEY_BITS(TSS_SRC_DATA_BUF* buffer,
                               TPMI_SM4_KEY_BITS* value,
                               TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_KEY_BITS(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_ALG_KEYEDHASH_SCHEME(
    const TPMI_ALG_KEYEDHASH_SCHEME* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_ALG_ID(value, buffer);
}

TPM_RC tss_Parse_TPMI_ALG_KEYEDHASH_SCHEME(TSS_SRC_DATA_BUF* buffer,
                                       TPMI_ALG_KEYEDHASH_SCHEME* value,
                                       TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_ALG_ID(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_ALG_ASYM_SCHEME(const TPMI_ALG_ASYM_SCHEME* value,
                                      TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_ALG_ID(value, buffer);
}

TPM_RC tss_Parse_TPMI_ALG_ASYM_SCHEME(TSS_SRC_DATA_BUF* buffer,
                                  TPMI_ALG_ASYM_SCHEME* value,
                                  TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_ALG_ID(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_ALG_RSA_SCHEME(const TPMI_ALG_RSA_SCHEME* value,
                                     TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_ALG_ID(value, buffer);
}

TPM_RC tss_Parse_TPMI_ALG_RSA_SCHEME(TSS_SRC_DATA_BUF* buffer,
                                 TPMI_ALG_RSA_SCHEME* value,
                                 TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_ALG_ID(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_ALG_RSA_DECRYPT(const TPMI_ALG_RSA_DECRYPT* value,
                                      TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_ALG_ID(value, buffer);
}

TPM_RC tss_Parse_TPMI_ALG_RSA_DECRYPT(TSS_SRC_DATA_BUF* buffer,
                                  TPMI_ALG_RSA_DECRYPT* value,
                                  TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_ALG_ID(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_RSA_KEY_BITS(const TPMI_RSA_KEY_BITS* value,
                                   TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_KEY_BITS(value, buffer);
}

TPM_RC tss_Parse_TPMI_RSA_KEY_BITS(TSS_SRC_DATA_BUF* buffer,
                               TPMI_RSA_KEY_BITS* value,
                               TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_KEY_BITS(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_ALG_ECC_SCHEME(const TPMI_ALG_ECC_SCHEME* value,
                                     TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_ALG_ID(value, buffer);
}

TPM_RC tss_Parse_TPMI_ALG_ECC_SCHEME(TSS_SRC_DATA_BUF* buffer,
                                 TPMI_ALG_ECC_SCHEME* value,
                                 TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_ALG_ID(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_ECC_CURVE(const TPM_ECC_CURVE* value,
                               TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT16(value, buffer);
}

TPM_RC tss_Parse_TPM_ECC_CURVE(TSS_SRC_DATA_BUF* buffer,
                           TPM_ECC_CURVE* value,
                           TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT16(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_ECC_CURVE(const TPMI_ECC_CURVE* value,
                                TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_ECC_CURVE(value, buffer);
}

TPM_RC tss_Parse_TPMI_ECC_CURVE(TSS_SRC_DATA_BUF* buffer,
                            TPMI_ECC_CURVE* value,
                            TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_ECC_CURVE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMI_ALG_PUBLIC(const TPMI_ALG_PUBLIC* value,
                                 TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_ALG_ID(value, buffer);
}

TPM_RC tss_Parse_TPMI_ALG_PUBLIC(TSS_SRC_DATA_BUF* buffer,
                             TPMI_ALG_PUBLIC* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_ALG_ID(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMA_ALGORITHM(const TPMA_ALGORITHM* value,
                                TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPMA_ALGORITHM(TSS_SRC_DATA_BUF* buffer,
                            TPMA_ALGORITHM* value,
                            TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMA_OBJECT(const TPMA_OBJECT* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPMA_OBJECT(TSS_SRC_DATA_BUF* buffer,
                         TPMA_OBJECT* value,
                         TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMA_SESSION(const TPMA_SESSION* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT8(value, buffer);
}

TPM_RC tss_Parse_TPMA_SESSION(TSS_SRC_DATA_BUF* buffer,
                          TPMA_SESSION* value,
                          TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT8(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMA_LOCALITY(const TPMA_LOCALITY* value,
                               TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT8(value, buffer);
}

TPM_RC tss_Parse_TPMA_LOCALITY(TSS_SRC_DATA_BUF* buffer,
                           TPMA_LOCALITY* value,
                           TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT8(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMA_PERMANENT(const TPMA_PERMANENT* value,
                                TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPMA_PERMANENT(TSS_SRC_DATA_BUF* buffer,
                            TPMA_PERMANENT* value,
                            TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMA_STARTUP_CLEAR(const TPMA_STARTUP_CLEAR* value,
                                    TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPMA_STARTUP_CLEAR(TSS_SRC_DATA_BUF* buffer,
                                TPMA_STARTUP_CLEAR* value,
                                TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMA_MEMORY(const TPMA_MEMORY* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPMA_MEMORY(TSS_SRC_DATA_BUF* buffer,
                         TPMA_MEMORY* value,
                         TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_CC(const TPM_CC* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPM_CC(TSS_SRC_DATA_BUF* buffer,
                    TPM_CC* value,
                    TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMA_CC(const TPMA_CC* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_CC(value, buffer);
}

TPM_RC tss_Parse_TPMA_CC(TSS_SRC_DATA_BUF* buffer,
                     TPMA_CC* value,
                     TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_CC(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_NV_INDEX(const TPM_NV_INDEX* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPM_NV_INDEX(TSS_SRC_DATA_BUF* buffer,
                          TPM_NV_INDEX* value,
                          TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMA_NV(const TPMA_NV* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPMA_NV(TSS_SRC_DATA_BUF* buffer,
                     TPMA_NV* value,
                     TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_SPEC(const TPM_SPEC* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPM_SPEC(TSS_SRC_DATA_BUF* buffer,
                      TPM_SPEC* value,
                      TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_GENERATED(const TPM_GENERATED* value,
                               TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPM_GENERATED(TSS_SRC_DATA_BUF* buffer,
                           TPM_GENERATED* value,
                           TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_RC(const TPM_RC* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPM_RC(TSS_SRC_DATA_BUF* buffer,
                    TPM_RC* value,
                    TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_CLOCK_ADJUST(const TPM_CLOCK_ADJUST* value,
                                  TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_INT8(value, buffer);
}

TPM_RC tss_Parse_TPM_CLOCK_ADJUST(TSS_SRC_DATA_BUF* buffer,
                              TPM_CLOCK_ADJUST* value,
                              TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_INT8(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_EO(const TPM_EO* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT16(value, buffer);
}

TPM_RC tss_Parse_TPM_EO(TSS_SRC_DATA_BUF* buffer,
                    TPM_EO* value,
                    TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT16(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_SU(const TPM_SU* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT16(value, buffer);
}

TPM_RC tss_Parse_TPM_SU(TSS_SRC_DATA_BUF* buffer,
                    TPM_SU* value,
                    TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT16(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_SE(const TPM_SE* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT8(value, buffer);
}

TPM_RC tss_Parse_TPM_SE(TSS_SRC_DATA_BUF* buffer,
                    TPM_SE* value,
                    TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT8(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_CAP(const TPM_CAP* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPM_CAP(TSS_SRC_DATA_BUF* buffer,
                     TPM_CAP* value,
                     TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_PT(const TPM_PT* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPM_PT(TSS_SRC_DATA_BUF* buffer,
                    TPM_PT* value,
                    TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_PT_PCR(const TPM_PT_PCR* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPM_PT_PCR(TSS_SRC_DATA_BUF* buffer,
                        TPM_PT_PCR* value,
                        TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_PS(const TPM_PS* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPM_PS(TSS_SRC_DATA_BUF* buffer,
                    TPM_PS* value,
                    TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_HT(const TPM_HT* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT8(value, buffer);
}

TPM_RC tss_Parse_TPM_HT(TSS_SRC_DATA_BUF* buffer,
                    TPM_HT* value,
                    TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT8(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_RH(const TPM_RH* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_UINT32(value, buffer);
}

TPM_RC tss_Parse_TPM_RH(TSS_SRC_DATA_BUF* buffer,
                    TPM_RH* value,
                    TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_UINT32(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPM_HC(const TPM_HC* value, TSS_DST_DATA_BUF* buffer) {
  return tss_Serialize_TPM_HANDLE(value, buffer);
}

TPM_RC tss_Parse_TPM_HC(TSS_SRC_DATA_BUF* buffer,
                    TPM_HC* value,
                    TSS_DST_DATA_BUF* value_bytes) {
  return tss_Parse_TPM_HANDLE(buffer, value, value_bytes);
}

TPM_RC tss_Serialize_TPMS_ALGORITHM_DESCRIPTION(
  const TPMS_ALGORITHM_DESCRIPTION* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPM_ALG_ID(&value->alg, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMA_ALGORITHM(&value->attributes, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_ALGORITHM_DESCRIPTION(TSS_SRC_DATA_BUF* buffer,
                                        TPMS_ALGORITHM_DESCRIPTION* value,
                                        TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPM_ALG_ID(buffer, &value->alg, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMA_ALGORITHM(buffer, &value->attributes, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMU_HA(const TPMU_HA* value,
                         TPMI_ALG_HASH selector,
                         TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_SHA384) {
    if (sizeof(value->sha384) < SHA384_DIGEST_SIZE) {
      return TPM_RC_INSUFFICIENT;
    }
    for (uint32_t i = 0; i < SHA384_DIGEST_SIZE; ++i) {
      result = tss_Serialize_BYTE(&value->sha384[i], buffer);
      if (result) {
        return result;
      }
    }
  }

  if (selector == TPM_ALG_SHA1) {
    if (sizeof(value->sha1) < SHA1_DIGEST_SIZE) {
      return TPM_RC_INSUFFICIENT;
    }
    for (uint32_t i = 0; i < SHA1_DIGEST_SIZE; ++i) {
      result = tss_Serialize_BYTE(&value->sha1[i], buffer);
      if (result) {
        return result;
      }
    }
  }

  if (selector == TPM_ALG_SM3_256) {
    if (sizeof(value->sm3_256) < SM3_256_DIGEST_SIZE) {
      return TPM_RC_INSUFFICIENT;
    }
    for (uint32_t i = 0; i < SM3_256_DIGEST_SIZE; ++i) {
      result = tss_Serialize_BYTE(&value->sm3_256[i], buffer);
      if (result) {
        return result;
      }
    }
  }

  if (selector == TPM_ALG_NULL) {
    // Do nothing.
  }

  if (selector == TPM_ALG_SHA256) {
    if (sizeof(value->sha256) < SHA256_DIGEST_SIZE) {
      return TPM_RC_INSUFFICIENT;
    }
    for (uint32_t i = 0; i < SHA256_DIGEST_SIZE; ++i) {
      result = tss_Serialize_BYTE(&value->sha256[i], buffer);
      if (result) {
        return result;
      }
    }
  }

  if (selector == TPM_ALG_SHA512) {
    if (sizeof(value->sha512) < SHA512_DIGEST_SIZE) {
      return TPM_RC_INSUFFICIENT;
    }
    for (uint32_t i = 0; i < SHA512_DIGEST_SIZE; ++i) {
      result = tss_Serialize_BYTE(&value->sha512[i], buffer);
      if (result) {
        return result;
      }
    }
  }
  return result;
}

TPM_RC tss_Parse_TPMU_HA(TSS_SRC_DATA_BUF* buffer,
                     TPMI_ALG_HASH selector,
                     TPMU_HA* value,
                     TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_SHA384) {
    if (sizeof(value->sha384) < SHA384_DIGEST_SIZE) {
      return TPM_RC_INSUFFICIENT;
    }
    for (uint32_t i = 0; i < SHA384_DIGEST_SIZE; ++i) {
      result = tss_Parse_BYTE(buffer, &value->sha384[i], value_bytes);
      if (result) {
        return result;
      }
    }
  }

  if (selector == TPM_ALG_SHA1) {
    if (sizeof(value->sha1) < SHA1_DIGEST_SIZE) {
      return TPM_RC_INSUFFICIENT;
    }
    for (uint32_t i = 0; i < SHA1_DIGEST_SIZE; ++i) {
      result = tss_Parse_BYTE(buffer, &value->sha1[i], value_bytes);
      if (result) {
        return result;
      }
    }
  }

  if (selector == TPM_ALG_SM3_256) {
    if (sizeof(value->sm3_256) < SM3_256_DIGEST_SIZE) {
      return TPM_RC_INSUFFICIENT;
    }
    for (uint32_t i = 0; i < SM3_256_DIGEST_SIZE; ++i) {
      result = tss_Parse_BYTE(buffer, &value->sm3_256[i], value_bytes);
      if (result) {
        return result;
      }
    }
  }

  if (selector == TPM_ALG_NULL) {
    // Do nothing.
  }

  if (selector == TPM_ALG_SHA256) {
    if (sizeof(value->sha256) < SHA256_DIGEST_SIZE) {
      return TPM_RC_INSUFFICIENT;
    }
    for (uint32_t i = 0; i < SHA256_DIGEST_SIZE; ++i) {
      result = tss_Parse_BYTE(buffer, &value->sha256[i], value_bytes);
      if (result) {
        return result;
      }
    }
  }

  if (selector == TPM_ALG_SHA512) {
    if (sizeof(value->sha512) < SHA512_DIGEST_SIZE) {
      return TPM_RC_INSUFFICIENT;
    }
    for (uint32_t i = 0; i < SHA512_DIGEST_SIZE; ++i) {
      result = tss_Parse_BYTE(buffer, &value->sha512[i], value_bytes);
      if (result) {
        return result;
      }
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMT_HA(const TPMT_HA* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_HASH(&value->hash_alg, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_HA(&value->digest, value->hash_alg, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMT_HA(TSS_SRC_DATA_BUF* buffer,
                     TPMT_HA* value,
                     TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_HASH(buffer, &value->hash_alg, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMU_HA(buffer, value->hash_alg, &value->digest, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_DATA(const TPM2B_DATA* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->buffer[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_DATA(TSS_SRC_DATA_BUF* buffer,
                        TPM2B_DATA* value,
                        TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->buffer[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_EVENT(const TPM2B_EVENT* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->buffer[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_EVENT(TSS_SRC_DATA_BUF* buffer,
                         TPM2B_EVENT* value,
                         TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->buffer[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_MAX_BUFFER(const TPM2B_MAX_BUFFER* value,
                                  TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->buffer[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_MAX_BUFFER(TSS_SRC_DATA_BUF* buffer,
                              TPM2B_MAX_BUFFER* value,
                              TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->buffer[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_MAX_NV_BUFFER(const TPM2B_MAX_NV_BUFFER* value,
                                     TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->buffer[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_MAX_NV_BUFFER(TSS_SRC_DATA_BUF* buffer,
                                 TPM2B_MAX_NV_BUFFER* value,
                                 TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->buffer[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_TIMEOUT(const TPM2B_TIMEOUT* value,
                               TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->buffer[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_TIMEOUT(TSS_SRC_DATA_BUF* buffer,
                           TPM2B_TIMEOUT* value,
                           TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->buffer[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_IV(const TPM2B_IV* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->buffer[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_IV(TSS_SRC_DATA_BUF* buffer,
                      TPM2B_IV* value,
                      TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->buffer[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_NAME(const TPM2B_NAME* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->name) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->name[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_NAME(TSS_SRC_DATA_BUF* buffer,
                        TPM2B_NAME* value,
                        TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->name) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->name[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_PCR_SELECT(const TPMS_PCR_SELECT* value,
                                 TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT8(&value->sizeof_select, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->pcr_select) < value->sizeof_select) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->sizeof_select; ++i) {
    result = tss_Serialize_BYTE(&value->pcr_select[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPMS_PCR_SELECT(TSS_SRC_DATA_BUF* buffer,
                             TPMS_PCR_SELECT* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT8(buffer, &value->sizeof_select, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->pcr_select) < value->sizeof_select) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint8_t i = 0; i < value->sizeof_select; ++i) {
    result = tss_Parse_BYTE(buffer, &value->pcr_select[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_PCR_SELECTION(const TPMS_PCR_SELECTION* value,
                                    TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_HASH(&value->hash, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_UINT8(&value->sizeof_select, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->pcr_select) < value->sizeof_select) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->sizeof_select; ++i) {
    result = tss_Serialize_BYTE(&value->pcr_select[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPMS_PCR_SELECTION(TSS_SRC_DATA_BUF* buffer,
                                TPMS_PCR_SELECTION* value,
                                TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_HASH(buffer, &value->hash, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_UINT8(buffer, &value->sizeof_select, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->pcr_select) < value->sizeof_select) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->sizeof_select; ++i) {
    result = tss_Parse_BYTE(buffer, &value->pcr_select[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMT_TK_CREATION(const TPMT_TK_CREATION* value,
                                  TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPM_ST(&value->tag, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMI_RH_HIERARCHY(&value->hierarchy, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_DIGEST(&value->digest, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMT_TK_CREATION(TSS_SRC_DATA_BUF* buffer,
                              TPMT_TK_CREATION* value,
                              TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPM_ST(buffer, &value->tag, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMI_RH_HIERARCHY(buffer, &value->hierarchy, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_DIGEST(buffer, &value->digest, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMT_TK_VERIFIED(const TPMT_TK_VERIFIED* value,
                                  TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPM_ST(&value->tag, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMI_RH_HIERARCHY(&value->hierarchy, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_DIGEST(&value->digest, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMT_TK_VERIFIED(TSS_SRC_DATA_BUF* buffer,
                              TPMT_TK_VERIFIED* value,
                              TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPM_ST(buffer, &value->tag, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMI_RH_HIERARCHY(buffer, &value->hierarchy, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_DIGEST(buffer, &value->digest, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMT_TK_AUTH(const TPMT_TK_AUTH* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPM_ST(&value->tag, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMI_RH_HIERARCHY(&value->hierarchy, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_DIGEST(&value->digest, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMT_TK_AUTH(TSS_SRC_DATA_BUF* buffer,
                          TPMT_TK_AUTH* value,
                          TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPM_ST(buffer, &value->tag, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMI_RH_HIERARCHY(buffer, &value->hierarchy, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_DIGEST(buffer, &value->digest, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMT_TK_HASHCHECK(const TPMT_TK_HASHCHECK* value,
                                   TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPM_ST(&value->tag, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMI_RH_HIERARCHY(&value->hierarchy, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_DIGEST(&value->digest, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMT_TK_HASHCHECK(TSS_SRC_DATA_BUF* buffer,
                               TPMT_TK_HASHCHECK* value,
                               TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPM_ST(buffer, &value->tag, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMI_RH_HIERARCHY(buffer, &value->hierarchy, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_DIGEST(buffer, &value->digest, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_ALG_PROPERTY(const TPMS_ALG_PROPERTY* value,
                                   TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPM_ALG_ID(&value->alg, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMA_ALGORITHM(&value->alg_properties, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_ALG_PROPERTY(TSS_SRC_DATA_BUF* buffer,
                               TPMS_ALG_PROPERTY* value,
                               TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPM_ALG_ID(buffer, &value->alg, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMA_ALGORITHM(buffer, &value->alg_properties, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_TAGGED_PROPERTY(const TPMS_TAGGED_PROPERTY* value,
                                      TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPM_PT(&value->property, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_UINT32(&value->value, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_TAGGED_PROPERTY(TSS_SRC_DATA_BUF* buffer,
                                  TPMS_TAGGED_PROPERTY* value,
                                  TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPM_PT(buffer, &value->property, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_UINT32(buffer, &value->value, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_TAGGED_PCR_SELECT(const TPMS_TAGGED_PCR_SELECT* value,
                                        TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPM_PT(&value->tag, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_UINT8(&value->sizeof_select, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->pcr_select) < value->sizeof_select) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->sizeof_select; ++i) {
    result = tss_Serialize_BYTE(&value->pcr_select[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPMS_TAGGED_PCR_SELECT(TSS_SRC_DATA_BUF* buffer,
                                    TPMS_TAGGED_PCR_SELECT* value,
                                    TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPM_PT(buffer, &value->tag, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_UINT8(buffer, &value->sizeof_select, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->pcr_select) < value->sizeof_select) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->sizeof_select; ++i) {
    result = tss_Parse_BYTE(buffer, &value->pcr_select[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPML_CC(const TPML_CC* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT32(&value->count, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->command_codes) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Serialize_TPM_CC(&value->command_codes[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPML_CC(TSS_SRC_DATA_BUF* buffer,
                     TPML_CC* value,
                     TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT32(buffer, &value->count, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->command_codes) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Parse_TPM_CC(buffer, &value->command_codes[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPML_CCA(const TPML_CCA* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT32(&value->count, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->command_attributes) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Serialize_TPMA_CC(&value->command_attributes[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPML_CCA(TSS_SRC_DATA_BUF* buffer,
                      TPML_CCA* value,
                      TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT32(buffer, &value->count, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->command_attributes) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Parse_TPMA_CC(buffer, &value->command_attributes[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPML_ALG(const TPML_ALG* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT32(&value->count, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->algorithms) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Serialize_TPM_ALG_ID(&value->algorithms[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPML_ALG(TSS_SRC_DATA_BUF* buffer,
                      TPML_ALG* value,
                      TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT32(buffer, &value->count, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->algorithms) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Parse_TPM_ALG_ID(buffer, &value->algorithms[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPML_HANDLE(const TPML_HANDLE* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT32(&value->count, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->handle) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Serialize_TPM_HANDLE(&value->handle[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPML_HANDLE(TSS_SRC_DATA_BUF* buffer,
                         TPML_HANDLE* value,
                         TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT32(buffer, &value->count, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->handle) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Parse_TPM_HANDLE(buffer, &value->handle[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPML_DIGEST(const TPML_DIGEST* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT32(&value->count, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->digests) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Serialize_TPM2B_DIGEST(&value->digests[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPML_DIGEST(TSS_SRC_DATA_BUF* buffer,
                         TPML_DIGEST* value,
                         TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT32(buffer, &value->count, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->digests) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Parse_TPM2B_DIGEST(buffer, &value->digests[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPML_DIGEST_VALUES(const TPML_DIGEST_VALUES* value,
                                    TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT32(&value->count, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->digests) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Serialize_TPMT_HA(&value->digests[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPML_DIGEST_VALUES(TSS_SRC_DATA_BUF* buffer,
                                TPML_DIGEST_VALUES* value,
                                TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT32(buffer, &value->count, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->digests) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Parse_TPMT_HA(buffer, &value->digests[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_DIGEST_VALUES(const TPM2B_DIGEST_VALUES* value,
                                     TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->buffer[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_DIGEST_VALUES(TSS_SRC_DATA_BUF* buffer,
                                 TPM2B_DIGEST_VALUES* value,
                                 TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->buffer[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPML_PCR_SELECTION(const TPML_PCR_SELECTION* value,
                                    TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT32(&value->count, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->pcr_selections) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Serialize_TPMS_PCR_SELECTION(&value->pcr_selections[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPML_PCR_SELECTION(TSS_SRC_DATA_BUF* buffer,
                                TPML_PCR_SELECTION* value,
                                TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT32(buffer, &value->count, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->pcr_selections) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Parse_TPMS_PCR_SELECTION(buffer, &value->pcr_selections[i],
                                      value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPML_ALG_PROPERTY(const TPML_ALG_PROPERTY* value,
                                   TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT32(&value->count, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->alg_properties) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Serialize_TPMS_ALG_PROPERTY(&value->alg_properties[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPML_ALG_PROPERTY(TSS_SRC_DATA_BUF* buffer,
                               TPML_ALG_PROPERTY* value,
                               TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT32(buffer, &value->count, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->alg_properties) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result =
        tss_Parse_TPMS_ALG_PROPERTY(buffer, &value->alg_properties[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPML_TAGGED_TPM_PROPERTY(const TPML_TAGGED_TPM_PROPERTY* value,
                                          TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT32(&value->count, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->tpm_property) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Serialize_TPMS_TAGGED_PROPERTY(&value->tpm_property[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPML_TAGGED_TPM_PROPERTY(TSS_SRC_DATA_BUF* buffer,
                                      TPML_TAGGED_TPM_PROPERTY* value,
                                      TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT32(buffer, &value->count, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->tpm_property) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Parse_TPMS_TAGGED_PROPERTY(buffer, &value->tpm_property[i],
                                        value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPML_TAGGED_PCR_PROPERTY(const TPML_TAGGED_PCR_PROPERTY* value,
                                          TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT32(&value->count, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->pcr_property) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Serialize_TPMS_TAGGED_PCR_SELECT(&value->pcr_property[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPML_TAGGED_PCR_PROPERTY(TSS_SRC_DATA_BUF* buffer,
                                      TPML_TAGGED_PCR_PROPERTY* value,
                                      TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT32(buffer, &value->count, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->pcr_property) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Parse_TPMS_TAGGED_PCR_SELECT(buffer, &value->pcr_property[i],
                                          value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPML_ECC_CURVE(const TPML_ECC_CURVE* value,
                                TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT32(&value->count, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->ecc_curves) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Serialize_TPM_ECC_CURVE(&value->ecc_curves[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPML_ECC_CURVE(TSS_SRC_DATA_BUF* buffer,
                            TPML_ECC_CURVE* value,
                            TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT32(buffer, &value->count, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->ecc_curves) < value->count) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->count; ++i) {
    result = tss_Parse_TPM_ECC_CURVE(buffer, &value->ecc_curves[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMU_CAPABILITIES(const TPMU_CAPABILITIES* value,
                                   TPM_CAP selector,
                                   TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_CAP_PCRS) {
    result = tss_Serialize_TPML_PCR_SELECTION(&value->assigned_pcr, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_CAP_TPM_PROPERTIES) {
    result = tss_Serialize_TPML_TAGGED_TPM_PROPERTY(&value->tpm_properties, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_CAP_PP_COMMANDS) {
    result = tss_Serialize_TPML_CC(&value->pp_commands, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_CAP_AUDIT_COMMANDS) {
    result = tss_Serialize_TPML_CC(&value->audit_commands, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_CAP_COMMANDS) {
    result = tss_Serialize_TPML_CCA(&value->command, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_CAP_ECC_CURVES) {
    result = tss_Serialize_TPML_ECC_CURVE(&value->ecc_curves, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_CAP_PCR_PROPERTIES) {
    result = tss_Serialize_TPML_TAGGED_PCR_PROPERTY(&value->pcr_properties, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_CAP_HANDLES) {
    result = tss_Serialize_TPML_HANDLE(&value->handles, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_CAP_ALGS) {
    result = tss_Serialize_TPML_ALG_PROPERTY(&value->algorithms, buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPMU_CAPABILITIES(TSS_SRC_DATA_BUF* buffer,
                               TPM_CAP selector,
                               TPMU_CAPABILITIES* value,
                               TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_CAP_PCRS) {
    result =
        tss_Parse_TPML_PCR_SELECTION(buffer, &value->assigned_pcr, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_CAP_TPM_PROPERTIES) {
    result = tss_Parse_TPML_TAGGED_TPM_PROPERTY(buffer, &value->tpm_properties,
                                            value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_CAP_PP_COMMANDS) {
    result = tss_Parse_TPML_CC(buffer, &value->pp_commands, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_CAP_AUDIT_COMMANDS) {
    result = tss_Parse_TPML_CC(buffer, &value->audit_commands, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_CAP_COMMANDS) {
    result = tss_Parse_TPML_CCA(buffer, &value->command, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_CAP_ECC_CURVES) {
    result = tss_Parse_TPML_ECC_CURVE(buffer, &value->ecc_curves, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_CAP_PCR_PROPERTIES) {
    result = tss_Parse_TPML_TAGGED_PCR_PROPERTY(buffer, &value->pcr_properties,
                                            value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_CAP_HANDLES) {
    result = tss_Parse_TPML_HANDLE(buffer, &value->handles, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_CAP_ALGS) {
    result = tss_Parse_TPML_ALG_PROPERTY(buffer, &value->algorithms, value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_CAPABILITY_DATA(const TPMS_CAPABILITY_DATA* value,
                                      TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPM_CAP(&value->capability, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_CAPABILITIES(&value->data, value->capability, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_CAPABILITY_DATA(TSS_SRC_DATA_BUF* buffer,
                                  TPMS_CAPABILITY_DATA* value,
                                  TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPM_CAP(buffer, &value->capability, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMU_CAPABILITIES(buffer, value->capability, &value->data,
                                   value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_CLOCK_INFO(const TPMS_CLOCK_INFO* value,
                                 TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT64(&value->clock, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_UINT32(&value->reset_count, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_UINT32(&value->restart_count, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMI_YES_NO(&value->safe, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_CLOCK_INFO(TSS_SRC_DATA_BUF* buffer,
                             TPMS_CLOCK_INFO* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT64(buffer, &value->clock, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_UINT32(buffer, &value->reset_count, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_UINT32(buffer, &value->restart_count, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMI_YES_NO(buffer, &value->safe, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_TIME_INFO(const TPMS_TIME_INFO* value,
                                TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT64(&value->time, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMS_CLOCK_INFO(&value->clock_info, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_TIME_INFO(TSS_SRC_DATA_BUF* buffer,
                            TPMS_TIME_INFO* value,
                            TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT64(buffer, &value->time, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMS_CLOCK_INFO(buffer, &value->clock_info, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_TIME_ATTEST_INFO(const TPMS_TIME_ATTEST_INFO* value,
                                       TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMS_TIME_INFO(&value->time, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_UINT64(&value->firmware_version, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_TIME_ATTEST_INFO(TSS_SRC_DATA_BUF* buffer,
                                   TPMS_TIME_ATTEST_INFO* value,
                                   TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMS_TIME_INFO(buffer, &value->time, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_UINT64(buffer, &value->firmware_version, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_CERTIFY_INFO(const TPMS_CERTIFY_INFO* value,
                                   TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPM2B_NAME(&value->name, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_NAME(&value->qualified_name, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_CERTIFY_INFO(TSS_SRC_DATA_BUF* buffer,
                               TPMS_CERTIFY_INFO* value,
                               TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPM2B_NAME(buffer, &value->name, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_NAME(buffer, &value->qualified_name, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_QUOTE_INFO(const TPMS_QUOTE_INFO* value,
                                 TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPML_PCR_SELECTION(&value->pcr_select, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_DIGEST(&value->pcr_digest, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_QUOTE_INFO(TSS_SRC_DATA_BUF* buffer,
                             TPMS_QUOTE_INFO* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPML_PCR_SELECTION(buffer, &value->pcr_select, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_DIGEST(buffer, &value->pcr_digest, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_COMMAND_AUDIT_INFO(const TPMS_COMMAND_AUDIT_INFO* value,
                                         TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT64(&value->audit_counter, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM_ALG_ID(&value->digest_alg, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_DIGEST(&value->audit_digest, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_DIGEST(&value->command_digest, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_COMMAND_AUDIT_INFO(TSS_SRC_DATA_BUF* buffer,
                                     TPMS_COMMAND_AUDIT_INFO* value,
                                     TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT64(buffer, &value->audit_counter, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM_ALG_ID(buffer, &value->digest_alg, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_DIGEST(buffer, &value->audit_digest, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_DIGEST(buffer, &value->command_digest, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_SESSION_AUDIT_INFO(const TPMS_SESSION_AUDIT_INFO* value,
                                         TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_YES_NO(&value->exclusive_session, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_DIGEST(&value->session_digest, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_SESSION_AUDIT_INFO(TSS_SRC_DATA_BUF* buffer,
                                     TPMS_SESSION_AUDIT_INFO* value,
                                     TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_YES_NO(buffer, &value->exclusive_session, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_DIGEST(buffer, &value->session_digest, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_CREATION_INFO(const TPMS_CREATION_INFO* value,
                                    TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPM2B_NAME(&value->object_name, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_DIGEST(&value->creation_hash, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_CREATION_INFO(TSS_SRC_DATA_BUF* buffer,
                                TPMS_CREATION_INFO* value,
                                TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPM2B_NAME(buffer, &value->object_name, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_DIGEST(buffer, &value->creation_hash, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_NV_CERTIFY_INFO(const TPMS_NV_CERTIFY_INFO* value,
                                      TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPM2B_NAME(&value->index_name, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_UINT16(&value->offset, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_MAX_NV_BUFFER(&value->nv_contents, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_NV_CERTIFY_INFO(TSS_SRC_DATA_BUF* buffer,
                                  TPMS_NV_CERTIFY_INFO* value,
                                  TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPM2B_NAME(buffer, &value->index_name, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_UINT16(buffer, &value->offset, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_MAX_NV_BUFFER(buffer, &value->nv_contents, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMU_ATTEST(const TPMU_ATTEST* value,
                             TPMI_ST_ATTEST selector,
                             TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ST_ATTEST_SESSION_AUDIT) {
    result = tss_Serialize_TPMS_SESSION_AUDIT_INFO(&value->session_audit, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ST_ATTEST_QUOTE) {
    result = tss_Serialize_TPMS_QUOTE_INFO(&value->quote, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ST_ATTEST_COMMAND_AUDIT) {
    result = tss_Serialize_TPMS_COMMAND_AUDIT_INFO(&value->command_audit, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ST_ATTEST_CERTIFY) {
    result = tss_Serialize_TPMS_CERTIFY_INFO(&value->certify, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ST_ATTEST_NV) {
    result = tss_Serialize_TPMS_NV_CERTIFY_INFO(&value->nv, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ST_ATTEST_TIME) {
    result = tss_Serialize_TPMS_TIME_ATTEST_INFO(&value->time, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ST_ATTEST_CREATION) {
    result = tss_Serialize_TPMS_CREATION_INFO(&value->creation, buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPMU_ATTEST(TSS_SRC_DATA_BUF* buffer,
                         TPMI_ST_ATTEST selector,
                         TPMU_ATTEST* value,
                         TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ST_ATTEST_SESSION_AUDIT) {
    result = tss_Parse_TPMS_SESSION_AUDIT_INFO(buffer, &value->session_audit,
                                           value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ST_ATTEST_QUOTE) {
    result = tss_Parse_TPMS_QUOTE_INFO(buffer, &value->quote, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ST_ATTEST_COMMAND_AUDIT) {
    result = tss_Parse_TPMS_COMMAND_AUDIT_INFO(buffer, &value->command_audit,
                                           value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ST_ATTEST_CERTIFY) {
    result = tss_Parse_TPMS_CERTIFY_INFO(buffer, &value->certify, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ST_ATTEST_NV) {
    result = tss_Parse_TPMS_NV_CERTIFY_INFO(buffer, &value->nv, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ST_ATTEST_TIME) {
    result = tss_Parse_TPMS_TIME_ATTEST_INFO(buffer, &value->time, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ST_ATTEST_CREATION) {
    result = tss_Parse_TPMS_CREATION_INFO(buffer, &value->creation, value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_ATTEST(const TPMS_ATTEST* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPM_GENERATED(&value->magic, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMI_ST_ATTEST(&value->type, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_NAME(&value->qualified_signer, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_DATA(&value->extra_data, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMS_CLOCK_INFO(&value->clock_info, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_UINT64(&value->firmware_version, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_ATTEST(&value->attested, value->type, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_ATTEST(TSS_SRC_DATA_BUF* buffer,
                         TPMS_ATTEST* value,
                         TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPM_GENERATED(buffer, &value->magic, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMI_ST_ATTEST(buffer, &value->type, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_NAME(buffer, &value->qualified_signer, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_DATA(buffer, &value->extra_data, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMS_CLOCK_INFO(buffer, &value->clock_info, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_UINT64(buffer, &value->firmware_version, value_bytes);
  if (result) {
    return result;
  }

  result =
      tss_Parse_TPMU_ATTEST(buffer, value->type, &value->attested, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_ATTEST(const TPM2B_ATTEST* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->attestation_data) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->attestation_data[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_ATTEST(TSS_SRC_DATA_BUF* buffer,
                          TPM2B_ATTEST* value,
                          TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->attestation_data) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->attestation_data[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_AUTH_COMMAND(const TPMS_AUTH_COMMAND* value,
                                   TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_SH_AUTH_SESSION(&value->session_handle, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_NONCE(&value->nonce, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMA_SESSION(&value->session_attributes, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_AUTH(&value->hmac, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_AUTH_COMMAND(TSS_SRC_DATA_BUF* buffer,
                               TPMS_AUTH_COMMAND* value,
                               TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result =
      tss_Parse_TPMI_SH_AUTH_SESSION(buffer, &value->session_handle, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_NONCE(buffer, &value->nonce, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMA_SESSION(buffer, &value->session_attributes, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_AUTH(buffer, &value->hmac, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_AUTH_RESPONSE(const TPMS_AUTH_RESPONSE* value,
                                    TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPM2B_NONCE(&value->nonce, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMA_SESSION(&value->session_attributes, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_AUTH(&value->hmac, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_AUTH_RESPONSE(TSS_SRC_DATA_BUF* buffer,
                                TPMS_AUTH_RESPONSE* value,
                                TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPM2B_NONCE(buffer, &value->nonce, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMA_SESSION(buffer, &value->session_attributes, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_AUTH(buffer, &value->hmac, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMU_SYM_KEY_BITS(const TPMU_SYM_KEY_BITS* value,
                                   TPMI_ALG_SYM selector,
                                   TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_NULL) {
    // Do nothing.
  }

  if (selector == TPM_ALG_SM4) {
    result = tss_Serialize_TPMI_SM4_KEY_BITS(&value->sm4, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_AES) {
    result = tss_Serialize_TPMI_AES_KEY_BITS(&value->aes, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_XOR) {
    result = tss_Serialize_TPMI_ALG_HASH(&value->xor_, buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPMU_SYM_KEY_BITS(TSS_SRC_DATA_BUF* buffer,
                               TPMI_ALG_SYM selector,
                               TPMU_SYM_KEY_BITS* value,
                               TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_NULL) {
    // Do nothing.
  }

  if (selector == TPM_ALG_SM4) {
    result = tss_Parse_TPMI_SM4_KEY_BITS(buffer, &value->sm4, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_AES) {
    result = tss_Parse_TPMI_AES_KEY_BITS(buffer, &value->aes, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_XOR) {
    result = tss_Parse_TPMI_ALG_HASH(buffer, &value->xor_, value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMU_SYM_MODE(const TPMU_SYM_MODE* value,
                               TPMI_ALG_SYM selector,
                               TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_NULL) {
    // Do nothing.
  }

  if (selector == TPM_ALG_SM4) {
    result = tss_Serialize_TPMI_ALG_SYM_MODE(&value->sm4, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_AES) {
    result = tss_Serialize_TPMI_ALG_SYM_MODE(&value->aes, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_XOR) {
    // Do nothing.
  }
  return result;
}

TPM_RC tss_Parse_TPMU_SYM_MODE(TSS_SRC_DATA_BUF* buffer,
                           TPMI_ALG_SYM selector,
                           TPMU_SYM_MODE* value,
                           TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_NULL) {
    // Do nothing.
  }

  if (selector == TPM_ALG_SM4) {
    result = tss_Parse_TPMI_ALG_SYM_MODE(buffer, &value->sm4, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_AES) {
    result = tss_Parse_TPMI_ALG_SYM_MODE(buffer, &value->aes, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_XOR) {
    // Do nothing.
  }
  return result;
}

TPM_RC tss_Serialize_TPMU_SYM_DETAILS(const TPMU_SYM_DETAILS* value,
                                  TPMI_ALG_SYM selector,
                                  TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;
    return result;
}

TPM_RC tss_Parse_TPMU_SYM_DETAILS(TSS_SRC_DATA_BUF* buffer,
                              TPMI_ALG_SYM selector,
                              TPMU_SYM_DETAILS* value,
                              TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;
    return result;
}

TPM_RC tss_Serialize_TPMT_SYM_DEF(const TPMT_SYM_DEF* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_SYM(&value->algorithm, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_SYM_KEY_BITS(&value->key_bits, value->algorithm, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_SYM_MODE(&value->mode, value->algorithm, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_SYM_DETAILS(&value->details, value->algorithm, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMT_SYM_DEF(TSS_SRC_DATA_BUF* buffer,
                          TPMT_SYM_DEF* value,
                          TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_SYM(buffer, &value->algorithm, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMU_SYM_KEY_BITS(buffer, value->algorithm, &value->key_bits,
                                   value_bytes);
  if (result) {
    return result;
  }

  result =
      tss_Parse_TPMU_SYM_MODE(buffer, value->algorithm, &value->mode, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMU_SYM_DETAILS(buffer, value->algorithm, &value->details,
                                  value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMT_SYM_DEF_OBJECT(const TPMT_SYM_DEF_OBJECT* value,
                                     TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_SYM_OBJECT(&value->algorithm, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_SYM_KEY_BITS(&value->key_bits, value->algorithm, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_SYM_MODE(&value->mode, value->algorithm, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_SYM_DETAILS(&value->details, value->algorithm, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMT_SYM_DEF_OBJECT(TSS_SRC_DATA_BUF* buffer,
                                 TPMT_SYM_DEF_OBJECT* value,
                                 TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_SYM_OBJECT(buffer, &value->algorithm, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMU_SYM_KEY_BITS(buffer, value->algorithm, &value->key_bits,
                                   value_bytes);
  if (result) {
    return result;
  }

  result =
      tss_Parse_TPMU_SYM_MODE(buffer, value->algorithm, &value->mode, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMU_SYM_DETAILS(buffer, value->algorithm, &value->details,
                                  value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_SYM_KEY(const TPM2B_SYM_KEY* value,
                               TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->buffer[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_SYM_KEY(TSS_SRC_DATA_BUF* buffer,
                           TPM2B_SYM_KEY* value,
                           TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->buffer[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_SYMCIPHER_PARMS(const TPMS_SYMCIPHER_PARMS* value,
                                      TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMT_SYM_DEF_OBJECT(&value->sym, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_SYMCIPHER_PARMS(TSS_SRC_DATA_BUF* buffer,
                                  TPMS_SYMCIPHER_PARMS* value,
                                  TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMT_SYM_DEF_OBJECT(buffer, &value->sym, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_SENSITIVE_DATA(const TPM2B_SENSITIVE_DATA* value,
                                      TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->buffer[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_SENSITIVE_DATA(TSS_SRC_DATA_BUF* buffer,
                                  TPM2B_SENSITIVE_DATA* value,
                                  TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->buffer[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_SENSITIVE_CREATE(const TPMS_SENSITIVE_CREATE* value,
                                       TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPM2B_AUTH(&value->user_auth, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_SENSITIVE_DATA(&value->data, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_SENSITIVE_CREATE(TSS_SRC_DATA_BUF* buffer,
                                   TPMS_SENSITIVE_CREATE* value,
                                   TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPM2B_AUTH(buffer, &value->user_auth, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_SENSITIVE_DATA(buffer, &value->data, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_SENSITIVE_CREATE(const TPM2B_SENSITIVE_CREATE* value,
                                        TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  TSS_DST_DATA_BUF field_bytes;
  result = dst_data_buf_set_at_offset(&field_bytes, buffer, sizeof(UINT16));
  if (result) {
    return result;
  }
  if (value->size) {
    if (value->size != sizeof(TPMS_SENSITIVE_CREATE)) {
      return TPM_RC_SIZE;
    }
    result = tss_Serialize_TPMS_SENSITIVE_CREATE(&value->sensitive, &field_bytes);
    if (result) {
      return result;
    }
  }
  UINT16 field_size = field_bytes.size - buffer->size - sizeof(UINT16);
  result = tss_Serialize_UINT16(&field_size, buffer);
  if (result) {
    return result;
  }
  return dst_data_buf_set_at_offset(buffer, &field_bytes, 0);
}

TPM_RC tss_Parse_TPM2B_SENSITIVE_CREATE(TSS_SRC_DATA_BUF* buffer,
                                    TPM2B_SENSITIVE_CREATE* value,
                                    TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  UINT16 parsed_size = 0;
  result = tss_Parse_UINT16(buffer, &parsed_size, value_bytes);
  if (result) {
    return result;
  }
  if (!parsed_size) {
    value->size = 0;
  } else {
    value->size = sizeof(TPMS_SENSITIVE_CREATE);
    result =
        tss_Parse_TPMS_SENSITIVE_CREATE(buffer, &value->sensitive, value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_SCHEME_XOR(const TPMS_SCHEME_XOR* value,
                                 TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_HASH(&value->hash_alg, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMI_ALG_KDF(&value->kdf, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_SCHEME_XOR(TSS_SRC_DATA_BUF* buffer,
                             TPMS_SCHEME_XOR* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_HASH(buffer, &value->hash_alg, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMI_ALG_KDF(buffer, &value->kdf, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMU_SCHEME_KEYEDHASH(const TPMU_SCHEME_KEYEDHASH* value,
                                       TPMI_ALG_KEYEDHASH_SCHEME selector,
                                       TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_NULL) {
    // Do nothing.
  }

  if (selector == TPM_ALG_HMAC) {
    result = tss_Serialize_TPMS_SCHEME_HMAC(&value->hmac, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_XOR) {
    result = tss_Serialize_TPMS_SCHEME_XOR(&value->xor_, buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPMU_SCHEME_KEYEDHASH(TSS_SRC_DATA_BUF* buffer,
                                   TPMI_ALG_KEYEDHASH_SCHEME selector,
                                   TPMU_SCHEME_KEYEDHASH* value,
                                   TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_NULL) {
    // Do nothing.
  }

  if (selector == TPM_ALG_HMAC) {
    result = tss_Parse_TPMS_SCHEME_HMAC(buffer, &value->hmac, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_XOR) {
    result = tss_Parse_TPMS_SCHEME_XOR(buffer, &value->xor_, value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMT_KEYEDHASH_SCHEME(const TPMT_KEYEDHASH_SCHEME* value,
                                       TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_KEYEDHASH_SCHEME(&value->scheme, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_SCHEME_KEYEDHASH(&value->details, value->scheme, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMT_KEYEDHASH_SCHEME(TSS_SRC_DATA_BUF* buffer,
                                   TPMT_KEYEDHASH_SCHEME* value,
                                   TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_KEYEDHASH_SCHEME(buffer, &value->scheme, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMU_SCHEME_KEYEDHASH(buffer, value->scheme, &value->details,
                                       value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_SCHEME_ECDAA(const TPMS_SCHEME_ECDAA* value,
                                   TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_HASH(&value->hash_alg, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_UINT16(&value->count, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_SCHEME_ECDAA(TSS_SRC_DATA_BUF* buffer,
                               TPMS_SCHEME_ECDAA* value,
                               TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_HASH(buffer, &value->hash_alg, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_UINT16(buffer, &value->count, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMU_SIG_SCHEME(const TPMU_SIG_SCHEME* value,
                                 TPMI_ALG_SIG_SCHEME selector,
                                 TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_HMAC) {
    result = tss_Serialize_TPMS_SCHEME_HMAC(&value->hmac, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECSCHNORR) {
    result = tss_Serialize_TPMS_SCHEME_ECSCHNORR(&value->ec_schnorr, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_RSAPSS) {
    result = tss_Serialize_TPMS_SCHEME_RSAPSS(&value->rsapss, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECDAA) {
    result = tss_Serialize_TPMS_SCHEME_ECDAA(&value->ecdaa, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_RSASSA) {
    result = tss_Serialize_TPMS_SCHEME_RSASSA(&value->rsassa, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_SM2) {
    result = tss_Serialize_TPMS_SCHEME_SM2(&value->sm2, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECDSA) {
    result = tss_Serialize_TPMS_SCHEME_ECDSA(&value->ecdsa, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_NULL) {
    // Do nothing.
  }
  return result;
}

TPM_RC tss_Parse_TPMU_SIG_SCHEME(TSS_SRC_DATA_BUF* buffer,
                             TPMI_ALG_SIG_SCHEME selector,
                             TPMU_SIG_SCHEME* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_HMAC) {
    result = tss_Parse_TPMS_SCHEME_HMAC(buffer, &value->hmac, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECSCHNORR) {
    result =
        tss_Parse_TPMS_SCHEME_ECSCHNORR(buffer, &value->ec_schnorr, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_RSAPSS) {
    result = tss_Parse_TPMS_SCHEME_RSAPSS(buffer, &value->rsapss, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECDAA) {
    result = tss_Parse_TPMS_SCHEME_ECDAA(buffer, &value->ecdaa, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_RSASSA) {
    result = tss_Parse_TPMS_SCHEME_RSASSA(buffer, &value->rsassa, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_SM2) {
    result = tss_Parse_TPMS_SCHEME_SM2(buffer, &value->sm2, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECDSA) {
    result = tss_Parse_TPMS_SCHEME_ECDSA(buffer, &value->ecdsa, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_NULL) {
    // Do nothing.
  }
  return result;
}

TPM_RC tss_Serialize_TPMT_SIG_SCHEME(const TPMT_SIG_SCHEME* value,
                                 TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_SIG_SCHEME(&value->scheme, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_SIG_SCHEME(&value->details, value->scheme, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMT_SIG_SCHEME(TSS_SRC_DATA_BUF* buffer,
                             TPMT_SIG_SCHEME* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_SIG_SCHEME(buffer, &value->scheme, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMU_SIG_SCHEME(buffer, value->scheme, &value->details,
                                 value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_SCHEME_OAEP(const TPMS_SCHEME_OAEP* value,
                                  TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_HASH(&value->hash_alg, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_SCHEME_OAEP(TSS_SRC_DATA_BUF* buffer,
                              TPMS_SCHEME_OAEP* value,
                              TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_HASH(buffer, &value->hash_alg, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_SCHEME_ECDH(const TPMS_SCHEME_ECDH* value,
                                  TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_HASH(&value->hash_alg, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_SCHEME_ECDH(TSS_SRC_DATA_BUF* buffer,
                              TPMS_SCHEME_ECDH* value,
                              TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_HASH(buffer, &value->hash_alg, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_SCHEME_MGF1(const TPMS_SCHEME_MGF1* value,
                                  TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_HASH(&value->hash_alg, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_SCHEME_MGF1(TSS_SRC_DATA_BUF* buffer,
                              TPMS_SCHEME_MGF1* value,
                              TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_HASH(buffer, &value->hash_alg, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_SCHEME_KDF1_SP800_56a(
    const TPMS_SCHEME_KDF1_SP800_56a* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_HASH(&value->hash_alg, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_SCHEME_KDF1_SP800_56a(TSS_SRC_DATA_BUF* buffer,
                                        TPMS_SCHEME_KDF1_SP800_56a* value,
                                        TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_HASH(buffer, &value->hash_alg, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_SCHEME_KDF2(const TPMS_SCHEME_KDF2* value,
                                  TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_HASH(&value->hash_alg, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_SCHEME_KDF2(TSS_SRC_DATA_BUF* buffer,
                              TPMS_SCHEME_KDF2* value,
                              TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_HASH(buffer, &value->hash_alg, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_SCHEME_KDF1_SP800_108(
    const TPMS_SCHEME_KDF1_SP800_108* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_HASH(&value->hash_alg, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_SCHEME_KDF1_SP800_108(TSS_SRC_DATA_BUF* buffer,
                                        TPMS_SCHEME_KDF1_SP800_108* value,
                                        TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_HASH(buffer, &value->hash_alg, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMU_KDF_SCHEME(const TPMU_KDF_SCHEME* value,
                                 TPMI_ALG_KDF selector,
                                 TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_KDF1_SP800_56a) {
    result = tss_Serialize_TPMS_SCHEME_KDF1_SP800_56a(&value->kdf1_sp800_56a, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_MGF1) {
    result = tss_Serialize_TPMS_SCHEME_MGF1(&value->mgf1, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_KDF1_SP800_108) {
    result = tss_Serialize_TPMS_SCHEME_KDF1_SP800_108(&value->kdf1_sp800_108, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_KDF2) {
    result = tss_Serialize_TPMS_SCHEME_KDF2(&value->kdf2, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_NULL) {
    // Do nothing.
  }
  return result;
}

TPM_RC tss_Parse_TPMU_KDF_SCHEME(TSS_SRC_DATA_BUF* buffer,
                             TPMI_ALG_KDF selector,
                             TPMU_KDF_SCHEME* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_KDF1_SP800_56a) {
    result = tss_Parse_TPMS_SCHEME_KDF1_SP800_56a(buffer, &value->kdf1_sp800_56a,
                                              value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_MGF1) {
    result = tss_Parse_TPMS_SCHEME_MGF1(buffer, &value->mgf1, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_KDF1_SP800_108) {
    result = tss_Parse_TPMS_SCHEME_KDF1_SP800_108(buffer, &value->kdf1_sp800_108,
                                              value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_KDF2) {
    result = tss_Parse_TPMS_SCHEME_KDF2(buffer, &value->kdf2, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_NULL) {
    // Do nothing.
  }
  return result;
}

TPM_RC tss_Serialize_TPMT_KDF_SCHEME(const TPMT_KDF_SCHEME* value,
                                 TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_KDF(&value->scheme, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_KDF_SCHEME(&value->details, value->scheme, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMT_KDF_SCHEME(TSS_SRC_DATA_BUF* buffer,
                             TPMT_KDF_SCHEME* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_KDF(buffer, &value->scheme, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMU_KDF_SCHEME(buffer, value->scheme, &value->details,
                                 value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMU_ASYM_SCHEME(const TPMU_ASYM_SCHEME* value,
                                  TPMI_ALG_ASYM_SCHEME selector,
                                  TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_RSAES) {
    // Do nothing.
  }

  if (selector == TPM_ALG_ECSCHNORR) {
    result = tss_Serialize_TPMS_SCHEME_ECSCHNORR(&value->ec_schnorr, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_NULL) {
    // Do nothing.
  }

  if (selector == TPM_ALG_ECDH) {
    result = tss_Serialize_TPMS_SCHEME_ECDH(&value->ecdh, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_OAEP) {
    result = tss_Serialize_TPMS_SCHEME_OAEP(&value->oaep, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_RSAPSS) {
    result = tss_Serialize_TPMS_SCHEME_RSAPSS(&value->rsapss, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECDAA) {
    result = tss_Serialize_TPMS_SCHEME_ECDAA(&value->ecdaa, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_RSASSA) {
    result = tss_Serialize_TPMS_SCHEME_RSASSA(&value->rsassa, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_SM2) {
    result = tss_Serialize_TPMS_SCHEME_SM2(&value->sm2, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECDSA) {
    result = tss_Serialize_TPMS_SCHEME_ECDSA(&value->ecdsa, buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPMU_ASYM_SCHEME(TSS_SRC_DATA_BUF* buffer,
                              TPMI_ALG_ASYM_SCHEME selector,
                              TPMU_ASYM_SCHEME* value,
                              TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_RSAES) {
    // Do nothing.
  }

  if (selector == TPM_ALG_ECSCHNORR) {
    result =
        tss_Parse_TPMS_SCHEME_ECSCHNORR(buffer, &value->ec_schnorr, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_NULL) {
    // Do nothing.
  }

  if (selector == TPM_ALG_ECDH) {
    result = tss_Parse_TPMS_SCHEME_ECDH(buffer, &value->ecdh, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_OAEP) {
    result = tss_Parse_TPMS_SCHEME_OAEP(buffer, &value->oaep, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_RSAPSS) {
    result = tss_Parse_TPMS_SCHEME_RSAPSS(buffer, &value->rsapss, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECDAA) {
    result = tss_Parse_TPMS_SCHEME_ECDAA(buffer, &value->ecdaa, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_RSASSA) {
    result = tss_Parse_TPMS_SCHEME_RSASSA(buffer, &value->rsassa, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_SM2) {
    result = tss_Parse_TPMS_SCHEME_SM2(buffer, &value->sm2, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECDSA) {
    result = tss_Parse_TPMS_SCHEME_ECDSA(buffer, &value->ecdsa, value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMT_ASYM_SCHEME(const TPMT_ASYM_SCHEME* value,
                                  TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_ASYM_SCHEME(&value->scheme, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_ASYM_SCHEME(&value->details, value->scheme, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMT_ASYM_SCHEME(TSS_SRC_DATA_BUF* buffer,
                              TPMT_ASYM_SCHEME* value,
                              TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_ASYM_SCHEME(buffer, &value->scheme, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMU_ASYM_SCHEME(buffer, value->scheme, &value->details,
                                  value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMT_RSA_SCHEME(const TPMT_RSA_SCHEME* value,
                                 TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_RSA_SCHEME(&value->scheme, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_ASYM_SCHEME(&value->details, value->scheme, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMT_RSA_SCHEME(TSS_SRC_DATA_BUF* buffer,
                             TPMT_RSA_SCHEME* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_RSA_SCHEME(buffer, &value->scheme, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMU_ASYM_SCHEME(buffer, value->scheme, &value->details,
                                  value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMT_RSA_DECRYPT(const TPMT_RSA_DECRYPT* value,
                                  TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_RSA_DECRYPT(&value->scheme, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_ASYM_SCHEME(&value->details, value->scheme, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMT_RSA_DECRYPT(TSS_SRC_DATA_BUF* buffer,
                              TPMT_RSA_DECRYPT* value,
                              TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_RSA_DECRYPT(buffer, &value->scheme, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMU_ASYM_SCHEME(buffer, value->scheme, &value->details,
                                  value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_PUBLIC_KEY_RSA(const TPM2B_PUBLIC_KEY_RSA* value,
                                      TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->buffer[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_PUBLIC_KEY_RSA(TSS_SRC_DATA_BUF* buffer,
                                  TPM2B_PUBLIC_KEY_RSA* value,
                                  TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->buffer[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_PRIVATE_KEY_RSA(const TPM2B_PRIVATE_KEY_RSA* value,
                                       TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->buffer[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_PRIVATE_KEY_RSA(TSS_SRC_DATA_BUF* buffer,
                                   TPM2B_PRIVATE_KEY_RSA* value,
                                   TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->buffer[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_ECC_PARAMETER(const TPM2B_ECC_PARAMETER* value,
                                     TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->buffer[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_ECC_PARAMETER(TSS_SRC_DATA_BUF* buffer,
                                 TPM2B_ECC_PARAMETER* value,
                                 TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->buffer[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_ECC_POINT(const TPMS_ECC_POINT* value,
                                TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPM2B_ECC_PARAMETER(&value->x, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_ECC_PARAMETER(&value->y, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_ECC_POINT(TSS_SRC_DATA_BUF* buffer,
                            TPMS_ECC_POINT* value,
                            TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPM2B_ECC_PARAMETER(buffer, &value->x, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_ECC_PARAMETER(buffer, &value->y, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_ECC_POINT(const TPM2B_ECC_POINT* value,
                                 TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  TSS_DST_DATA_BUF field_bytes;
  result = dst_data_buf_set_at_offset(&field_bytes, buffer, sizeof(UINT16));
  if (result) {
    return result;
  }
  if (value->size) {
    if (value->size != sizeof(TPMS_ECC_POINT)) {
      return TPM_RC_SIZE;
    }
    result = tss_Serialize_TPMS_ECC_POINT(&value->point, &field_bytes);
    if (result) {
      return result;
    }
  }
  UINT16 field_size = field_bytes.size - buffer->size - sizeof(UINT16);
  result = tss_Serialize_UINT16(&field_size, buffer);
  if (result) {
    return result;
  }
  return dst_data_buf_set_at_offset(buffer, &field_bytes, 0);
}

TPM_RC tss_Parse_TPM2B_ECC_POINT(TSS_SRC_DATA_BUF* buffer,
                             TPM2B_ECC_POINT* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  UINT16 parsed_size = 0;
  result = tss_Parse_UINT16(buffer, &parsed_size, value_bytes);
  if (result) {
    return result;
  }
  if (!parsed_size) {
    value->size = 0;
  } else {
    value->size = sizeof(TPMS_ECC_POINT);
    result = tss_Parse_TPMS_ECC_POINT(buffer, &value->point, value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMT_ECC_SCHEME(const TPMT_ECC_SCHEME* value,
                                 TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_ECC_SCHEME(&value->scheme, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_SIG_SCHEME(&value->details, value->scheme, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMT_ECC_SCHEME(TSS_SRC_DATA_BUF* buffer,
                             TPMT_ECC_SCHEME* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_ECC_SCHEME(buffer, &value->scheme, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMU_SIG_SCHEME(buffer, value->scheme, &value->details,
                                 value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_ALGORITHM_DETAIL_ECC(
    const TPMS_ALGORITHM_DETAIL_ECC* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPM_ECC_CURVE(&value->curve_id, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_UINT16(&value->key_size, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMT_KDF_SCHEME(&value->kdf, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMT_ECC_SCHEME(&value->sign, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_ECC_PARAMETER(&value->p, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_ECC_PARAMETER(&value->a, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_ECC_PARAMETER(&value->b, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_ECC_PARAMETER(&value->g_x, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_ECC_PARAMETER(&value->g_y, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_ECC_PARAMETER(&value->n, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_ECC_PARAMETER(&value->h, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_ALGORITHM_DETAIL_ECC(TSS_SRC_DATA_BUF* buffer,
                                       TPMS_ALGORITHM_DETAIL_ECC* value,
                                       TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPM_ECC_CURVE(buffer, &value->curve_id, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_UINT16(buffer, &value->key_size, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMT_KDF_SCHEME(buffer, &value->kdf, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMT_ECC_SCHEME(buffer, &value->sign, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_ECC_PARAMETER(buffer, &value->p, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_ECC_PARAMETER(buffer, &value->a, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_ECC_PARAMETER(buffer, &value->b, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_ECC_PARAMETER(buffer, &value->g_x, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_ECC_PARAMETER(buffer, &value->g_y, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_ECC_PARAMETER(buffer, &value->n, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_ECC_PARAMETER(buffer, &value->h, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_SIGNATURE_RSASSA(const TPMS_SIGNATURE_RSASSA* value,
                                       TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_HASH(&value->hash, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_PUBLIC_KEY_RSA(&value->sig, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_SIGNATURE_RSASSA(TSS_SRC_DATA_BUF* buffer,
                                   TPMS_SIGNATURE_RSASSA* value,
                                   TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_HASH(buffer, &value->hash, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_PUBLIC_KEY_RSA(buffer, &value->sig, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_SIGNATURE_RSAPSS(const TPMS_SIGNATURE_RSAPSS* value,
                                       TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_HASH(&value->hash, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_PUBLIC_KEY_RSA(&value->sig, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_SIGNATURE_RSAPSS(TSS_SRC_DATA_BUF* buffer,
                                   TPMS_SIGNATURE_RSAPSS* value,
                                   TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_HASH(buffer, &value->hash, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_PUBLIC_KEY_RSA(buffer, &value->sig, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_SIGNATURE_ECDSA(const TPMS_SIGNATURE_ECDSA* value,
                                      TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_HASH(&value->hash, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_ECC_PARAMETER(&value->signature_r, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_ECC_PARAMETER(&value->signature_s, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_SIGNATURE_ECDSA(TSS_SRC_DATA_BUF* buffer,
                                  TPMS_SIGNATURE_ECDSA* value,
                                  TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_HASH(buffer, &value->hash, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_ECC_PARAMETER(buffer, &value->signature_r, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_ECC_PARAMETER(buffer, &value->signature_s, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMU_SIGNATURE(const TPMU_SIGNATURE* value,
                                TPMI_ALG_SIG_SCHEME selector,
                                TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_HMAC) {
    result = tss_Serialize_TPMT_HA(&value->hmac, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECSCHNORR) {
    result = tss_Serialize_TPMS_SIGNATURE_ECDSA(&value->ecschnorr, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_RSAPSS) {
    result = tss_Serialize_TPMS_SIGNATURE_RSAPSS(&value->rsapss, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECDAA) {
    result = tss_Serialize_TPMS_SIGNATURE_ECDSA(&value->ecdaa, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_RSASSA) {
    result = tss_Serialize_TPMS_SIGNATURE_RSASSA(&value->rsassa, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_SM2) {
    result = tss_Serialize_TPMS_SIGNATURE_ECDSA(&value->sm2, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECDSA) {
    result = tss_Serialize_TPMS_SIGNATURE_ECDSA(&value->ecdsa, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_NULL) {
    // Do nothing.
  }
  return result;
}

TPM_RC tss_Parse_TPMU_SIGNATURE(TSS_SRC_DATA_BUF* buffer,
                            TPMI_ALG_SIG_SCHEME selector,
                            TPMU_SIGNATURE* value,
                            TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_HMAC) {
    result = tss_Parse_TPMT_HA(buffer, &value->hmac, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECSCHNORR) {
    result = tss_Parse_TPMS_SIGNATURE_ECDSA(buffer, &value->ecschnorr, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_RSAPSS) {
    result = tss_Parse_TPMS_SIGNATURE_RSAPSS(buffer, &value->rsapss, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECDAA) {
    result = tss_Parse_TPMS_SIGNATURE_ECDSA(buffer, &value->ecdaa, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_RSASSA) {
    result = tss_Parse_TPMS_SIGNATURE_RSASSA(buffer, &value->rsassa, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_SM2) {
    result = tss_Parse_TPMS_SIGNATURE_ECDSA(buffer, &value->sm2, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECDSA) {
    result = tss_Parse_TPMS_SIGNATURE_ECDSA(buffer, &value->ecdsa, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_NULL) {
    // Do nothing.
  }
  return result;
}

TPM_RC tss_Serialize_TPMT_SIGNATURE(const TPMT_SIGNATURE* value,
                                TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_SIG_SCHEME(&value->sig_alg, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_SIGNATURE(&value->signature, value->sig_alg, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMT_SIGNATURE(TSS_SRC_DATA_BUF* buffer,
                            TPMT_SIGNATURE* value,
                            TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_SIG_SCHEME(buffer, &value->sig_alg, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMU_SIGNATURE(buffer, value->sig_alg, &value->signature,
                                value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_ENCRYPTED_SECRET(const TPM2B_ENCRYPTED_SECRET* value,
                                        TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->secret) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->secret[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_ENCRYPTED_SECRET(TSS_SRC_DATA_BUF* buffer,
                                    TPM2B_ENCRYPTED_SECRET* value,
                                    TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->secret) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->secret[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_KEYEDHASH_PARMS(const TPMS_KEYEDHASH_PARMS* value,
                                      TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMT_KEYEDHASH_SCHEME(&value->scheme, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_KEYEDHASH_PARMS(TSS_SRC_DATA_BUF* buffer,
                                  TPMS_KEYEDHASH_PARMS* value,
                                  TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMT_KEYEDHASH_SCHEME(buffer, &value->scheme, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_ASYM_PARMS(const TPMS_ASYM_PARMS* value,
                                 TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMT_SYM_DEF_OBJECT(&value->symmetric, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMT_ASYM_SCHEME(&value->scheme, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_ASYM_PARMS(TSS_SRC_DATA_BUF* buffer,
                             TPMS_ASYM_PARMS* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMT_SYM_DEF_OBJECT(buffer, &value->symmetric, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMT_ASYM_SCHEME(buffer, &value->scheme, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_RSA_PARMS(const TPMS_RSA_PARMS* value,
                                TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMT_SYM_DEF_OBJECT(&value->symmetric, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMT_RSA_SCHEME(&value->scheme, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMI_RSA_KEY_BITS(&value->key_bits, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_UINT32(&value->exponent, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_RSA_PARMS(TSS_SRC_DATA_BUF* buffer,
                            TPMS_RSA_PARMS* value,
                            TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMT_SYM_DEF_OBJECT(buffer, &value->symmetric, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMT_RSA_SCHEME(buffer, &value->scheme, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMI_RSA_KEY_BITS(buffer, &value->key_bits, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_UINT32(buffer, &value->exponent, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_ECC_PARMS(const TPMS_ECC_PARMS* value,
                                TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMT_SYM_DEF_OBJECT(&value->symmetric, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMT_ECC_SCHEME(&value->scheme, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMI_ECC_CURVE(&value->curve_id, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMT_KDF_SCHEME(&value->kdf, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_ECC_PARMS(TSS_SRC_DATA_BUF* buffer,
                            TPMS_ECC_PARMS* value,
                            TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMT_SYM_DEF_OBJECT(buffer, &value->symmetric, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMT_ECC_SCHEME(buffer, &value->scheme, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMI_ECC_CURVE(buffer, &value->curve_id, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMT_KDF_SCHEME(buffer, &value->kdf, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMU_PUBLIC_PARMS(const TPMU_PUBLIC_PARMS* value,
                                   TPMI_ALG_PUBLIC selector,
                                   TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_KEYEDHASH) {
    result = tss_Serialize_TPMS_KEYEDHASH_PARMS(&value->keyed_hash_detail, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_RSA) {
    result = tss_Serialize_TPMS_RSA_PARMS(&value->rsa_detail, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_SYMCIPHER) {
    result = tss_Serialize_TPMS_SYMCIPHER_PARMS(&value->sym_detail, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECC) {
    result = tss_Serialize_TPMS_ECC_PARMS(&value->ecc_detail, buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPMU_PUBLIC_PARMS(TSS_SRC_DATA_BUF* buffer,
                               TPMI_ALG_PUBLIC selector,
                               TPMU_PUBLIC_PARMS* value,
                               TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_KEYEDHASH) {
    result = tss_Parse_TPMS_KEYEDHASH_PARMS(buffer, &value->keyed_hash_detail,
                                        value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_RSA) {
    result = tss_Parse_TPMS_RSA_PARMS(buffer, &value->rsa_detail, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_SYMCIPHER) {
    result =
        tss_Parse_TPMS_SYMCIPHER_PARMS(buffer, &value->sym_detail, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECC) {
    result = tss_Parse_TPMS_ECC_PARMS(buffer, &value->ecc_detail, value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMT_PUBLIC_PARMS(const TPMT_PUBLIC_PARMS* value,
                                   TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_PUBLIC(&value->type, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_PUBLIC_PARMS(&value->parameters, value->type, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMT_PUBLIC_PARMS(TSS_SRC_DATA_BUF* buffer,
                               TPMT_PUBLIC_PARMS* value,
                               TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_PUBLIC(buffer, &value->type, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMU_PUBLIC_PARMS(buffer, value->type, &value->parameters,
                                   value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMU_PUBLIC_ID(const TPMU_PUBLIC_ID* value,
                                TPMI_ALG_PUBLIC selector,
                                TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_KEYEDHASH) {
    result = tss_Serialize_TPM2B_DIGEST(&value->keyed_hash, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_RSA) {
    result = tss_Serialize_TPM2B_PUBLIC_KEY_RSA(&value->rsa, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_SYMCIPHER) {
    result = tss_Serialize_TPM2B_DIGEST(&value->sym, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECC) {
    result = tss_Serialize_TPMS_ECC_POINT(&value->ecc, buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPMU_PUBLIC_ID(TSS_SRC_DATA_BUF* buffer,
                            TPMI_ALG_PUBLIC selector,
                            TPMU_PUBLIC_ID* value,
                            TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_KEYEDHASH) {
    result = tss_Parse_TPM2B_DIGEST(buffer, &value->keyed_hash, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_RSA) {
    result = tss_Parse_TPM2B_PUBLIC_KEY_RSA(buffer, &value->rsa, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_SYMCIPHER) {
    result = tss_Parse_TPM2B_DIGEST(buffer, &value->sym, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECC) {
    result = tss_Parse_TPMS_ECC_POINT(buffer, &value->ecc, value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMT_PUBLIC(const TPMT_PUBLIC* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_PUBLIC(&value->type, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMI_ALG_HASH(&value->name_alg, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMA_OBJECT(&value->object_attributes, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_DIGEST(&value->auth_policy, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_PUBLIC_PARMS(&value->parameters, value->type, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_PUBLIC_ID(&value->unique, value->type, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMT_PUBLIC(TSS_SRC_DATA_BUF* buffer,
                         TPMT_PUBLIC* value,
                         TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_PUBLIC(buffer, &value->type, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMI_ALG_HASH(buffer, &value->name_alg, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMA_OBJECT(buffer, &value->object_attributes, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_DIGEST(buffer, &value->auth_policy, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMU_PUBLIC_PARMS(buffer, value->type, &value->parameters,
                                   value_bytes);
  if (result) {
    return result;
  }

  result =
      tss_Parse_TPMU_PUBLIC_ID(buffer, value->type, &value->unique, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_PUBLIC(const TPM2B_PUBLIC* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  TSS_DST_DATA_BUF field_bytes;
  result = dst_data_buf_set_at_offset(&field_bytes, buffer, sizeof(UINT16));
  if (result) {
    return result;
  }
  if (value->size) {
    if (value->size != sizeof(TPMT_PUBLIC)) {
      return TPM_RC_SIZE;
    }
    result = tss_Serialize_TPMT_PUBLIC(&value->public_area, &field_bytes);
    if (result) {
      return result;
    }
  }
  UINT16 field_size = field_bytes.size - buffer->size - sizeof(UINT16);
  result = tss_Serialize_UINT16(&field_size, buffer);
  if (result) {
    return result;
  }
  return dst_data_buf_set_at_offset(buffer, &field_bytes, 0);
}

TPM_RC tss_Parse_TPM2B_PUBLIC(TSS_SRC_DATA_BUF* buffer,
                          TPM2B_PUBLIC* value,
                          TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  UINT16 parsed_size = 0;
  result = tss_Parse_UINT16(buffer, &parsed_size, value_bytes);
  if (result) {
    return result;
  }
  if (!parsed_size) {
    value->size = 0;
  } else {
    value->size = sizeof(TPMT_PUBLIC);
    result = tss_Parse_TPMT_PUBLIC(buffer, &value->public_area, value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_PRIVATE_VENDOR_SPECIFIC(
    const TPM2B_PRIVATE_VENDOR_SPECIFIC* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->buffer[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_PRIVATE_VENDOR_SPECIFIC(TSS_SRC_DATA_BUF* buffer,
                                           TPM2B_PRIVATE_VENDOR_SPECIFIC* value,
                                           TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->buffer[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMU_SENSITIVE_COMPOSITE(const TPMU_SENSITIVE_COMPOSITE* value,
                                          TPMI_ALG_PUBLIC selector,
                                          TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_KEYEDHASH) {
    result = tss_Serialize_TPM2B_SENSITIVE_DATA(&value->bits, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_RSA) {
    result = tss_Serialize_TPM2B_PRIVATE_KEY_RSA(&value->rsa, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_SYMCIPHER) {
    result = tss_Serialize_TPM2B_SYM_KEY(&value->sym, buffer);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECC) {
    result = tss_Serialize_TPM2B_ECC_PARAMETER(&value->ecc, buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPMU_SENSITIVE_COMPOSITE(TSS_SRC_DATA_BUF* buffer,
                                      TPMI_ALG_PUBLIC selector,
                                      TPMU_SENSITIVE_COMPOSITE* value,
                                      TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  if (selector == TPM_ALG_KEYEDHASH) {
    result = tss_Parse_TPM2B_SENSITIVE_DATA(buffer, &value->bits, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_RSA) {
    result = tss_Parse_TPM2B_PRIVATE_KEY_RSA(buffer, &value->rsa, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_SYMCIPHER) {
    result = tss_Parse_TPM2B_SYM_KEY(buffer, &value->sym, value_bytes);
    if (result) {
      return result;
    }
  }

  if (selector == TPM_ALG_ECC) {
    result = tss_Parse_TPM2B_ECC_PARAMETER(buffer, &value->ecc, value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMT_SENSITIVE(const TPMT_SENSITIVE* value,
                                TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_ALG_PUBLIC(&value->sensitive_type, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_AUTH(&value->auth_value, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_DIGEST(&value->seed_value, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMU_SENSITIVE_COMPOSITE(&value->sensitive,
                                              value->sensitive_type, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMT_SENSITIVE(TSS_SRC_DATA_BUF* buffer,
                            TPMT_SENSITIVE* value,
                            TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_ALG_PUBLIC(buffer, &value->sensitive_type, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_AUTH(buffer, &value->auth_value, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_DIGEST(buffer, &value->seed_value, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMU_SENSITIVE_COMPOSITE(buffer, value->sensitive_type,
                                          &value->sensitive, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_SENSITIVE(const TPM2B_SENSITIVE* value,
                                 TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  TSS_DST_DATA_BUF field_bytes;
  result = dst_data_buf_set_at_offset(&field_bytes, buffer, sizeof(UINT16));
  if (result) {
    return result;
  }
  if (value->size) {
    if (value->size != sizeof(TPMT_SENSITIVE)) {
      return TPM_RC_SIZE;
    }
    result = tss_Serialize_TPMT_SENSITIVE(&value->sensitive_area, &field_bytes);
    if (result) {
      return result;
    }
  }
  UINT16 field_size = field_bytes.size - buffer->size - sizeof(UINT16);
  result = tss_Serialize_UINT16(&field_size, buffer);
  if (result) {
    return result;
  }
  return dst_data_buf_set_at_offset(buffer, &field_bytes, 0);
}

TPM_RC tss_Parse_TPM2B_SENSITIVE(TSS_SRC_DATA_BUF* buffer,
                             TPM2B_SENSITIVE* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  UINT16 parsed_size = 0;
  result = tss_Parse_UINT16(buffer, &parsed_size, value_bytes);
  if (result) {
    return result;
  }
  if (!parsed_size) {
    value->size = 0;
  } else {
    value->size = sizeof(TPMT_SENSITIVE);
    result = tss_Parse_TPMT_SENSITIVE(buffer, &value->sensitive_area, value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize__PRIVATE(const _PRIVATE* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPM2B_DIGEST(&value->integrity_outer, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_DIGEST(&value->integrity_inner, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMT_SENSITIVE(&value->sensitive, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse__PRIVATE(TSS_SRC_DATA_BUF* buffer,
                      _PRIVATE* value,
                      TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPM2B_DIGEST(buffer, &value->integrity_outer, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_DIGEST(buffer, &value->integrity_inner, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMT_SENSITIVE(buffer, &value->sensitive, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_PRIVATE(const TPM2B_PRIVATE* value,
                               TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->buffer[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_PRIVATE(TSS_SRC_DATA_BUF* buffer,
                           TPM2B_PRIVATE* value,
                           TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->buffer[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize__ID_OBJECT(const _ID_OBJECT* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPM2B_DIGEST(&value->integrity_hmac, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_DIGEST(&value->enc_identity, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse__ID_OBJECT(TSS_SRC_DATA_BUF* buffer,
                        _ID_OBJECT* value,
                        TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPM2B_DIGEST(buffer, &value->integrity_hmac, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_DIGEST(buffer, &value->enc_identity, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_ID_OBJECT(const TPM2B_ID_OBJECT* value,
                                 TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->credential) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->credential[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_ID_OBJECT(TSS_SRC_DATA_BUF* buffer,
                             TPM2B_ID_OBJECT* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->credential) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->credential[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_NV_PUBLIC(const TPMS_NV_PUBLIC* value,
                                TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPMI_RH_NV_INDEX(&value->nv_index, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMI_ALG_HASH(&value->name_alg, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMA_NV(&value->attributes, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_DIGEST(&value->auth_policy, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_UINT16(&value->data_size, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_NV_PUBLIC(TSS_SRC_DATA_BUF* buffer,
                            TPMS_NV_PUBLIC* value,
                            TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPMI_RH_NV_INDEX(buffer, &value->nv_index, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMI_ALG_HASH(buffer, &value->name_alg, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMA_NV(buffer, &value->attributes, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_DIGEST(buffer, &value->auth_policy, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_UINT16(buffer, &value->data_size, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_NV_PUBLIC(const TPM2B_NV_PUBLIC* value,
                                 TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  TSS_DST_DATA_BUF field_bytes;
  result = dst_data_buf_set_at_offset(&field_bytes, buffer, sizeof(UINT16));
  if (result) {
    return result;
  }
  if (value->size) {
    if (value->size != sizeof(TPMS_NV_PUBLIC)) {
      return TPM_RC_SIZE;
    }
    result = tss_Serialize_TPMS_NV_PUBLIC(&value->nv_public, &field_bytes);
    if (result) {
      return result;
    }
  }
  UINT16 field_size = field_bytes.size - buffer->size - sizeof(UINT16);
  result = tss_Serialize_UINT16(&field_size, buffer);
  if (result) {
    return result;
  }
  return dst_data_buf_set_at_offset(buffer, &field_bytes, 0);
}

TPM_RC tss_Parse_TPM2B_NV_PUBLIC(TSS_SRC_DATA_BUF* buffer,
                             TPM2B_NV_PUBLIC* value,
                             TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  UINT16 parsed_size = 0;
  result = tss_Parse_UINT16(buffer, &parsed_size, value_bytes);
  if (result) {
    return result;
  }
  if (!parsed_size) {
    value->size = 0;
  } else {
    value->size = sizeof(TPMS_NV_PUBLIC);
    result = tss_Parse_TPMS_NV_PUBLIC(buffer, &value->nv_public, value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_CONTEXT_SENSITIVE(const TPM2B_CONTEXT_SENSITIVE* value,
                                         TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->buffer[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_CONTEXT_SENSITIVE(TSS_SRC_DATA_BUF* buffer,
                                     TPM2B_CONTEXT_SENSITIVE* value,
                                     TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->buffer[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_CONTEXT_DATA(const TPMS_CONTEXT_DATA* value,
                                   TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPM2B_DIGEST(&value->integrity, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_CONTEXT_SENSITIVE(&value->encrypted, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_CONTEXT_DATA(TSS_SRC_DATA_BUF* buffer,
                               TPMS_CONTEXT_DATA* value,
                               TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPM2B_DIGEST(buffer, &value->integrity, value_bytes);
  if (result) {
    return result;
  }

  result =
      tss_Parse_TPM2B_CONTEXT_SENSITIVE(buffer, &value->encrypted, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_CONTEXT_DATA(const TPM2B_CONTEXT_DATA* value,
                                    TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT16(&value->size, buffer);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Serialize_BYTE(&value->buffer[i], buffer);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Parse_TPM2B_CONTEXT_DATA(TSS_SRC_DATA_BUF* buffer,
                                TPM2B_CONTEXT_DATA* value,
                                TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT16(buffer, &value->size, value_bytes);
  if (result) {
    return result;
  }

  if (arraysize(value->buffer) < value->size) {
    return TPM_RC_INSUFFICIENT;
  }
  for (uint32_t i = 0; i < value->size; ++i) {
    result = tss_Parse_BYTE(buffer, &value->buffer[i], value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_CONTEXT(const TPMS_CONTEXT* value, TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_UINT64(&value->sequence, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMI_DH_CONTEXT(&value->saved_handle, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMI_RH_HIERARCHY(&value->hierarchy, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_CONTEXT_DATA(&value->context_blob, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_CONTEXT(TSS_SRC_DATA_BUF* buffer,
                          TPMS_CONTEXT* value,
                          TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_UINT64(buffer, &value->sequence, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMI_DH_CONTEXT(buffer, &value->saved_handle, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMI_RH_HIERARCHY(buffer, &value->hierarchy, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_CONTEXT_DATA(buffer, &value->context_blob, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPMS_CREATION_DATA(const TPMS_CREATION_DATA* value,
                                    TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Serialize_TPML_PCR_SELECTION(&value->pcr_select, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_DIGEST(&value->pcr_digest, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPMA_LOCALITY(&value->locality, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM_ALG_ID(&value->parent_name_alg, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_NAME(&value->parent_name, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_NAME(&value->parent_qualified_name, buffer);
  if (result) {
    return result;
  }

  result = tss_Serialize_TPM2B_DATA(&value->outside_info, buffer);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Parse_TPMS_CREATION_DATA(TSS_SRC_DATA_BUF* buffer,
                                TPMS_CREATION_DATA* value,
                                TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  result = tss_Parse_TPML_PCR_SELECTION(buffer, &value->pcr_select, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_DIGEST(buffer, &value->pcr_digest, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPMA_LOCALITY(buffer, &value->locality, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM_ALG_ID(buffer, &value->parent_name_alg, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_NAME(buffer, &value->parent_name, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_NAME(buffer, &value->parent_qualified_name, value_bytes);
  if (result) {
    return result;
  }

  result = tss_Parse_TPM2B_DATA(buffer, &value->outside_info, value_bytes);
  if (result) {
    return result;
  }
  return result;
}

TPM_RC tss_Serialize_TPM2B_CREATION_DATA(const TPM2B_CREATION_DATA* value,
                                     TSS_DST_DATA_BUF* buffer) {
  TPM_RC result = TPM_RC_SUCCESS;

  TSS_DST_DATA_BUF field_bytes;
  result = dst_data_buf_set_at_offset(&field_bytes, buffer, sizeof(UINT16));
  if (result) {
    return result;
  }
  if (value->size) {
    if (value->size != sizeof(TPMS_CREATION_DATA)) {
      return TPM_RC_SIZE;
    }
    result = tss_Serialize_TPMS_CREATION_DATA(&value->creation_data, &field_bytes);
    if (result) {
      return result;
    }
  }
  UINT16 field_size = field_bytes.size - buffer->size - sizeof(UINT16);
  result = tss_Serialize_UINT16(&field_size, buffer);
  if (result) {
    return result;
  }
  return dst_data_buf_set_at_offset(buffer, &field_bytes, 0);
}

TPM_RC tss_Parse_TPM2B_CREATION_DATA(TSS_SRC_DATA_BUF* buffer,
                                 TPM2B_CREATION_DATA* value,
                                 TSS_DST_DATA_BUF* value_bytes) {
  TPM_RC result = TPM_RC_SUCCESS;

  UINT16 parsed_size = 0;
  result = tss_Parse_UINT16(buffer, &parsed_size, value_bytes);
  if (result) {
    return result;
  }
  if (!parsed_size) {
    value->size = 0;
  } else {
    value->size = sizeof(TPMS_CREATION_DATA);
    result =
        tss_Parse_TPMS_CREATION_DATA(buffer, &value->creation_data, value_bytes);
    if (result) {
      return result;
    }
  }
  return result;
}

static TPM_RC EncryptField(TSS_DST_DATA_BUF* field,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate
) {
  if (authorization_delegate) {
    // Encrypt just the field data, not the size.
    if (field->size < sizeof(UINT16))
      return TPM_RC_FAILURE;
    TSS_SRC_DATA_BUF field_data = {field->size - sizeof(UINT16),
                                 field->buffer + sizeof(UINT16)};
    if (tss_EncryptCommandParameter(authorization_delegate, &field_data)) {
      return TSS_RC_ENCRYPTION_FAILED;
    }
  }
  return TPM_RC_SUCCESS;
}

// Insert authorization section and set tag and command size fields
static TPM_RC FinalizeCommand(
    TSS_DST_DATA_BUF* serialized_command,
    size_t authorization_section_offset,
    uint8_t* command_hash_buffer /* SHA256_DIGEST_SIZE */,
    bool is_command_parameter_encryption_possible,
    bool is_response_parameter_encryption_possible,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  TPMI_ST_COMMAND_TAG tag = TPM_ST_NO_SESSIONS;
  TPM_RC rc;

  if (authorization_delegate && command_hash_buffer) {
    TSS_SRC_DATA_BUF command_hash = {SHA256_DIGEST_SIZE, command_hash_buffer};
    uint8_t authorization_section_buf[MAX_AUTH_SECTION_SIZE];
    TSS_DST_DATA_BUF authorization_section_bytes = {MAX_AUTH_SECTION_SIZE, 0,
                                                  authorization_section_buf};
    if (tss_GetCommandAuthorization(authorization_delegate,
            &command_hash, is_command_parameter_encryption_possible,
            is_response_parameter_encryption_possible,
            &authorization_section_bytes)) {
      return TSS_RC_AUTHORIZATION_FAILED;
    }
    if (authorization_section_bytes.size != 0) {
      TSS_DST_DATA_BUF authorization_section;
      rc = dst_data_buf_insert(serialized_command, &authorization_section,
          authorization_section_offset,
          authorization_section_bytes.size + sizeof(UINT32));
      if (rc != TPM_RC_SUCCESS) {
        return rc;
      }
      tag = TPM_ST_SESSIONS;
      UINT32 size = authorization_section_bytes.size;
      rc = tss_Serialize_UINT32(&size, &authorization_section);
      if (rc != TPM_RC_SUCCESS) {
        return rc;
      }
      rc = dst_data_buf_append(&authorization_section,
                               authorization_section_bytes.buffer,
                               authorization_section_bytes.size);
    }
  }
  UINT32 command_size = serialized_command->size;
  if (command_size < 10) {
    return TPM_RC_FAILURE;
  }
  TSS_DST_DATA_BUF tag_size_bytes = {
      sizeof(TPMI_ST_COMMAND_TAG) + sizeof(UINT32),
      0,
      serialized_command->buffer};
  rc = tss_Serialize_TPMI_ST_COMMAND_TAG(&tag, &tag_size_bytes);
  if (rc != TPM_RC_SUCCESS) {
    return rc;
  }
  return tss_Serialize_UINT32(&command_size, &tag_size_bytes);
}

typedef struct cmd_processor_t {
  TPM_CC command_code;
  bool is_command_parameter_encryption_possible;
  bool is_response_parameter_encryption_possible;
  size_t authorization_section_offset;
  TSS_DST_DATA_BUF* serialized_command;
  TSS_DST_DATA_BUF field_bytes;
  TSS_AUTHORIZATION_DELEGATE* authorization_delegate;
  pinweaver_eal_sha256_ctx_t hash;
  bool next_encrypt;
} cmd_processor_t;

static TSS_DST_DATA_BUF* cmd_process_start_field(cmd_processor_t* p) {
  dst_data_buf_set_to_remainder(&p->field_bytes, p->serialized_command);
  return &p->field_bytes;
}

static TPM_RC cmd_process_field(cmd_processor_t* p,
                                const TPM2B_NAME* optional_name) {
  p->field_bytes.max_size = p->field_bytes.size;
  dst_data_buf_skip(p->serialized_command, p->field_bytes.size, NULL);
  if (p->next_encrypt) {
    TPM_RC rc = EncryptField(&p->field_bytes,
                             p->authorization_delegate);
    if (rc != TPM_RC_SUCCESS)
      return rc;
  }
  p->next_encrypt = false;

  const void* data;
  size_t size;
  if (optional_name) {
    data = optional_name->name;
    size = optional_name->size;
    if (size > sizeof(optional_name->name))
      return TPM_RC_INSUFFICIENT;
  } else {
    data = p->field_bytes.buffer;
    size = p->field_bytes.size;
  }
  if (pinweaver_eal_sha256_update(&p->hash, data, size))
    return TSS_RC_HASH_ERROR;
  return TPM_RC_SUCCESS;
}

static TPM_RC cmd_process_start(
    cmd_processor_t* p,
    TPM_CC command_code,
    bool is_command_parameter_encryption_possible,
    bool is_response_parameter_encryption_possible,
    TSS_DST_DATA_BUF* serialized_command,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate
) {
  memset(p, 0, sizeof(cmd_processor_t));

  p->command_code = command_code;
  p->is_command_parameter_encryption_possible =
      is_command_parameter_encryption_possible;
  p->is_response_parameter_encryption_possible =
      is_response_parameter_encryption_possible;
  p->serialized_command = serialized_command;
  p->authorization_delegate = authorization_delegate;

  if (pinweaver_eal_sha256_init(&p->hash))
    return TSS_RC_HASH_ERROR;

  TPM_RC rc = dst_data_buf_skip(p->serialized_command,
      sizeof(TPMI_ST_COMMAND_TAG) + sizeof(UINT32), NULL);
  if (rc != TPM_RC_SUCCESS)
    return rc;

  TSS_DST_DATA_BUF* field_bytes = cmd_process_start_field(p);
  rc = tss_Serialize_TPM_CC(&p->command_code, field_bytes);
  if (rc != TPM_RC_SUCCESS)
    return rc;
  return cmd_process_field(p, NULL);
}

static void cmd_process_param_section(cmd_processor_t* p) {
  p->authorization_section_offset = p->serialized_command->size;
  p->next_encrypt = p->is_command_parameter_encryption_possible;
}

static TPM_RC cmd_process_finish(cmd_processor_t* p) {
  uint8_t command_hash_buffer[SHA256_DIGEST_SIZE];
  if (pinweaver_eal_sha256_final(&p->hash, command_hash_buffer))
    return TSS_RC_HASH_ERROR;
  if (p->authorization_section_offset == 0) {
    p->authorization_section_offset = p->serialized_command->size;
  }
  return FinalizeCommand(p->serialized_command,
      p->authorization_section_offset,
      command_hash_buffer,
      p->is_command_parameter_encryption_possible,
      p->is_response_parameter_encryption_possible,
      p->authorization_delegate);
}

#define CMD_PROCESS(data, type, name) \
  do { \
    TSS_DST_DATA_BUF* field_bytes = cmd_process_start_field(&p); \
    TPM_RC rc = tss_Serialize_##type((data), field_bytes); \
    if (rc != TPM_RC_SUCCESS) { \
      PINWEAVER_EAL_INFO("Failed to serialize %s %s", #type, #data); \
      return rc; \
    } \
    rc = cmd_process_field(&p, (name)); \
    if (rc != TPM_RC_SUCCESS) { \
      PINWEAVER_EAL_INFO("Failed to process %s %s", #type, #data); \
      return rc; \
    } \
  } while(0)

#define CMD_PROCESS_VAL(field, type) \
  CMD_PROCESS(&(field), type, NULL)

#define CMD_PROCESS_HANDLE(field) \
  CMD_PROCESS(&(field), TPM_HANDLE, field##_name)

#define CMD_PROCESS_PTR(field, type) \
  CMD_PROCESS(field, type, NULL)

#define CMD_PROCESS_START(cc, cmd_enc, rsp_enc) \
  cmd_processor_t p; \
  do { \
    TPM_RC rc = cmd_process_start(&p, (cc), (cmd_enc), (rsp_enc), \
        serialized_command, authorization_delegate); \
    if (rc != TPM_RC_SUCCESS) { \
      PINWEAVER_EAL_INFO("Failed to start processing for cmd %#x", cc); \
      return rc; \
    } \
  } while(0)

#define CMD_PROCESS_FINISH() \
  return cmd_process_finish(&p)

#define CMD_PROCESS_PARAM_SECTION() \
  cmd_process_param_section(&p)

typedef struct rsp_processor_t {
  TSS_SRC_DATA_BUF buffer;
  TSS_AUTHORIZATION_DELEGATE* authorization_delegate;
  uint8_t* response_code_ptr;
  TPM_CC command_code;
  TPM_ST tag;
  bool decrypt_first;
} rsp_processor_t;

static TPM_RC rsp_process_start(
    rsp_processor_t* p,
    const TSS_SRC_DATA_BUF* response,
    TPM_CC command_code,
    bool decrypt_first,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate
) {
  memset(p, 0, sizeof(rsp_processor_t));

  p->buffer.size = response->size;
  p->buffer.buffer = response->buffer;
  p->command_code = command_code;
  p->decrypt_first = decrypt_first;
  p->authorization_delegate = authorization_delegate;

  TPM_RC rc = tss_Parse_TPM_ST(&p->buffer, &p->tag, NULL);
  if (rc != TPM_RC_SUCCESS) {
    return rc;
  }

  UINT32 response_size;
  rc = tss_Parse_UINT32(&p->buffer, &response_size, NULL);
  if (rc != TPM_RC_SUCCESS) {
    return rc;
  }

  TPM_RC response_code;
  p->response_code_ptr = p->buffer.buffer;
  rc = tss_Parse_TPM_RC(&p->buffer, &response_code, NULL);
  if (rc != TPM_RC_SUCCESS) {
    return rc;
  }

  if (response_size != response->size) {
    return TPM_RC_SIZE;
  }
  return response_code;
}

static TPM_RC rsp_process_auth_block(rsp_processor_t* p) {
  if (p->tag != TPM_ST_SESSIONS)
    return TPM_RC_SUCCESS;

  TPM_CC command_code_buffer;
  TSS_DST_DATA_BUF command_code_bytes = {sizeof(TPM_CC), 0,
                                       (uint8_t *)&command_code_buffer};
  TPM_RC rc = tss_Serialize_TPM_CC(&p->command_code, &command_code_bytes);
  if (rc != TPM_RC_SUCCESS) {
    return rc;
  }

  UINT32 parameter_size;
  rc = tss_Parse_UINT32(&p->buffer, &parameter_size, NULL);
  if (rc != TPM_RC_SUCCESS) {
    return rc;
  }
  if (parameter_size > p->buffer.size) {
    return TPM_RC_INSUFFICIENT;
  }

  TSS_SRC_DATA_BUF authorization_section_bytes =
      {p->buffer.size - parameter_size, p->buffer.buffer + parameter_size};
  p->buffer.size = parameter_size;

  pinweaver_eal_sha256_ctx_t hash;
  if (pinweaver_eal_sha256_init(&hash))
    return TSS_RC_HASH_ERROR;
  if (pinweaver_eal_sha256_update(&hash, p->response_code_ptr,
                                  sizeof(TPM_RC))) {
    return TSS_RC_HASH_ERROR;
  }
  if (pinweaver_eal_sha256_update(&hash, command_code_bytes.buffer,
                                  command_code_bytes.size)) {
    return TSS_RC_HASH_ERROR;
  }
  if (pinweaver_eal_sha256_update(&hash, p->buffer.buffer, p->buffer.size))
    return TSS_RC_HASH_ERROR;

  uint8_t response_hash_buffer[SHA256_DIGEST_SIZE];
  TSS_SRC_DATA_BUF response_hash = {SHA256_DIGEST_SIZE, response_hash_buffer};
  if (pinweaver_eal_sha256_final(&hash, response_hash_buffer))
    return TSS_RC_HASH_ERROR;

  if (!p->authorization_delegate) {
    return TSS_RC_AUTHORIZATION_FAILED;
  }
  if (tss_CheckResponseAuthorization(p->authorization_delegate,
      &response_hash, &authorization_section_bytes)) {
    return TSS_RC_AUTHORIZATION_FAILED;
  }

  if (p->decrypt_first) {
    TSS_SRC_DATA_BUF first_param =
        {p->buffer.size, p->buffer.buffer};
    UINT16 first_param_size;
    rc = tss_Parse_UINT16(&first_param, &first_param_size, NULL);
    if (rc != TPM_RC_SUCCESS) {
      return rc;
    }
    if (first_param_size > first_param.size) {
      return TPM_RC_INSUFFICIENT;
    }
    first_param.size = first_param_size;
    if (tss_DecryptResponseParameter(p->authorization_delegate, &first_param)) {
      return TSS_RC_ENCRYPTION_FAILED;
    }
  }
  return TPM_RC_SUCCESS;
}

#define RSP_PROCESS_START(cc, decrypt_first) \
  rsp_processor_t p; \
  do { \
    TPM_RC rc = rsp_process_start(&p, response, cc, decrypt_first, \
                                  authorization_delegate); \
    if (rc != TPM_RC_SUCCESS) { \
      return rc; \
    } \
  } while(0)

#define RSP_PROCESS_AUTH_BLOCK() \
  do { \
    TPM_RC rc = rsp_process_auth_block(&p); \
    if (rc != TPM_RC_SUCCESS) { \
      PINWEAVER_EAL_INFO("Failed to process auth block"); \
      return rc; \
    } \
  } while(0)

#define RSP_PROCESS_FIELD(field, type) \
  do { \
    TPM_RC rc = tss_Parse_##type(&p.buffer, field, NULL); \
    if (rc != TPM_RC_SUCCESS) { \
      PINWEAVER_EAL_INFO("Failed to parse %s %s", #type, #field); \
      return rc; \
    } \
  } while(0)

#define RSP_PROCESS_FINISH() \
  return TPM_RC_SUCCESS

static TPM_RC tss_SerializeCommand_StartAuthSession(
    const TPMI_DH_OBJECT tpm_key,
    const TPM2B_NAME* tpm_key_name,
    const TPMI_DH_ENTITY bind,
    const TPM2B_NAME* bind_name,
    const TPM2B_NONCE* nonce_caller,
    const TPM2B_ENCRYPTED_SECRET* encrypted_salt,
    const TPM_SE session_type,
    const TPMT_SYM_DEF* symmetric,
    const TPMI_ALG_HASH auth_hash,
    TSS_DST_DATA_BUF* serialized_command,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  CMD_PROCESS_START(TPM_CC_StartAuthSession, true, true);
  CMD_PROCESS_HANDLE(tpm_key);
  CMD_PROCESS_HANDLE(bind);
  CMD_PROCESS_PARAM_SECTION();
  CMD_PROCESS_PTR(nonce_caller, TPM2B_NONCE);
  CMD_PROCESS_PTR(encrypted_salt, TPM2B_ENCRYPTED_SECRET);
  CMD_PROCESS_VAL(session_type, TPM_SE);
  CMD_PROCESS_PTR(symmetric, TPMT_SYM_DEF);
  CMD_PROCESS_VAL(auth_hash, TPMI_ALG_HASH);
  CMD_PROCESS_FINISH();
}

static TPM_RC tss_ParseResponse_StartAuthSession(
    const TSS_SRC_DATA_BUF* response,
    TPMI_SH_AUTH_SESSION* session_handle,
    TPM2B_NONCE* nonce_tpm,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  RSP_PROCESS_START(TPM_CC_StartAuthSession, true);
  RSP_PROCESS_FIELD(session_handle, TPMI_SH_AUTH_SESSION);
  RSP_PROCESS_AUTH_BLOCK();
  RSP_PROCESS_FIELD(nonce_tpm, TPM2B_NONCE);
  RSP_PROCESS_FINISH();
}

TPM_RC tss_StartAuthSession(
    const TPMI_DH_OBJECT tpm_key,
    const TPM2B_NAME* tpm_key_name,
    const TPMI_DH_ENTITY bind,
    const TPM2B_NAME* bind_name,
    const TPM2B_NONCE* nonce_caller,
    const TPM2B_ENCRYPTED_SECRET* encrypted_salt,
    const TPM_SE session_type,
    const TPMT_SYM_DEF* symmetric,
    const TPMI_ALG_HASH auth_hash,
    TPMI_SH_AUTH_SESSION* session_handle,
    TPM2B_NONCE* nonce_tpm,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  TSS_DST_DATA_BUF command = {PINWEAVER_TSS_MAX_MESSAGE_SIZE, 0, g_message};
  TPM_RC rc = tss_SerializeCommand_StartAuthSession(
      tpm_key, tpm_key_name, bind, bind_name, nonce_caller, encrypted_salt,
      session_type, symmetric, auth_hash, &command, authorization_delegate);
  if (rc != TPM_RC_SUCCESS) {
    return rc;
  }
  TSS_SRC_DATA_BUF response;
  SendCommandAndWait(&command, &response);
  rc = tss_ParseResponse_StartAuthSession(&response, session_handle, nonce_tpm,
                                      authorization_delegate);
  return rc;
}

static TPM_RC tss_SerializeCommand_ReadPublic(
    const TPMI_DH_OBJECT object_handle,
    const TPM2B_NAME* object_handle_name,
    TSS_DST_DATA_BUF* serialized_command,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  CMD_PROCESS_START(TPM_CC_ReadPublic, false, true);
  CMD_PROCESS_HANDLE(object_handle);
  CMD_PROCESS_FINISH();
}

static TPM_RC tss_ParseResponse_ReadPublic(
    const TSS_SRC_DATA_BUF* response,
    TPM2B_PUBLIC* out_public,
    TPM2B_NAME* name,
    TPM2B_NAME* qualified_name,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  RSP_PROCESS_START(TPM_CC_ReadPublic, true);
  RSP_PROCESS_AUTH_BLOCK();
  RSP_PROCESS_FIELD(out_public, TPM2B_PUBLIC);
  RSP_PROCESS_FIELD(name, TPM2B_NAME);
  RSP_PROCESS_FIELD(qualified_name, TPM2B_NAME);
  RSP_PROCESS_FINISH();
}

TPM_RC tss_ReadPublic(const TPMI_DH_OBJECT object_handle,
                  const TPM2B_NAME* object_handle_name,
                  TPM2B_PUBLIC* out_public,
                  TPM2B_NAME* name,
                  TPM2B_NAME* qualified_name,
                  TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  TSS_DST_DATA_BUF command = {PINWEAVER_TSS_MAX_MESSAGE_SIZE, 0, g_message};
  TPM_RC rc = tss_SerializeCommand_ReadPublic(object_handle, object_handle_name,
                                          &command, authorization_delegate);
  if (rc != TPM_RC_SUCCESS) {
    return rc;
  }
  TSS_SRC_DATA_BUF response;
  SendCommandAndWait(&command, &response);

  rc = tss_ParseResponse_ReadPublic(&response, out_public, name, qualified_name,
                                authorization_delegate);
  return rc;
}

static TPM_RC tss_SerializeCommand_PolicyCommandCode(
    const TPMI_SH_POLICY policy_session,
    const TPM2B_NAME* policy_session_name,
    const TPM_CC code,
    TSS_DST_DATA_BUF* serialized_command,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  CMD_PROCESS_START(TPM_CC_PolicyCommandCode, false, false);
  CMD_PROCESS_HANDLE(policy_session);
  CMD_PROCESS_PARAM_SECTION();
  CMD_PROCESS_VAL(code, TPM_CC);
  CMD_PROCESS_FINISH();
}

static TPM_RC tss_ParseResponse_PolicyCommandCode(
    const TSS_SRC_DATA_BUF* response,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  RSP_PROCESS_START(TPM_CC_PolicyCommandCode, false);
  RSP_PROCESS_AUTH_BLOCK();
  RSP_PROCESS_FINISH();
}

TPM_RC tss_PolicyCommandCode(
    const TPMI_SH_POLICY policy_session,
    const TPM2B_NAME* policy_session_name,
    const TPM_CC code,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  TSS_DST_DATA_BUF command = {PINWEAVER_TSS_MAX_MESSAGE_SIZE, 0, g_message};
  TPM_RC rc = tss_SerializeCommand_PolicyCommandCode(
      policy_session, policy_session_name, code, &command,
      authorization_delegate);
  if (rc != TPM_RC_SUCCESS) {
    return rc;
  }
  TSS_SRC_DATA_BUF response;
  SendCommandAndWait(&command, &response);

  rc = tss_ParseResponse_PolicyCommandCode(&response, authorization_delegate);
  return rc;
}

static TPM_RC tss_SerializeCommand_PolicyAuthValue(
    const TPMI_SH_POLICY policy_session,
    const TPM2B_NAME* policy_session_name,
    TSS_DST_DATA_BUF* serialized_command,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  CMD_PROCESS_START(TPM_CC_PolicyAuthValue, false, false);
  CMD_PROCESS_HANDLE(policy_session);
  CMD_PROCESS_FINISH();
}

static TPM_RC tss_ParseResponse_PolicyAuthValue(
    const TSS_SRC_DATA_BUF* response,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  RSP_PROCESS_START(TPM_CC_PolicyAuthValue, false);
  RSP_PROCESS_AUTH_BLOCK();
  RSP_PROCESS_FINISH();
}

TPM_RC tss_PolicyAuthValue(const TPMI_SH_POLICY policy_session,
                       const TPM2B_NAME* policy_session_name,
                       TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  TSS_DST_DATA_BUF command = {PINWEAVER_TSS_MAX_MESSAGE_SIZE, 0, g_message};
  TPM_RC rc = tss_SerializeCommand_PolicyAuthValue(
      policy_session, policy_session_name, &command, authorization_delegate);
  if (rc != TPM_RC_SUCCESS) {
    return rc;
  }
  TSS_SRC_DATA_BUF response;
  SendCommandAndWait(&command, &response);

  rc = tss_ParseResponse_PolicyAuthValue(&response, authorization_delegate);
  return rc;
}

static TPM_RC tss_SerializeCommand_FlushContext(
    const TPMI_DH_CONTEXT flush_handle,
    TSS_DST_DATA_BUF* serialized_command,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  CMD_PROCESS_START(TPM_CC_FlushContext, false, false);
  CMD_PROCESS_PARAM_SECTION();
  CMD_PROCESS_VAL(flush_handle, TPMI_DH_CONTEXT);
  CMD_PROCESS_FINISH();
}

static TPM_RC tss_ParseResponse_FlushContext(
    const TSS_SRC_DATA_BUF* response,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  RSP_PROCESS_START(TPM_CC_FlushContext, false);
  RSP_PROCESS_AUTH_BLOCK();
  RSP_PROCESS_FINISH();
}

TPM_RC tss_FlushContext(const TPMI_DH_CONTEXT flush_handle,
                    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  TSS_DST_DATA_BUF command = {PINWEAVER_TSS_MAX_MESSAGE_SIZE, 0, g_message};
  TPM_RC rc = tss_SerializeCommand_FlushContext(flush_handle, &command,
                                            authorization_delegate);
  if (rc != TPM_RC_SUCCESS) {
    return rc;
  }
  TSS_SRC_DATA_BUF response;
  SendCommandAndWait(&command, &response);

  rc = tss_ParseResponse_FlushContext(&response, authorization_delegate);
  return rc;
}

static TPM_RC tss_SerializeCommand_NV_DefineSpace(
    const TPMI_RH_PROVISION auth_handle,
    const TPM2B_NAME* auth_handle_name,
    const TPM2B_AUTH* auth,
    const TPM2B_NV_PUBLIC* public_info,
    TSS_DST_DATA_BUF* serialized_command,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  CMD_PROCESS_START(TPM_CC_NV_DefineSpace, true, false);
  CMD_PROCESS_HANDLE(auth_handle);
  CMD_PROCESS_PARAM_SECTION();
  CMD_PROCESS_PTR(auth, TPM2B_AUTH);
  CMD_PROCESS_PTR(public_info, TPM2B_NV_PUBLIC);
  CMD_PROCESS_FINISH();
}

static TPM_RC tss_ParseResponse_NV_DefineSpace(
    const TSS_SRC_DATA_BUF* response,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  RSP_PROCESS_START(TPM_CC_NV_DefineSpace, false);
  RSP_PROCESS_AUTH_BLOCK();
  RSP_PROCESS_FINISH();
}

TPM_RC tss_NV_DefineSpace(const TPMI_RH_PROVISION auth_handle,
                      const TPM2B_NAME* auth_handle_name,
                      const TPM2B_AUTH* auth,
                      const TPM2B_NV_PUBLIC* public_info,
                      TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  TSS_DST_DATA_BUF command = {PINWEAVER_TSS_MAX_MESSAGE_SIZE, 0, g_message};
  TPM_RC rc = tss_SerializeCommand_NV_DefineSpace(auth_handle, auth_handle_name,
                                              auth, public_info, &command,
                                              authorization_delegate);
  if (rc != TPM_RC_SUCCESS) {
    return rc;
  }
  TSS_SRC_DATA_BUF response;
  SendCommandAndWait(&command, &response);

  rc = tss_ParseResponse_NV_DefineSpace(&response, authorization_delegate);
  return rc;
}

static TPM_RC tss_SerializeCommand_NV_ReadPublic(
    const TPMI_RH_NV_INDEX nv_index,
    const TPM2B_NAME* nv_index_name,
    TSS_DST_DATA_BUF* serialized_command,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  CMD_PROCESS_START(TPM_CC_NV_ReadPublic, false, true);
  CMD_PROCESS_HANDLE(nv_index);
  CMD_PROCESS_FINISH();
}

static TPM_RC tss_ParseResponse_NV_ReadPublic(
    const TSS_SRC_DATA_BUF* response,
    TPM2B_NV_PUBLIC* nv_public,
    TPM2B_NAME* nv_name,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  RSP_PROCESS_START(TPM_CC_NV_ReadPublic, true);
  RSP_PROCESS_AUTH_BLOCK();
  RSP_PROCESS_FIELD(nv_public, TPM2B_NV_PUBLIC);
  RSP_PROCESS_FIELD(nv_name, TPM2B_NAME);
  RSP_PROCESS_FINISH();
}

TPM_RC tss_NV_ReadPublic(const TPMI_RH_NV_INDEX nv_index,
                     const TPM2B_NAME* nv_index_name,
                     TPM2B_NV_PUBLIC* nv_public,
                     TPM2B_NAME* nv_name,
                     TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  TSS_DST_DATA_BUF command = {PINWEAVER_TSS_MAX_MESSAGE_SIZE, 0, g_message};
  TPM_RC rc = tss_SerializeCommand_NV_ReadPublic(nv_index, nv_index_name, &command,
                                             authorization_delegate);
  if (rc != TPM_RC_SUCCESS) {
    return rc;
  }
  TSS_SRC_DATA_BUF response;
  SendCommandAndWait(&command, &response);

  rc = tss_ParseResponse_NV_ReadPublic(&response, nv_public, nv_name,
                                   authorization_delegate);
  return rc;
}

static TPM_RC tss_SerializeCommand_NV_Write(
    const TPMI_RH_NV_AUTH auth_handle,
    const TPM2B_NAME* auth_handle_name,
    const TPMI_RH_NV_INDEX nv_index,
    const TPM2B_NAME* nv_index_name,
    const TPM2B_MAX_NV_BUFFER* data,
    const UINT16 offset,
    TSS_DST_DATA_BUF* serialized_command,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  CMD_PROCESS_START(TPM_CC_NV_Write, true, false);
  CMD_PROCESS_HANDLE(auth_handle);
  CMD_PROCESS_HANDLE(nv_index);
  CMD_PROCESS_PARAM_SECTION();
  CMD_PROCESS_PTR(data, TPM2B_MAX_NV_BUFFER);
  CMD_PROCESS_VAL(offset, UINT16);
  CMD_PROCESS_FINISH();
}

static TPM_RC tss_ParseResponse_NV_Write(
    const TSS_SRC_DATA_BUF* response,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  RSP_PROCESS_START(TPM_CC_NV_Write, false);
  RSP_PROCESS_AUTH_BLOCK();
  RSP_PROCESS_FINISH();
}

TPM_RC tss_NV_Write(const TPMI_RH_NV_AUTH auth_handle,
                const TPM2B_NAME* auth_handle_name,
                const TPMI_RH_NV_INDEX nv_index,
                const TPM2B_NAME* nv_index_name,
                const TPM2B_MAX_NV_BUFFER* data,
                const UINT16 offset,
                TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  TSS_DST_DATA_BUF command = {PINWEAVER_TSS_MAX_MESSAGE_SIZE, 0, g_message};
  TPM_RC rc = tss_SerializeCommand_NV_Write(auth_handle, auth_handle_name, nv_index,
                                        nv_index_name, data, offset, &command,
                                        authorization_delegate);
  if (rc != TPM_RC_SUCCESS) {
    return rc;
  }
  TSS_SRC_DATA_BUF response;
  SendCommandAndWait(&command, &response);

  rc = tss_ParseResponse_NV_Write(&response, authorization_delegate);
  return rc;
}

static TPM_RC tss_SerializeCommand_NV_Read(
    const TPMI_RH_NV_AUTH auth_handle,
    const TPM2B_NAME* auth_handle_name,
    const TPMI_RH_NV_INDEX nv_index,
    const TPM2B_NAME* nv_index_name,
    const UINT16 size,
    const UINT16 offset,
    TSS_DST_DATA_BUF* serialized_command,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  CMD_PROCESS_START(TPM_CC_NV_Read, false, true);
  CMD_PROCESS_HANDLE(auth_handle);
  CMD_PROCESS_HANDLE(nv_index);
  CMD_PROCESS_PARAM_SECTION();
  CMD_PROCESS_VAL(size, UINT16);
  CMD_PROCESS_VAL(offset, UINT16);
  CMD_PROCESS_FINISH();
}

static TPM_RC tss_ParseResponse_NV_Read(
    const TSS_SRC_DATA_BUF* response,
    TPM2B_MAX_NV_BUFFER* data,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  RSP_PROCESS_START(TPM_CC_NV_Read, true);
  RSP_PROCESS_AUTH_BLOCK();
  RSP_PROCESS_FIELD(data, TPM2B_MAX_NV_BUFFER);
  RSP_PROCESS_FINISH();
}

TPM_RC tss_NV_Read(const TPMI_RH_NV_AUTH auth_handle,
               const TPM2B_NAME* auth_handle_name,
               const TPMI_RH_NV_INDEX nv_index,
               const TPM2B_NAME* nv_index_name,
               const UINT16 size,
               const UINT16 offset,
               TPM2B_MAX_NV_BUFFER* data,
               TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  TSS_DST_DATA_BUF command = {PINWEAVER_TSS_MAX_MESSAGE_SIZE, 0, g_message};
  TPM_RC rc = tss_SerializeCommand_NV_Read(auth_handle, auth_handle_name, nv_index,
                                       nv_index_name, size, offset, &command,
                                       authorization_delegate);
  if (rc != TPM_RC_SUCCESS) {
    return rc;
  }
  TSS_SRC_DATA_BUF response;
  SendCommandAndWait(&command, &response);

  rc = tss_ParseResponse_NV_Read(&response, data, authorization_delegate);
  return rc;
}

static TPM_RC tss_SerializeCommand_NV_ChangeAuth(
    const TPMI_RH_NV_INDEX nv_index,
    const TPM2B_NAME* nv_index_name,
    const TPM2B_AUTH* new_auth,
    TSS_DST_DATA_BUF* serialized_command,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  CMD_PROCESS_START(TPM_CC_NV_ChangeAuth, true, false);
  CMD_PROCESS_HANDLE(nv_index);
  CMD_PROCESS_PARAM_SECTION();
  CMD_PROCESS_PTR(new_auth, TPM2B_AUTH);
  CMD_PROCESS_FINISH();
}

static TPM_RC tss_ParseResponse_NV_ChangeAuth(
    const TSS_SRC_DATA_BUF* response,
    TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  RSP_PROCESS_START(TPM_CC_NV_ChangeAuth, false);
  RSP_PROCESS_AUTH_BLOCK();
  RSP_PROCESS_FINISH();
}

TPM_RC tss_NV_ChangeAuth(const TPMI_RH_NV_INDEX nv_index,
                     const TPM2B_NAME* nv_index_name,
                     const TPM2B_AUTH* new_auth,
                     TSS_AUTHORIZATION_DELEGATE* authorization_delegate) {
  TSS_DST_DATA_BUF command = {PINWEAVER_TSS_MAX_MESSAGE_SIZE, 0, g_message};
  TPM_RC rc = tss_SerializeCommand_NV_ChangeAuth(nv_index, nv_index_name, new_auth,
                                             &command, authorization_delegate);
  if (rc != TPM_RC_SUCCESS) {
    return rc;
  }
  TSS_SRC_DATA_BUF response;
  SendCommandAndWait(&command, &response);

  rc = tss_ParseResponse_NV_ChangeAuth(&response, authorization_delegate);
  return rc;
}
