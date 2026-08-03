// Copyright 2021 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <memory>

#include <gtest/gtest.h>

#include "mock_eal.h"
#include "mock_tss.h"

extern "C" {
#include "tpm_storage.h"
#include "tpm_storage_internal.h"
}

using testing::_;
using testing::ElementsAre;
using testing::ElementsAreArray;
using testing::Invoke;
using testing::Mock;
using testing::NiceMock;
using testing::Return;

// Mocked salting key
constexpr TPM2B_ECC_PARAMETER kSaltingX = {.size = 32,
                                           .buffer = {
                                               0x70,
                                               0x0A,
                                           }};
constexpr TPM2B_ECC_PARAMETER kSaltingY = {.size = 32,
                                           .buffer = {
                                               0x70,
                                               0x0B,
                                           }};
constexpr TPMS_ECC_POINT kPubSaltingKey = {.x = kSaltingX, .y = kSaltingY};

class Test : public testing::Test {
public:
  struct NVLogState {
    bool written;
    uint32_t counter;
  };

  struct OPStats {
    size_t total = 0;
    size_t succeeded = 0;
  };

  struct NVStats {
    OPStats reads;
    OPStats writes;
    OPStats defines;
    OPStats change_auth;
  };

  struct NVForcedRes {
    TPM_RC rc;
    int count; // -1 means indefinite
  };

  struct OPRes {
    bool fail = false;
    int count = 0; // -1 means indefinite

    void set_fail_for(int value) {
      fail = true;
      count = value;
    }
  };

  Test();
  void SetUp() override;

protected:
  void GlobalReset();
  void SetUpNV();

  uint16_t ExpectedNVDataSize(TPM_HANDLE handle);
  TPM_RC WriteNV(TPM_HANDLE handle, std::string data);
  TPM_RC ReadNV(TPM_HANDLE handle, std::string *data);
  TPM_RC GetNVPublic(TPM_HANDLE handle, TPMS_NV_PUBLIC *pub);
  TPM_RC DefineNV(TPM_HANDLE handle, TPMS_NV_PUBLIC pub);
  TPM_RC ChangeAuth(TPM_HANDLE handle);

  TPM_RC ApplyForcedResult(TPM_HANDLE handle,
                           std::map<TPM_HANDLE, NVForcedRes> &res_map);
  void SetTreeDescrRestartCount(uint32_t count);
  void SetTreeDescr(pw_long_term_storage_t *tree_data);
  void SetLogEntry(int n, uint32_t counter, const uint8_t hash[32]);

  void CreateLogInNV(TPM_HANDLE handle, uint32_t counter, bool written = true);
  void DeleteLogInNV(TPM_HANDLE handle);
  void CreateTreeInNV(bool written = true);
  void DeleteTreeInNV();

  void SetLogStateInNV(const NVLogState entries[MAX_LOG_ENTRIES],
                       const NVLogState persist = {false, 0});

  NiceMock<MockEalInterface> eal_;
  NiceMock<MockTssInterface> tss_;

  bool bad_tpm_key_ = false;
  bool tpm_key_set_ = true;
  bool tpm_key_committed_ = true;
  OPRes aes_result_[2];

  std::map<TPM_HANDLE, NVStats> nv_stats_;
  std::map<TPM_HANDLE, std::string> nv_spaces_;
  std::map<TPM_HANDLE, TPMS_NV_PUBLIC> nv_pub_areas_;
  std::map<TPM_HANDLE, NVForcedRes> force_write_results_;
  std::map<TPM_HANDLE, NVForcedRes> force_read_results_;
  std::map<TPM_HANDLE, NVForcedRes> force_define_results_;
  std::map<TPM_HANDLE, NVForcedRes> force_change_auth_results_;
};

Test::Test() {
  SetEal(&eal_);
  SetTss(&tss_);
}

void Test::SetUp() {
  GlobalReset();
  SetUpNV();
}

