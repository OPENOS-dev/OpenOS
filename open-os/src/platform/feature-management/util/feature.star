# Copyright 2023 The ChromiumOS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Functions to generate feature protobuf"""

load(
    "@proto//feature_management.proto",
    feature_pb = "chromiumos.feature_management.api.software",
)
load("//util/generate.star", "generate")

_ALL_USAGES = [
    feature_pb.Feature.USAGE_LOCAL,
    feature_pb.Feature.USAGE_CHROME,
    feature_pb.Feature.USAGE_ANDROID,
]

_ALL_SCOPES = [
    feature_pb.Feature.SCOPE_DEVICES_0,
    feature_pb.Feature.SCOPE_DEVICES_1,
]

def _get_contacts(emails):
    contacts = []
    for email in emails:
        contacts.append(feature_pb.Feature.Contact(email = email))

    return contacts

def _create_feature(name, contacts, feature_level, scopes = _ALL_SCOPES, usages = _ALL_USAGES):
    return feature_pb.Feature(
        name = name,
        contacts = _get_contacts(contacts),
        feature_level = feature_level,
        scopes = scopes,
        usages = usages,
    )

def _append_feature(features, feature):
    features.append(feature)

feature = struct(
    create_feature = _create_feature,
    generate = generate.generate_include,
)
