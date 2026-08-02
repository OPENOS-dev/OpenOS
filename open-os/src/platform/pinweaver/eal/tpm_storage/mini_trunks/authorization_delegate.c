// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "authorization_delegate.h"

// Provides authorization data for a command which has a cpHash value of
// |command_hash|. The availability of encryption for the command is indicated
// by |is_*_parameter_encryption_possible|. On success, |authorization| is
// populated with the exact octets for the Authorization Area of the command.
// Returns true on success.
int tss_GetCommandAuthorization(
    TSS_AUTHORIZATION_DELEGATE* delegate,
    const TSS_SRC_DATA_BUF* command_hash,
    bool is_command_parameter_encryption_possible,
    bool is_response_parameter_encryption_possible,
    TSS_DST_DATA_BUF* authorization) {
  return tss_GetCommandAuthorizationHmac(
      &delegate->hmac,
      command_hash, 
      is_command_parameter_encryption_possible,
      is_response_parameter_encryption_possible,
      authorization);
}

// Checks authorization data for a response which has a rpHash value of
// |response_hash|. The exact octets from the Authorization Area of the
// response are given in |authorization|. Returns true iff the authorization
// is valid.
int tss_CheckResponseAuthorization(
    TSS_AUTHORIZATION_DELEGATE* delegate,
    const TSS_SRC_DATA_BUF* response_hash,
    const TSS_SRC_DATA_BUF* authorization) {
  return tss_CheckResponseAuthorizationHmac(
      &delegate->hmac, response_hash, authorization);
}

// Encrypts |parameter| if encryption is enabled. Returns true on success.
int tss_EncryptCommandParameter(
    TSS_AUTHORIZATION_DELEGATE* delegate,
    TSS_SRC_DATA_BUF* parameter) {
  return tss_EncryptCommandParameterHmac(&delegate->hmac, parameter);
}

// Decrypts |parameter| if encryption is enabled. Returns true on success.
int tss_DecryptResponseParameter(
    TSS_AUTHORIZATION_DELEGATE* delegate,
    TSS_SRC_DATA_BUF* parameter) {
  return tss_DecryptResponseParameterHmac(&delegate->hmac, parameter);
}

// Returns the current TPM-generated nonce that is associated with the
// authorization session. Returns true on success.
int tss_GetTpmNonce(
    TSS_AUTHORIZATION_DELEGATE* delegate,
    TSS_DST_DATA_BUF* nonce) {
  return tss_GetTpmNonceHmac(&delegate->hmac, nonce);
}