// Set default NV behavior
void Test::SetUpNV() {
  ON_CALL(tss_, ReadPublic(_, _, _, _, _, _))
      .WillByDefault(Invoke(
          [this](const TPMI_DH_OBJECT object_handle,
                 const TPM2B_NAME *object_handle_name, TPM2B_PUBLIC *out_public,
                 TPM2B_NAME *name, TPM2B_NAME *qualified_name,
                 TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
            out_public->size = sizeof(out_public->public_area);
            memset(&out_public->public_area, 0,
                   sizeof(out_public->public_area));
            out_public->public_area.type = TPM_ALG_ECC;
            out_public->public_area.parameters.ecc_detail.curve_id =
                TPM_ECC_NIST_P256;
            out_public->public_area.object_attributes =
                REQUIRED_SALTING_KEY_ATTR;
            out_public->public_area.unique.ecc = kPubSaltingKey;

            return TPM_RC_SUCCESS;
          }));
  ON_CALL(tss_, NV_Read(_, _, _, _, _, _, _, _))
      .WillByDefault(
          Invoke([this](const TPMI_RH_NV_AUTH auth_handle,
                        const TPM2B_NAME *auth_handle_name,
                        const TPMI_RH_NV_INDEX nv_index,
                        const TPM2B_NAME *nv_index_name, const UINT16 size,
                        const UINT16 offset, TPM2B_MAX_NV_BUFFER *data,
                        TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
            EXPECT_EQ(offset, 0);
            std::string data_str;
            TPM_RC rc = ReadNV(nv_index, &data_str);
            if (rc) {
              return rc;
            }
            size_t read_size = std::min(data_str.size(), sizeof(data->buffer));
            read_size = std::min(read_size, (size_t)size);
            memcpy(data->buffer, data_str.data(), read_size);
            data->size = read_size;
            return TPM_RC_SUCCESS;
          }));
  ON_CALL(tss_, NV_Write(_, _, _, _, _, _, _))
      .WillByDefault(
          Invoke([this](const TPMI_RH_NV_AUTH auth_handle,
                        const TPM2B_NAME *auth_handle_name,
                        const TPMI_RH_NV_INDEX nv_index,
                        const TPM2B_NAME *nv_index_name,
                        const TPM2B_MAX_NV_BUFFER *data, const UINT16 offset,
                        TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
            EXPECT_EQ(offset, 0);
            size_t write_size =
                std::min((size_t)data->size, sizeof(data->buffer));
            std::string data_str((char *)data->buffer, write_size);
            return WriteNV(nv_index, data_str);
          }));
  ON_CALL(tss_, NV_ReadPublic(_, _, _, _, _))
      .WillByDefault(
          Invoke([this](const TPMI_RH_NV_INDEX nv_index,
                        const TPM2B_NAME *nv_index_name,
                        TPM2B_NV_PUBLIC *nv_public, TPM2B_NAME *nv_name,
                        TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
            TPMS_NV_PUBLIC pub;
            TPM_RC rc = GetNVPublic(nv_index, &pub);
            if (rc) {
              return rc;
            }
            memcpy(&nv_public->nv_public, &pub, sizeof(pub));
            nv_public->size = sizeof(pub);
            EXPECT_EQ(calc_nv_name(&pub, nv_name), 0);
            return TPM_RC_SUCCESS;
          }));
  ON_CALL(tss_, NV_DefineSpace(_, _, _, _, _))
      .WillByDefault(Invoke(
          [this](const TPMI_RH_PROVISION auth_handle,
                 const TPM2B_NAME *auth_handle_name, const TPM2B_AUTH *auth,
                 const TPM2B_NV_PUBLIC *public_info,
                 TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
            TPMS_NV_PUBLIC pub;
            EXPECT_EQ(public_info->size, sizeof(pub));
            memcpy(&pub, &public_info->nv_public, sizeof(pub));
            return DefineNV(pub.nv_index, pub);
          }));
  ON_CALL(tss_, NV_ChangeAuth(_, _, _, _))
      .WillByDefault(Invoke(
          [this](const TPMI_RH_NV_INDEX nv_index,
                 const TPM2B_NAME *nv_index_name, const TPM2B_AUTH *new_auth,
                 TSS_AUTHORIZATION_DELEGATE *authorization_delegate) {
            return ChangeAuth(nv_index);
          }));
  ON_CALL(eal_, get_tpm_key_hash(_, _))
      .WillByDefault(Invoke([this](uint8_t tpm_key_hash[32], bool *committed) {
        *committed = tpm_key_committed_ ? 1 : 0;
        if (!tpm_key_set_)
          return 1;
        if (bad_tpm_key_) {
          memset(tpm_key_hash, MockEalInterface::kHashByte + 1, 32);
        } else {
          memset(tpm_key_hash, MockEalInterface::kHashByte, 32);
        }
        return 0;
      }));
  ON_CALL(eal_, generate_ecdh_points(_, _, _))
      .WillByDefault(Invoke([this](const TPMS_ECC_POINT *pub_key,
                                   TPMS_ECC_POINT *ephemeral_point,
                                   TPMS_ECC_POINT *z_point) {
        memset(ephemeral_point, 0, sizeof(TPMS_ECC_POINT));
        memset(z_point, 0, sizeof(TPMS_ECC_POINT));
        ephemeral_point->x.size = 32;
        ephemeral_point->y.size = 32;
        z_point->x.size = 32;
        z_point->y.size = 32;
        return 0;
      }));
  ON_CALL(eal_, aes256_ctr(_, _, _, _, _, _))
      .WillByDefault(Invoke(
          [this](const void *key, size_t key_size, /* in bytes */
                 const void *iv, const void *data, size_t size, void *res) {
            // Mock encryption: do nothing
            int kind = -1;
            if (key == g_data_key[0]) {
              kind = 0;
            } else if (key == g_data_key[1]) {
              kind = 1;
            }
            if (kind >= 0) {
              if (aes_result_[kind].fail && aes_result_[kind].count) {
                if (aes_result_[kind].count > 0)
                  aes_result_[kind].count--;
                return -1;
              }
            }
            memcpy(res, data, size);
            return 0;
          }));
}

// Reset global status to defaults
void Test::GlobalReset() {
  g_storage_state = TPMSS_NOT_STARTED;
  g_auth_session = {
      0,
  };
  g_tree_descriptor_filled = false;
  for (int kind : {0, 1}) {
    g_auth_value_obtained[kind] = false;
    g_device_key_obtained[kind] = false;
    g_data_key_obtained[kind] = false;
  }
  g_max_counter = 0;
}

uint16_t Test::ExpectedNVDataSize(TPM_HANDLE handle) {
  if (handle == TREE_DESCRIPTOR_HANDLE) {
    return sizeof(tpm_storage_tree_descriptor_t);
  } else {
    return sizeof(tpm_storage_log_entry_t);
  }
}

