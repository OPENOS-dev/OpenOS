#!/usr/bin/env lucicfg generate
# Copyright 2023 The ChromiumOS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

""" File required by the Makefile, does not add any features."""

load("//util/proto.star", "protos")
load(
    "@proto//feature_management.proto",
    feature_pb = "chromiumos.feature_management.api.software",
)
load("//util/feature.star", "feature")

def all_features():
    return []

# Output filename has to match filename.
feature.generate(feature_pb.FeatureBundle(features = all_features()), "empty_features.binary")
