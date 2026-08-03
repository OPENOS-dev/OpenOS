// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOCK_TSS_H_
#define MOCK_TSS_H_

extern "C" {
#include <tss.h>
} // extern "C"

#include <gmock/gmock.h>

class TssInterface {
public:
  struct StartAuthSessionParam {
    const TPMI_DH_OBJECT tpm_key;
    const TPM2B_NAME *tpm_key_name;
    const TPMI_DH_ENTITY bind;
    const TPM2B_NAME *bind_name;
    const TPM2B_NONCE *nonce_caller;
    const TPM2B_ENCRYPTED_SECRET *encrypted_salt;
    const TPM_SE session_type;
    const TPMT_SYM_DEF *symmetric;
    const TPMI_ALG_HASH auth_hash;
  };
  TssInterface() = default;
  virtual ~TssInterface() = default;
  virtual TPM_RC
  StartAuthSession(const StartAuthSessionParam &param,
                   TPMI_SH_AUTH_SESSION *session_handle, TPM2B_NONCE *nonce_tpm,
                   TSS_AUTHORIZATION_DELEGATE *authorization_delegate) = 0;
  virtual TPM_RC
  ReadPublic(const TPMI_DH_OBJECT object_handle,
             const TPM2B_NAME *object_handle_name, TPM2B_PUBLIC *out_public,
             TPM2B_NAME *name, TPM2B_NAME *qualified_name,
             TSS_AUTHORIZATION_DELEGATE *authorization_delegate) = 0;
  virtual TPM_RC
  PolicyCommandCode(const TPMI_SH_POLICY policy_session,
                    const TPM2B_NAME *policy_session_name, const TPM_CC code,
                    TSS_AUTHORIZATION_DELEGATE *authorization_delegate) = 0;
  virtual TPM_RC
  PolicyAuthValue(const TPMI_SH_POLICY policy_session,
                  const TPM2B_NAME *policy_session_name,
                  TSS_AUTHORIZATION_DELEGATE *authorization_delegate) = 0;
  virtual TPM_RC
  FlushContext(const TPMI_DH_CONTEXT flush_handle,
               TSS_AUTHORIZATION_DELEGATE *authorization_delegate) = 0;
  virtual TPM_RC
  NV_DefineSpace(const TPMI_RH_PROVISION auth_handle,
                 const TPM2B_NAME *auth_handle_name, const TPM2B_AUTH *auth,
                 const TPM2B_NV_PUBLIC *public_info,
                 TSS_AUTHORIZATION_DELEGATE *authorization_delegate) = 0;
  virtual TPM_RC
  NV_ReadPublic(const TPMI_RH_NV_INDEX nv_index,
                const TPM2B_NAME *nv_index_name, TPM2B_NV_PUBLIC *nv_public,
                TPM2B_NAME *nv_name,
                TSS_AUTHORIZATION_DELEGATE *authorization_delegate) = 0;
  virtual TPM_RC
  NV_Write(const TPMI_RH_NV_AUTH auth_handle,
           const TPM2B_NAME *auth_handle_name, const TPMI_RH_NV_INDEX nv_index,
           const TPM2B_NAME *nv_index_name, const TPM2B_MAX_NV_BUFFER *data,
           const UINT16 offset,
           TSS_AUTHORIZATION_DELEGATE *authorization_delegate) = 0;
  virtual TPM_RC
  NV_Read(const TPMI_RH_NV_AUTH auth_handle, const TPM2B_NAME *auth_handle_name,
          const TPMI_RH_NV_INDEX nv_index, const TPM2B_NAME *nv_index_name,
          const UINT16 size, const UINT16 offset, TPM2B_MAX_NV_BUFFER *data,
          TSS_AUTHORIZATION_DELEGATE *authorization_delegate) = 0;
  virtual TPM_RC
  NV_ChangeAuth(const TPMI_RH_NV_INDEX nv_index,
                const TPM2B_NAME *nv_index_name, const TPM2B_AUTH *new_auth,
                TSS_AUTHORIZATION_DELEGATE *authorization_delegate) = 0;
  virtual int InitSessionHmac(TSS_AUTHORIZATION_DELEGATE_HMAC *delegate,
                              TPM_HANDLE session_handle,
                              const TPM2B_NONCE *tpm_nonce,
                              const TPM2B_NONCE *caller_nonce,
                              const TPM2B_DIGEST *salt,
                              const TPM2B_DIGEST *bind_auth_value,
                              bool enable_parameter_encryption) = 0;
  virtual TPM_RC Serialize_TPMS_NV_PUBLIC(const TPMS_NV_PUBLIC *value,
                                          TSS_DST_DATA_BUF *buffer) = 0;
  virtual TPM_RC Serialize_TPMS_ECC_POINT(const TPMS_ECC_POINT *value,
                                          TSS_DST_DATA_BUF *buffer) = 0;
};