TPM_RC Test::ApplyForcedResult(TPM_HANDLE handle,
                               std::map<TPM_HANDLE, NVForcedRes> &res_map) {
  auto res = res_map.find(handle);
  if (res != res_map.end()) {
    NVForcedRes nv_res = res->second;
    if (nv_res.count == 0) {
      nv_res.rc = TPM_RC_SUCCESS;
    } else if (nv_res.count > 0) {
      nv_res.count--;
    }
    if (nv_res.count == 0) {
      res_map.erase(handle);
    } else {
      res_map[handle].count = nv_res.count;
    }
    return nv_res.rc;
  }
  return TPM_RC_SUCCESS;
}

void Test::SetTreeDescrRestartCount(uint32_t count) {
  tpm_storage_tree_descriptor_t tree;
  memset(&tree, 0, sizeof(tree));
  tree.version = 0;
  tree.restart_count = count;
  std::string data((char *)&tree, sizeof(tree));
  nv_spaces_[TREE_DESCRIPTOR_HANDLE] = data;
}

void Test::SetTreeDescr(pw_long_term_storage_t *tree_data) {
  tpm_storage_tree_descriptor_t tree;
  memset(&tree, 0, sizeof(tree));
  tree.version = 0;
  tree.restart_count = 1;
  memcpy(&tree.descriptor, tree_data, sizeof(tree.descriptor));
  std::string data((char *)&tree, sizeof(tree));
  nv_spaces_[TREE_DESCRIPTOR_HANDLE] = data;
}

void Test::SetLogEntry(int n, uint32_t counter, const uint8_t hash[32]) {
  tpm_storage_log_entry_t entry;
  entry.version = 0;
  entry.counter = counter;
  memset(entry.iv, 0xFF, sizeof(entry.iv));

  log_sensitive_data_t plaintext;
  memset(&plaintext, 0xA5, sizeof(plaintext));
  memcpy(plaintext.entry.root, hash, 32);
  calc_log_sensitive_data_hash(&plaintext, plaintext.hash);
  // Assumes mocked aes256_ctr implementation
  memcpy(entry.encrypted_data, &plaintext, sizeof(log_sensitive_data_t));

  std::string data((char *)&entry, sizeof(entry));
  nv_spaces_[LOG_ENTRY_FIRST_HANDLE + n] = data;
}

TPM_RC Test::WriteNV(TPM_HANDLE handle, std::string data) {
  nv_stats_[handle].writes.total++;
  TPM_RC rc = ApplyForcedResult(handle, force_write_results_);
  if (rc != TPM_RC_SUCCESS) {
    return rc;
  }
  auto pub = nv_pub_areas_.find(handle);
  if (pub == nv_pub_areas_.end()) {
    return TPM_RC_HANDLE + TPM_RC_H + TPM_RC_1;
  }
  nv_spaces_[handle] = data;
  nv_pub_areas_[handle].attributes |= TPMA_NV_WRITTEN;
  nv_stats_[handle].writes.succeeded++;
  return TPM_RC_SUCCESS;
}

TPM_RC Test::ReadNV(TPM_HANDLE handle, std::string *data) {
  nv_stats_[handle].reads.total++;
  TPM_RC rc = ApplyForcedResult(handle, force_read_results_);
  if (rc != TPM_RC_SUCCESS) {
    return rc;
  }
  auto pub = nv_pub_areas_.find(handle);
  if (pub == nv_pub_areas_.end()) {
    return TPM_RC_HANDLE + TPM_RC_H + TPM_RC_1;
  }
  auto elem = nv_spaces_.find(handle);
  if (elem == nv_spaces_.end()) {
    return TPM_RC_NV_UNINITIALIZED;
  }
  *data = elem->second;
  nv_stats_[handle].reads.succeeded++;
  return TPM_RC_SUCCESS;
}

TPM_RC Test::GetNVPublic(TPM_HANDLE handle, TPMS_NV_PUBLIC *pub) {
  auto elem = nv_pub_areas_.find(handle);
  if (elem == nv_pub_areas_.end()) {
    return TPM_RC_HANDLE + TPM_RC_H + TPM_RC_1;
  }
  *pub = elem->second;
  return TPM_RC_SUCCESS;
}

TPM_RC Test::DefineNV(TPM_HANDLE handle, TPMS_NV_PUBLIC pub) {
  nv_stats_[handle].defines.total++;
  TPM_RC rc = ApplyForcedResult(handle, force_define_results_);
  if (rc != TPM_RC_SUCCESS) {
    return rc;
  }
  auto elem = nv_pub_areas_.find(handle);
  if (elem != nv_pub_areas_.end()) {
    return TPM_RC_NV_DEFINED;
  }
  pub.attributes &= ~TPMA_NV_WRITTEN;
  nv_pub_areas_[handle] = pub;
  nv_spaces_.erase(handle);
  nv_stats_[handle].defines.succeeded++;
  return TPM_RC_SUCCESS;
}

TPM_RC Test::ChangeAuth(TPM_HANDLE handle) {
  nv_stats_[handle].change_auth.total++;
  TPM_RC rc = ApplyForcedResult(handle, force_change_auth_results_);
  if (rc != TPM_RC_SUCCESS) {
    return rc;
  }
  nv_stats_[handle].change_auth.succeeded++;
  return TPM_RC_SUCCESS;
}

