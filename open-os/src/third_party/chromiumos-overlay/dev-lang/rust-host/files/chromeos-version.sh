#!/bin/bash -eu
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

my_dir="$(dirname "$(readlink -m "$0")")"
chromiumos_overlay="${my_dir}/../../.."
cros_rustc_eclass="${chromiumos_overlay}/eclass/cros-rustc.eclass"

# Find the value after RUSTC_STABLE_VERSION, removing any double-quotes & comments.
# Since the script quits after the first match, `version` may be either empty, or have
# the contents of a single match.
version="$(
	sed -n '{
		s/^RUSTC_STABLE_VERSION="\([^"]*\)".*/\1/p;
		T;
		q;
	}' "${cros_rustc_eclass}"
)"

if [[ -z "${version}" ]]; then
	echo "Could not find RUSTC_STABLE_VERSION in ${cros_rustc_eclass}!" >&2
	exit 1
fi

echo "${version}"