class MockTssInterface : public TssInterface {
public:
  MockTssInterface();
  ~MockTssInterface() override = default;
  MOCK_METHOD4(StartAuthSession,
               TPM_RC(const StartAuthSessionParam &param,
                      TPMI_SH_AUTH_SESSION *session_handle,
                      TPM2B_NONCE *nonce_tpm,
                      TSS_AUTHORIZATION_DELEGATE *authorization_delegate));
  MOCK_METHOD6(ReadPublic,
               TPM_RC(const TPMI_DH_OBJECT object_handle,
                      const TPM2B_NAME *object_handle_name,
                      TPM2B_PUBLIC *out_public, TPM2B_NAME *name,
                      TPM2B_NAME *qualified_name,
                      TSS_AUTHORIZATION_DELEGATE *authorization_delegate));
  MOCK_METHOD4(PolicyCommandCode,
               TPM_RC(const TPMI_SH_POLICY policy_session,
                      const TPM2B_NAME *policy_session_name, const TPM_CC code,
                      TSS_AUTHORIZATION_DELEGATE *authorization_delegate));
  MOCK_METHOD3(PolicyAuthValue,
               TPM_RC(const TPMI_SH_POLICY policy_session,
                      const TPM2B_NAME *policy_session_name,
                      TSS_AUTHORIZATION_DELEGATE *authorization_delegate));
  MOCK_METHOD2(FlushContext,
               TPM_RC(const TPMI_DH_CONTEXT flush_handle,
                      TSS_AUTHORIZATION_DELEGATE *authorization_delegate));
  MOCK_METHOD5(NV_DefineSpace,
               TPM_RC(const TPMI_RH_PROVISION auth_handle,
                      const TPM2B_NAME *auth_handle_name,
                      const TPM2B_AUTH *auth,
                      const TPM2B_NV_PUBLIC *public_info,
                      TSS_AUTHORIZATION_DELEGATE *authorization_delegate));
  MOCK_METHOD5(NV_ReadPublic,
               TPM_RC(const TPMI_RH_NV_INDEX nv_index,
                      const TPM2B_NAME *nv_index_name,
                      TPM2B_NV_PUBLIC *nv_public, TPM2B_NAME *nv_name,
                      TSS_AUTHORIZATION_DELEGATE *authorization_delegate));
  MOCK_METHOD7(NV_Write,
               TPM_RC(const TPMI_RH_NV_AUTH auth_handle,
                      const TPM2B_NAME *auth_handle_name,
                      const TPMI_RH_NV_INDEX nv_index,
                      const TPM2B_NAME *nv_index_name,
                      const TPM2B_MAX_NV_BUFFER *data, const UINT16 offset,
                      TSS_AUTHORIZATION_DELEGATE *authorization_delegate));
  MOCK_METHOD8(NV_Read,
               TPM_RC(const TPMI_RH_NV_AUTH auth_handle,
                      const TPM2B_NAME *auth_handle_name,
                      const TPMI_RH_NV_INDEX nv_index,
                      const TPM2B_NAME *nv_index_name, const UINT16 size,
                      const UINT16 offset, TPM2B_MAX_NV_BUFFER *data,
                      TSS_AUTHORIZATION_DELEGATE *authorization_delegate));
  MOCK_METHOD4(NV_ChangeAuth,
               TPM_RC(const TPMI_RH_NV_INDEX nv_index,
                      const TPM2B_NAME *nv_index_name,
                      const TPM2B_AUTH *new_auth,
                      TSS_AUTHORIZATION_DELEGATE *authorization_delegate));
  MOCK_METHOD7(InitSessionHmac,
               int(TSS_AUTHORIZATION_DELEGATE_HMAC *delegate,
                   TPM_HANDLE session_handle, const TPM2B_NONCE *tpm_nonce,
                   const TPM2B_NONCE *caller_nonce, const TPM2B_DIGEST *salt,
                   const TPM2B_DIGEST *bind_auth_value,
                   bool enable_parameter_encryption));
  MOCK_METHOD2(Serialize_TPMS_NV_PUBLIC,
               TPM_RC(const TPMS_NV_PUBLIC *value, TSS_DST_DATA_BUF *buffer));
  MOCK_METHOD2(Serialize_TPMS_ECC_POINT,
               TPM_RC(const TPMS_ECC_POINT *value, TSS_DST_DATA_BUF *buffer));
};

TssInterface *GetTss();
void SetTss(TssInterface *interface);

#endif // MOCK_TSS_H_