void Test::CreateLogInNV(TPM_HANDLE handle, uint32_t counter, bool written) {
  TPMS_NV_PUBLIC pub;
  set_nv_public_area(handle, sizeof(tpm_storage_log_entry_t), written, &pub);
  nv_pub_areas_[handle] = pub;

  if (written) {
    tpm_storage_log_entry_t entry;
    entry.version = 0;
    entry.counter = counter;
    memset(entry.iv, 0xFF, sizeof(entry.iv));

    log_sensitive_data_t plaintext;
    memset(&plaintext, 0xA5, sizeof(plaintext));
    calc_log_sensitive_data_hash(&plaintext, plaintext.hash);
    // Assumes mocked aes256_ctr implementation
    memcpy(entry.encrypted_data, &plaintext, sizeof(log_sensitive_data_t));

    std::string data((char *)&entry, sizeof(entry));
    nv_spaces_[handle] = data;
  } else {
    nv_spaces_.erase(handle);
  }
}

void Test::DeleteLogInNV(TPM_HANDLE handle) {
  nv_spaces_.erase(handle);
  nv_pub_areas_.erase(handle);
}

void Test::CreateTreeInNV(bool written) {
  TPMS_NV_PUBLIC pub;
  set_nv_public_area(TREE_DESCRIPTOR_HANDLE,
                     sizeof(tpm_storage_tree_descriptor_t), written, &pub);
  nv_pub_areas_[TREE_DESCRIPTOR_HANDLE] = pub;

  if (written) {
    tpm_storage_tree_descriptor_t tree;
    memset(&tree, 0xFF, sizeof(tree));
    tree.version = 0;
    std::string data((char *)&tree, sizeof(tree));
    nv_spaces_[TREE_DESCRIPTOR_HANDLE] = data;
  } else {
    nv_spaces_.erase(TREE_DESCRIPTOR_HANDLE);
  }
}

void Test::DeleteTreeInNV() {
  nv_spaces_.erase(TREE_DESCRIPTOR_HANDLE);
  nv_pub_areas_.erase(TREE_DESCRIPTOR_HANDLE);
}

void Test::SetLogStateInNV(const NVLogState entries[MAX_LOG_ENTRIES],
                           const NVLogState persist) {
  for (int n = 0; n < MAX_LOG_ENTRIES; ++n) {
    CreateLogInNV(LOG_ENTRY_FIRST_HANDLE + n, entries[n].counter,
                  entries[n].written);
  }
  CreateLogInNV(LOG_ENTRY_PERSIST_HANDLE, persist.counter, persist.written);
}

TEST_F(Test, StartSimpleSuccess) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 1}, {true, 2}, {true, 3}, {true, 4}, {true, 5},
  };
  SetLogStateInNV(state);
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(4, 3, 2, 1, 0));
}

TEST_F(Test, StartSimpleRandomOrderSuccess) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 2}, {true, 4}, {true, 5}, {true, 1}, {true, 3},
  };
  SetLogStateInNV(state);
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(2, 1, 4, 0, 3));
}

TEST_F(Test, StartSimpleReversedSuccess) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 5}, {true, 4}, {true, 3}, {true, 2}, {true, 1},
  };
  SetLogStateInNV(state);
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(0, 1, 2, 3, 4));
}

TEST_F(Test, StartPersistDupFullSuccess) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 1}, {true, 2}, {true, 3}, {true, 4}, {true, 5},
  };
  SetLogStateInNV(state, {true, 1});
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(4, 3, 2, 1, 0));
}

TEST_F(Test, StartPersistExtraFullSuccess) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 1}, {true, 2}, {true, 3}, {true, 4}, {true, 5},
  };
  SetLogStateInNV(state, {true, 6});
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(4, 3, 2, 1, 0));
}

TEST_F(Test, StartPersistDupSuccess) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 1}, {false, 0}, {true, 3}, {true, 4}, {true, 5},
  };
  SetLogStateInNV(state, {true, 4});
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(4, 3, 2, 0, 1));
}

TEST_F(Test, StartDupLogFail) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 1}, {true, 2}, {true, 2}, {true, 4}, {true, 5},
  };
  SetLogStateInNV(state);
  EXPECT_EQ(g_storage_state, TPMSS_NOT_STARTED);
  EXPECT_EQ(pinweaver_eal_storage_start(), -1);
}

TEST_F(Test, StartOneLostSuccess) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 1}, {true, 2}, {true, 3}, {false, 0}, {true, 5},
  };
  SetLogStateInNV(state);
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(4, 2, 1, 0, 3));
}

TEST_F(Test, StartInterruptedInPersistMidSuccess) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 1}, {true, 3}, {false, 0}, {true, 7}, {true, 9},
  };
  SetLogStateInNV(state, {true, 2});
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(4, 3, 1, 2, 0));
  GlobalReset();
  DeleteLogInNV(LOG_ENTRY_PERSIST_HANDLE);
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(4, 3, 1, 2, 0));
}

TEST_F(Test, StartInterruptedInPersistLaterSuccess) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 1}, {true, 3}, {false, 0}, {true, 7}, {true, 9},
  };
  SetLogStateInNV(state, {true, 10});
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(2, 4, 3, 1, 0));
  GlobalReset();
  DeleteLogInNV(LOG_ENTRY_PERSIST_HANDLE);
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(2, 4, 3, 1, 0));
}

TEST_F(Test, StartInterruptedInPersistEarlierSuccess) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 3}, {true, 5}, {false, 0}, {true, 9}, {true, 11},
  };
  SetLogStateInNV(state, {true, 1});
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(4, 3, 1, 0, 2));
  GlobalReset();
  DeleteLogInNV(LOG_ENTRY_PERSIST_HANDLE);
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(4, 3, 1, 0, 2));
}

