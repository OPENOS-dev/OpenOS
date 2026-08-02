// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MINI_TRUNKS_HMAC_AUTHORIZATION_DELEGATE_H_
#define MINI_TRUNKS_HMAC_AUTHORIZATION_DELEGATE_H_

#include "tss_serde.h"

// TSS_AUTHORIZATION_DELEGATE is an interface passed to TPM commands. The delegate
// takes care of providing the authorization data for commands and verifying
// authorization data for responses. It also handles parameter encryption for
// commands and parameter decryption for responses.
typedef struct TSS_AUTHORIZATION_DELEGATE_HMAC {
  TPM_HANDLE session_handle_;
  TPM2B_NONCE caller_nonce_;
  TPM2B_NONCE tpm_nonce_;
  bool is_parameter_encryption_enabled_;
  bool nonce_generated_;
  TPM2B_DIGEST session_key_;
  TPM2B_DIGEST entity_authorization_value_;
  TPM2B_DIGEST future_authorization_value_;
  bool future_authorization_value_set_;
  // This boolean flag determines if the entity_authorization_value_ is needed
  // when computing the hmac_key to create the authorization hmac. Defaults
  // to false, but policy sessions may set this flag to true.
  bool use_entity_authorization_for_encryption_only_;
} TSS_AUTHORIZATION_DELEGATE_HMAC;

// Provides authorization data for a command which has a cpHash value of
// |command_hash|. The availability of encryption for the command is indicated
// by |is_*_parameter_encryption_possible|. On success, |authorization| is
// populated with the exact octets for the Authorization Area of the command.
// Returns 0 on success.
int tss_GetCommandAuthorizationHmac(
    TSS_AUTHORIZATION_DELEGATE_HMAC* delegate,
    const TSS_SRC_DATA_BUF* command_hash,
    bool is_command_parameter_encryption_possible,
    bool is_response_parameter_encryption_possible,
    TSS_DST_DATA_BUF* authorization);

// Checks authorization data for a response which has a rpHash value of
// |response_hash|. The exact octets from the Authorization Area of the
// response are given in |authorization|. Returns true iff the authorization
// is valid.
int tss_CheckResponseAuthorizationHmac(
    TSS_AUTHORIZATION_DELEGATE_HMAC* delegate,
    const TSS_SRC_DATA_BUF* response_hash,
    const TSS_SRC_DATA_BUF* authorization);

// Encrypts |parameter| if encryption is enabled. Returns true on success.
int tss_EncryptCommandParameterHmac(
    TSS_AUTHORIZATION_DELEGATE_HMAC* delegate,
    TSS_SRC_DATA_BUF* parameter);

// Decrypts |parameter| if encryption is enabled. Returns true on success.
int tss_DecryptResponseParameterHmac(
    TSS_AUTHORIZATION_DELEGATE_HMAC* delegate,
    TSS_SRC_DATA_BUF* parameter);

// Returns the current TPM-generated nonce that is associated with the
// authorization session. Returns true on success.
int tss_GetTpmNonceHmac(
    TSS_AUTHORIZATION_DELEGATE_HMAC* delegate,
    TSS_DST_DATA_BUF* nonce);

// This function is called with the return data of |StartAuthSession|. It
// will initialize the session to start providing auth information. It can
// only be called once per delegate, and must be called before the delegate
// is used for any operation. The boolean arg |enable_parameter_encryption|
// specifies if parameter encryption should be enabled for this delegate.
// |salt| and |bind_auth_value| specify the injected auth values into this
// delegate.
int tss_InitSessionHmac(
    TSS_AUTHORIZATION_DELEGATE_HMAC* delegate,
    TPM_HANDLE session_handle,
    const TPM2B_NONCE* tpm_nonce,
    const TPM2B_NONCE* caller_nonce,
    const TPM2B_DIGEST* salt,
    const TPM2B_DIGEST* bind_auth_value,
    bool enable_parameter_encryption);

#endif  // MINI_TRUNKS_HMAC_AUTHORIZATION_DELEGATE_H_
