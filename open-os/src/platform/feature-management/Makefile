# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# TODO(b:281404554): Upgrade to blaze.

SRCDIR := $(shell pwd)
GENERATED = ${SRCDIR}/generated
PROTO = ${SRCDIR}/proto
DEVICES = ${SRCDIR}/devices
DEVICES_TEST = ${SRCDIR}/devices_test

# Protobuf binary code.
GENERATED_DESCPB_BIN = ${GENERATED}/descpb.bin

# All the features starlak files to be processed.
FEATURES_BINARIES = $(patsubst ${SRCDIR}/%.star, \
		      ${GENERATED}/%.binary, \
		      $(wildcard ${SRCDIR}/*features.star))

# Binary version of the database with all feature descriptions.
GENERATED_FEATURE_BIN = ${GENERATED}/features_final.binary

# protobuf describing the device selections.
DEVICES_SELECTIONS = $(wildcard ${DEVICES}/*textproto)

# protobuf describing the device selection samples.
DEVICES_SELECTION_SAMPLES = $(wildcard ${DEVICES_TEST}/*textproto)

# Binary version of the database with all feature descriptions.
GENERATED_DEVICE_BIN = ${GENERATED}/devices_final.binary

# Binary version of the device selection test data for libsegmentation.
GENERATED_SELECTION_SAMPLE_BIN = ${GENERATED}/selection_sample_final.binary

# Generated .h file from the database for libsegmentation.
GENERATED_LIGSEGMENTATION_PB_H = ${GENERATED}/libsegmentation_pb.h

# Generated .h file for test data for libsegmentation.
GENERATED_LIGSEGMENTATION_TEST_PB_H  = ${GENERATED}/libsegmentation_test_pb.h

# Verbose? Use V=1
ifeq ($(filter-out 0,${V}),)
Q := @
endif

# Default target.
.PHONY: all
all: ${GENERATED_LIGSEGMENTATION_PB_H} ${GENERATED_LIGSEGMENTATION_TEST_PB_H}

${GENERATED_DESCPB_BIN} : $(wildcard ${PROTO}/*.proto) | ${GENERATED}
	${Q}/usr/bin/protoc -I${PROTO} --descriptor_set_out=$@ $(filter-out $<,$^)

${GENERATED}/%.binary : %.star ${GENERATED_DESCPB_BIN}
	${Q}/usr/bin/lucicfg generate $<

${GENERATED_FEATURE_BIN} : ${FEATURES_BINARIES}
	${Q}/bin/cat $^ > $@

${GENERATED_DEVICE_BIN} : ${DEVICES_SELECTIONS} | ${GENERATED}
	${Q}/bin/cat $^ | protoc -I "./proto" \
	--encode=chromiumos.feature_management.api.software.SelectionBundle \
	device_selection.proto > $@

${GENERATED_SELECTION_SAMPLE_BIN} : ${DEVICES_SELECTION_SAMPLES} | ${GENERATED}
	${Q}/bin/cat $^ | protoc -I "./proto" \
	--encode=chromiumos.feature_management.api.software.SelectionSampleBundle \
	device_selection.proto >$@

${GENERATED_LIGSEGMENTATION_PB_H} : ${GENERATED_FEATURE_BIN} \
	                                  ${GENERATED_DEVICE_BIN}
	${Q}${SRCDIR}/generate_include_file.sh \
		$(GENERATED) \
		$(GENERATED_LIGSEGMENTATION_PB_H:$(GENERATED)/%=%) \
	$(foreach f,$^, ${f} $(patsubst ${GENERATED}/%_final.binary, \
			       protobuf_% ,${f}))

${GENERATED_LIGSEGMENTATION_TEST_PB_H} : ${GENERATED_SELECTION_SAMPLE_BIN}
	${Q}${SRCDIR}/generate_include_file.sh \
		$(GENERATED) \
		$(GENERATED_LIGSEGMENTATION_TEST_PB_H:$(GENERATED)/%=%) \
		$(foreach f,$^, ${f} $(patsubst ${GENERATED}/%_final.binary, \
				       protobuf_% ,${f}))

.PHONY: clean
clean:
	${Q}/bin/rm -rf ${GENERATED}

${GENERATED}:
	${Q}mkdir -p $@