TEST_F(Test, StartInterruptedFailToContinue) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 3}, {true, 5}, {false, 0}, {true, 9}, {true, 11},
  };
  SetLogStateInNV(state, {true, 7});
  force_write_results_[LOG_ENTRY_FIRST_HANDLE + 2] = {TPM_RC_FAILURE, -1};
  EXPECT_EQ(pinweaver_eal_storage_start(), -1);
  EXPECT_EQ(g_storage_state, TPMSS_NOT_STARTED);
  GlobalReset();
  force_write_results_.erase(LOG_ENTRY_FIRST_HANDLE + 2);
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(4, 3, 2, 1, 0));
}

TEST_F(Test, StartReadFailure) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 3}, {true, 5}, {true, 7}, {true, 9}, {true, 11},
  };
  SetLogStateInNV(state);
  force_read_results_[LOG_ENTRY_FIRST_HANDLE + 2] = {TPM_RC_FAILURE, -1};
  EXPECT_EQ(g_storage_state, TPMSS_NOT_STARTED);
  EXPECT_EQ(pinweaver_eal_storage_start(), -1);
}

TEST_F(Test, StartReadRetriedSuccess) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 1}, {true, 2}, {true, 3}, {true, 4}, {true, 5},
  };
  SetLogStateInNV(state);
  force_read_results_[LOG_ENTRY_FIRST_HANDLE + 2] = {TPM_RC_FAILURE, 1};
  force_read_results_[LOG_ENTRY_FIRST_HANDLE + 0] = {TPM_RC_FAILURE, 2};
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(4, 3, 2, 1, 0));
}

TEST_F(Test, StartReadPersistFailure) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 3}, {true, 5}, {false, 0}, {true, 9}, {true, 11},
  };
  SetLogStateInNV(state, {true, 7});
  force_read_results_[LOG_ENTRY_PERSIST_HANDLE] = {TPM_RC_FAILURE, -1};
  EXPECT_EQ(pinweaver_eal_storage_start(), -1);
}

TEST_F(Test, StartWritePersistFailure) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 3}, {true, 5}, {false, 0}, {true, 9}, {true, 11},
  };
  SetLogStateInNV(state, {true, 7});
  force_write_results_[LOG_ENTRY_FIRST_HANDLE + 2] = {TPM_RC_FAILURE, -1};
  EXPECT_EQ(pinweaver_eal_storage_start(), -1);
  EXPECT_EQ(g_storage_state, TPMSS_NOT_STARTED);
  GlobalReset();
  force_write_results_.erase(LOG_ENTRY_FIRST_HANDLE + 2);
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(4, 3, 2, 1, 0));
}

TEST_F(Test, StartWritePersistRetrySuccess) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 3}, {true, 5}, {false, 0}, {true, 9}, {true, 11},
  };
  SetLogStateInNV(state, {true, 7});
  force_write_results_[LOG_ENTRY_FIRST_HANDLE + 2] = {TPM_RC_FAILURE, 2};
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(4, 3, 2, 1, 0));
}

TEST_F(Test, StartNoTree) {
  EXPECT_EQ(pinweaver_eal_storage_start(), -1);
  EXPECT_EQ(g_storage_state, TPMSS_NOT_INITIALIZED);
}

TEST_F(Test, StartUnwrittenTree) {
  CreateTreeInNV(false);
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(4, 3, 2, 1, 0));
}

TEST_F(Test, ChangeAuthSuccess) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 5}, {true, 9}, {true, 3}, {true, 7}, {true, 11},
  };
  SetLogStateInNV(state);
  force_read_results_[TREE_DESCRIPTOR_HANDLE] = {
      TPM_RC_BAD_AUTH + TPM_RC_S + TPM_RC_1, 1};
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(4, 1, 3, 0, 2));
  EXPECT_EQ(nv_stats_[TREE_DESCRIPTOR_HANDLE].change_auth.total, 1);
  EXPECT_EQ(nv_stats_[TREE_DESCRIPTOR_HANDLE].change_auth.succeeded, 1);
  EXPECT_EQ(nv_stats_[LOG_ENTRY_PERSIST_HANDLE].change_auth.total, 1);
  EXPECT_EQ(nv_stats_[LOG_ENTRY_PERSIST_HANDLE].change_auth.succeeded, 1);
  for (int n = 0; n < MAX_LOG_ENTRIES; ++n) {
    EXPECT_EQ(nv_stats_[LOG_ENTRY_FIRST_HANDLE + n].change_auth.total, 1);
    EXPECT_EQ(nv_stats_[LOG_ENTRY_FIRST_HANDLE + n].change_auth.succeeded, 1);
  }
}

TEST_F(Test, ChangeAuthIgnoreErrors) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 5}, {true, 9}, {true, 3}, {true, 7}, {true, 11},
  };
  SetLogStateInNV(state);
  force_read_results_[TREE_DESCRIPTOR_HANDLE] = {
      TPM_RC_BAD_AUTH + TPM_RC_S + TPM_RC_1, 1};
  force_change_auth_results_[LOG_ENTRY_FIRST_HANDLE] = {TPM_RC_FAILURE, -1};
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(4, 1, 3, 0, 2));
}

TEST_F(Test, DecryptFailure) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 5}, {true, 4}, {true, 3}, {true, 2}, {true, 1},
  };
  SetLogStateInNV(state);
  // Fail decrypt with both keys for one space.
  aes_result_[0].set_fail_for(-1);
  aes_result_[1].set_fail_for(-1);
  EXPECT_EQ(pinweaver_eal_storage_start(), -1);
  EXPECT_EQ(g_storage_state, TPMSS_NOT_STARTED);
}

