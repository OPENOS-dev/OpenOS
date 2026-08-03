# Copyright 2021 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

ifneq ($(V),)
  Q :=
else
  Q := @
endif

# we need clang because gtest is built with clang
CC = clang
CXX = clang++
MKDIR ?= mkdir

BUILD_PATH = $(abspath $(CURDIR)/build)
TOP_PATH = $(abspath $(CURDIR)/../../..)
TPM_STORAGE_PATH = $(TOP_PATH)/eal/tpm_storage
MINI_TRUNKS_PATH = $(TPM_STORAGE_PATH)/mini_trunks
TEST_PATH = $(TPM_STORAGE_PATH)/test

TARGET = $(BUILD_PATH)/tpm_storage_test
TARGET_PROFRAW = $(TARGET).profraw
TARGET_PROFDATA = $(TARGET).profdata
COV_REPORT = $(BUILD_PATH)/coverage_report.html
TESTED_SRC = tpm_storage.c
SRCS = $(TESTED_SRC) tpm_storage_test.cc mock_tss.cc mock_eal.cc
TESTED_OBJ = $(TESTED_SRC:%.c=$(BUILD_PATH)/%.o)
OBJS = $(filter %.o,$(SRCS:%.c=$(BUILD_PATH)/%.o))
OBJS += $(filter %.o,$(SRCS:%.cc=$(BUILD_PATH)/%.o))
INCLUDES = -I$(TEST_PATH) -I$(TPM_STORAGE_PATH) \
	-I$(MINI_TRUNKS_PATH) -I$(TOP_PATH)
PROFILE_FLAGS = -fprofile-instr-generate -fcoverage-mapping
OPTIONS = -g $(PROFILE_FLAGS) -O3 -DTEST_TPM_STORAGE
CXXOPTIONS = $(OPTIONS) -std=c++11
LDFLAGS += $(PROFILE_FLAGS)
LIBS = -lgtest -lgmock -lgcov -lstdc++ -lpthread
DEPS = Makefile

.PHONY: all build clean cov run
.DEFAULT: all

all: run cov

build: $(TARGET)

clean:
	$(RM) -rf $(BUILD_PATH)

cov: $(COV_REPORT)
	$(Q)llvm-cov report $(TESTED_OBJ) --instr-profile=$(TARGET_PROFDATA) \
		--ignore-filename-regex=".*\.h"

$(TARGET): $(BUILD_PATH) $(OBJS)
	@echo "  BIN $@"
	$(Q)$(CC) -o $@ $(OBJS) -L/usr/lib64 $(LDFLAGS) -L$(BUILD_PATH) $(LIBS)

$(COV_REPORT): $(TARGET) $(TARGET_PROFDATA)
	$(Q)llvm-cov show $(TESTED_OBJ) --instr-profile=$(TARGET_PROFDATA) \
		--format=html > $(COV_REPORT)

$(TARGET_PROFDATA): $(TARGET_PROFRAW)
	$(Q)llvm-profdata merge -sparse $(TARGET_PROFRAW) -o $(TARGET_PROFDATA)

.PRECIOUS: $(TARGET_PROFRAW)

run: $(TARGET)
	$(Q)LLVM_PROFILE_FILE=$(TARGET_PROFRAW) $(TARGET)

$(BUILD_PATH):
	$(Q)$(MKDIR) -p $@

vpath %.c $(TPM_STORAGE_PATH)

$(BUILD_PATH)/%.o: %.c $(DEPS)
	@echo "  CC $@"
	$(Q)$(CC) -c $< -o $@ $(CFLAGS) $(OPTIONS) $(INCLUDES)

$(BUILD_PATH)/%.o: %.cc $(DEPS)
	@echo "  CPP $@"
	$(Q)$(CXX) -c -o $@ $< $(CXXFLAGS) $(CXXOPTIONS) $(INCLUDES)