TEST_F(Test, ReEncryptSuccess) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 5}, {true, 4}, {true, 3}, {true, 2}, {true, 1},
  };
  SetLogStateInNV(state);
  // Fail decrypt for the first 2 spaces with the current key.
  aes_result_[0].set_fail_for(2);
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(0, 1, 2, 3, 4));
  EXPECT_EQ(nv_stats_[LOG_ENTRY_FIRST_HANDLE + 0].writes.total, 1);
  EXPECT_EQ(nv_stats_[LOG_ENTRY_FIRST_HANDLE + 0].writes.succeeded, 1);
  EXPECT_EQ(nv_stats_[LOG_ENTRY_FIRST_HANDLE + 1].writes.total, 1);
  EXPECT_EQ(nv_stats_[LOG_ENTRY_FIRST_HANDLE + 1].writes.succeeded, 1);
  EXPECT_EQ(nv_stats_[LOG_ENTRY_FIRST_HANDLE + 2].writes.total, 0);
  EXPECT_EQ(nv_stats_[LOG_ENTRY_FIRST_HANDLE + 3].writes.total, 0);
  EXPECT_EQ(nv_stats_[LOG_ENTRY_FIRST_HANDLE + 4].writes.total, 0);
  EXPECT_GE(nv_stats_[LOG_ENTRY_PERSIST_HANDLE].writes.total, 2);
}

TEST_F(Test, ReEncryptWriteFailureIgnored) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 5}, {true, 4}, {true, 3}, {true, 2}, {true, 1},
  };
  SetLogStateInNV(state);
  // Fail decrypt for the first 2 spaces with the current key.
  aes_result_[0].set_fail_for(2);
  force_write_results_[LOG_ENTRY_FIRST_HANDLE + 1] = {TPM_RC_FAILURE, -1};
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(0, 1, 2, 3, 4));
  EXPECT_EQ(nv_stats_[LOG_ENTRY_FIRST_HANDLE + 0].writes.total, 1);
  EXPECT_EQ(nv_stats_[LOG_ENTRY_FIRST_HANDLE + 0].writes.succeeded, 1);
  EXPECT_GE(nv_stats_[LOG_ENTRY_FIRST_HANDLE + 1].writes.total, 1);
  EXPECT_EQ(nv_stats_[LOG_ENTRY_FIRST_HANDLE + 1].writes.succeeded, 0);
  EXPECT_GT(nv_stats_[LOG_ENTRY_PERSIST_HANDLE].writes.total, 0);
}

TEST_F(Test, ReEncryptWritePersistFailureIgnored) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 5}, {true, 4}, {true, 3}, {true, 2}, {true, 1},
  };
  SetLogStateInNV(state);
  // Fail decrypt for the first 2 spaces with the current key.
  aes_result_[0].set_fail_for(2);
  force_write_results_[LOG_ENTRY_PERSIST_HANDLE] = {TPM_RC_FAILURE, -1};
  EXPECT_EQ(pinweaver_eal_storage_start(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(0, 1, 2, 3, 4));
  EXPECT_EQ(nv_stats_[LOG_ENTRY_FIRST_HANDLE + 0].writes.total, 0);
  EXPECT_EQ(nv_stats_[LOG_ENTRY_FIRST_HANDLE + 1].writes.total, 0);
  EXPECT_GT(nv_stats_[LOG_ENTRY_PERSIST_HANDLE].writes.total, 0);
}

TEST_F(Test, InitOwnerSuccess) {
  EXPECT_EQ(pinweaver_eal_storage_initialize_owner(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(4, 3, 2, 1, 0));

  EXPECT_EQ(nv_stats_[TREE_DESCRIPTOR_HANDLE].defines.total, 1);
  EXPECT_EQ(nv_stats_[TREE_DESCRIPTOR_HANDLE].writes.total, 0);
  EXPECT_EQ(nv_stats_[LOG_ENTRY_PERSIST_HANDLE].defines.total, 1);
  EXPECT_EQ(nv_stats_[LOG_ENTRY_PERSIST_HANDLE].writes.total, 0);
  for (int n = 0; n < MAX_LOG_ENTRIES; ++n) {
    EXPECT_EQ(g_cached_log_entries[n].counter, 0);
    EXPECT_EQ(nv_stats_[LOG_ENTRY_FIRST_HANDLE + n].defines.total, 1);
    EXPECT_EQ(nv_stats_[LOG_ENTRY_FIRST_HANDLE + n].writes.total, 0);
  }
}

TEST_F(Test, InitOwnerIsNopIfDefined) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 5}, {false, 4}, {true, 3}, {true, 2}, {false, 1},
  };
  SetLogStateInNV(state);
  EXPECT_EQ(pinweaver_eal_storage_initialize_owner(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
}

TEST_F(Test, InitOwnerDefineFailureRetry) {
  force_define_results_[TREE_DESCRIPTOR_HANDLE] = {TPM_RC_FAILURE, 2};
  force_define_results_[LOG_ENTRY_FIRST_HANDLE + 4] = {TPM_RC_FAILURE, 1};
  force_define_results_[LOG_ENTRY_PERSIST_HANDLE] = {TPM_RC_FAILURE, 1};
  EXPECT_EQ(pinweaver_eal_storage_initialize_owner(), 0);
  EXPECT_EQ(g_storage_state, TPMSS_READY);
  EXPECT_THAT(g_cache_order, ElementsAre(4, 3, 2, 1, 0));
}

TEST_F(Test, InitOwnerFailure) {
  force_define_results_[LOG_ENTRY_FIRST_HANDLE + 3] = {TPM_RC_FAILURE, -1};
  EXPECT_EQ(pinweaver_eal_storage_initialize_owner(), -1);
  EXPECT_EQ(g_storage_state, TPMSS_NOT_INITIALIZED);
}

TEST_F(Test, InitStateSuccess) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {true, 5}, {true, 4}, {true, 3}, {true, 2}, {true, 1},
  };
  SetLogStateInNV(state);

  uint8_t hash[32];
  memset(hash, 0x78, sizeof(hash));
  SetTreeDescrRestartCount(898);
  SetLogEntry(0, 5, hash);

  uint8_t result_hash[32] = {};
  uint32_t result_restart_count = 22;
  EXPECT_EQ(
      pinweaver_eal_storage_init_state(result_hash, &result_restart_count), 0);
  EXPECT_EQ(result_restart_count, 898 + 1);
  EXPECT_THAT(result_hash, ElementsAreArray(hash));
  // EXPECT_EQ(memcmp(result_hash, hash, sizeof(hash)), 0);
}

TEST_F(Test, InitStateUninitialized) {
  CreateTreeInNV(false);
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {false, 0}, {false, 0}, {false, 0}, {false, 0}, {false, 0},
  };
  SetLogStateInNV(state);

  uint8_t expected_hash[32] = {
      0,
  };

  uint8_t result_hash[32];
  memset(result_hash, 0xFF, sizeof(result_hash));
  uint32_t result_restart_count = 22;
  EXPECT_EQ(
      pinweaver_eal_storage_init_state(result_hash, &result_restart_count), 0);
  EXPECT_EQ(result_restart_count, 1);
  EXPECT_THAT(result_hash, ElementsAreArray(expected_hash));
}

TEST_F(Test, InitStateFailure) {
  uint8_t result_hash[32] = {};
  uint32_t result_restart_count = 22;
  EXPECT_EQ(
      pinweaver_eal_storage_init_state(result_hash, &result_restart_count), -1);
}

TEST_F(Test, GetSetTreeDataSuccess) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {false, 0}, {false, 0}, {false, 0}, {false, 0}, {false, 0},
  };
  SetLogStateInNV(state);

  pw_long_term_storage_t tree = {};
  tree.bits_per_level.v = 10;
  tree.height.v = 18;
  SetTreeDescr(&tree);

  pw_long_term_storage_t result_tree;
  EXPECT_EQ(pinweaver_eal_storage_get_tree_data(&result_tree), 0);
  EXPECT_EQ(result_tree.bits_per_level.v, 10);
  EXPECT_EQ(result_tree.height.v, 18);
  EXPECT_GT(nv_stats_[TREE_DESCRIPTOR_HANDLE].reads.total, 0);
  // We wrote as a part of start to update restart_count
  EXPECT_EQ(nv_stats_[TREE_DESCRIPTOR_HANDLE].writes.total, 1);
  EXPECT_EQ(nv_stats_[TREE_DESCRIPTOR_HANDLE].writes.succeeded, 1);

  tree.bits_per_level.v = 5;
  tree.height.v = 42;
  EXPECT_EQ(pinweaver_eal_storage_set_tree_data(&tree), 0);
  EXPECT_EQ(nv_stats_[TREE_DESCRIPTOR_HANDLE].writes.total, 2);
  EXPECT_EQ(nv_stats_[TREE_DESCRIPTOR_HANDLE].writes.succeeded, 2);

  nv_stats_[TREE_DESCRIPTOR_HANDLE].reads.total = 0;
  EXPECT_EQ(pinweaver_eal_storage_get_tree_data(&result_tree), 0);
  EXPECT_EQ(nv_stats_[TREE_DESCRIPTOR_HANDLE].reads.total, 0);
  EXPECT_EQ(result_tree.bits_per_level.v, 5);
  EXPECT_EQ(result_tree.height.v, 42);
}

TEST_F(Test, GetSetTreeDataStartFailure) {
  pw_long_term_storage_t tree = {};
  EXPECT_EQ(pinweaver_eal_storage_get_tree_data(&tree), -1);
  EXPECT_EQ(pinweaver_eal_storage_set_tree_data(&tree), -1);
}

TEST_F(Test, GetSetLogSuccess) {
  uint32_t counter[MAX_LOG_ENTRIES] = {1, 5, 4, 2, 3};
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES];
  for (int n = 0; n < MAX_LOG_ENTRIES; ++n) {
    state[n] = {true, counter[n]};
  }
  SetLogStateInNV(state);

  for (int n = 0; n < MAX_LOG_ENTRIES; ++n) {
    uint8_t hash[32];
    memset(hash, (int)counter[n], sizeof(hash));
    SetLogEntry(n, counter[n], hash);
  }

  pw_log_storage_t result = {};
  EXPECT_EQ(pinweaver_eal_storage_get_log(&result), 0);
  EXPECT_THAT(g_cache_order, ElementsAre(1, 2, 4, 3, 0));

  uint32_t expected_counter = MAX_LOG_ENTRIES;
  for (int n = 0; n < PW_LOG_ENTRY_COUNT; ++n, --expected_counter) {
    uint8_t expected_hash[32];
    memset(expected_hash, expected_counter, sizeof(expected_hash));
    EXPECT_THAT(result.entries[n].root, ElementsAreArray(expected_hash));
  }

  pw_log_storage_t update;
  memcpy(&update, &result, sizeof(result));

  memmove(update.entries + 1, update.entries,
          (PW_LOG_ENTRY_COUNT - 1) * sizeof(update.entries[0]));
  memset(update.entries[0].root, 100, sizeof(update.entries[0].root));
  EXPECT_EQ(pinweaver_eal_storage_set_log(&update), 0);
  EXPECT_THAT(g_cache_order, ElementsAre(0, 1, 2, 4, 3));
  EXPECT_EQ(nv_stats_[LOG_ENTRY_FIRST_HANDLE + 0].writes.succeeded, 1);

  EXPECT_EQ(pinweaver_eal_storage_get_log(&result), 0);
  EXPECT_EQ(memcmp(result.entries, update.entries, sizeof(result.entries)), 0);
}

TEST_F(Test, GetSetLogUnwritten) {
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES] = {
      {false, 0}, {false, 0}, {false, 0}, {false, 0}, {false, 0},
  };
  SetLogStateInNV(state);

  pw_log_storage_t result = {};
  EXPECT_EQ(pinweaver_eal_storage_get_log(&result), 0);

  uint8_t expected_hash[32] = {
      0,
  };
  for (int n = 0; n < PW_LOG_ENTRY_COUNT; ++n) {
    EXPECT_THAT(result.entries[n].root, ElementsAreArray(expected_hash));
  }

  pw_log_storage_t update;
  memcpy(&update, &result, sizeof(result));

  memmove(update.entries + 1, update.entries,
          (PW_LOG_ENTRY_COUNT - 1) * sizeof(update.entries[0]));
  memset(update.entries[0].root, 100, sizeof(update.entries[0].root));
  EXPECT_EQ(pinweaver_eal_storage_set_log(&update), 0);

  EXPECT_EQ(pinweaver_eal_storage_get_log(&result), 0);
  EXPECT_EQ(memcmp(result.entries, update.entries, sizeof(result.entries)), 0);
}

TEST_F(Test, GetSetLogWriteFailure) {
  uint32_t counter[MAX_LOG_ENTRIES] = {1, 5, 4, 2, 3};
  CreateTreeInNV();
  Test::NVLogState state[MAX_LOG_ENTRIES];
  for (int n = 0; n < MAX_LOG_ENTRIES; ++n) {
    state[n] = {true, counter[n]};
  }
  SetLogStateInNV(state);

  for (int n = 0; n < MAX_LOG_ENTRIES; ++n) {
    uint8_t hash[32];
    memset(hash, (int)counter[n], sizeof(hash));
    SetLogEntry(n, counter[n], hash);
  }

  pw_log_storage_t result = {};
  EXPECT_EQ(pinweaver_eal_storage_get_log(&result), 0);
  EXPECT_THAT(g_cache_order, ElementsAre(1, 2, 4, 3, 0));

  uint32_t expected_counter = MAX_LOG_ENTRIES;
  for (int n = 0; n < PW_LOG_ENTRY_COUNT; ++n, --expected_counter) {
    uint8_t expected_hash[32];
    memset(expected_hash, expected_counter, sizeof(expected_hash));
    EXPECT_THAT(result.entries[n].root, ElementsAreArray(expected_hash));
  }

  pw_log_storage_t pre_update;
  memcpy(&pre_update, &result, sizeof(result));

  pw_log_storage_t update;
  memcpy(&update, &result, sizeof(result));

  memmove(update.entries + 1, update.entries,
          (PW_LOG_ENTRY_COUNT - 1) * sizeof(update.entries[0]));
  force_write_results_[LOG_ENTRY_FIRST_HANDLE + 0] = {TPM_RC_FAILURE, -1};
  EXPECT_EQ(pinweaver_eal_storage_set_log(&update), -1);
  EXPECT_THAT(g_cache_order, ElementsAre(1, 2, 4, 3, 0));
  EXPECT_EQ(nv_stats_[LOG_ENTRY_FIRST_HANDLE + 0].writes.succeeded, 0);

  EXPECT_EQ(pinweaver_eal_storage_get_log(&result), 0);
  EXPECT_EQ(memcmp(result.entries, pre_update.entries, sizeof(result.entries)),
            0);
}

TEST_F(Test, GetSetLogStartFailure) {
  pw_log_storage_t data = {};
  EXPECT_EQ(pinweaver_eal_storage_get_log(&data), -1);
  EXPECT_EQ(pinweaver_eal_storage_set_log(&data), -1);
}

TEST_F(Test, CachedLogEntryOrder) {
  g_cache_order[0] = -1;
  EXPECT_EQ(get_first_cached_log_entry(), nullptr);
  g_cache_order[0] = 100;
  EXPECT_EQ(get_first_cached_log_entry(), nullptr);
  g_cache_order[0] = 1;
  EXPECT_EQ(get_first_cached_log_entry(), &g_cached_log_entries[1]);

  g_cache_order[MAX_LOG_ENTRIES - 1] = -2;
  EXPECT_EQ(get_last_cached_log_entry_num(), -1);
  g_cache_order[MAX_LOG_ENTRIES - 1] = 100;
  EXPECT_EQ(get_last_cached_log_entry_num(), -1);
  g_cache_order[MAX_LOG_ENTRIES - 1] = 1;
  EXPECT_EQ(get_last_cached_log_entry_num(), 1);
}

TEST_F(Test, DeviceKeyObtainedOnce) {
  EXPECT_CALL(eal_, get_device_key(0, _)).Times(1);
  EXPECT_CALL(eal_, get_device_key(1, _)).Times(1);
  EXPECT_EQ(ensure_device_key(0), 0);
  EXPECT_EQ(ensure_device_key(0), 0);
  EXPECT_EQ(ensure_device_key(1), 0);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}