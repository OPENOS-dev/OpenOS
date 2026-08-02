#!/bin/bash
# Copyright 2018 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Set up a Debian container. This is run from inside the container as root.

set -eux -o pipefail

main() {
    local release=$1
    export DEBIAN_FRONTEND=noninteractive

    local cros_staging_list="/etc/apt/sources.list.d/cros-staging.list"
    echo "deb [trusted=yes] file:///run/apt ${release} main" > "${cros_staging_list}"
    if [[ "${release}" = "bookworm" ]]; then
        echo "deb https://deb.debian.org/debian bookworm-backports main" >> "${cros_staging_list}"
    elif [[ "${release}" = "bullseye" ]]; then
        echo "deb https://archive.debian.org/debian bullseye-backports main" >> "${cros_staging_list}"
    fi

    apt-get -o Acquire::Retries=3 update

    apt-get -o Acquire::Retries=3 -q -y --allow-unauthenticated install cros-apt-config
    apt-get -o Acquire::Retries=3 -q -y --allow-unauthenticated install cros-guest-tools
    # Upgrade packages again to ensure cros-apt-config changes are picked up.
    apt-get -o Acquire::Retries=3 -q -y dist-upgrade

    apt-get clean
    rm "${cros_staging_list}"
    apt-get -o Acquire::Retries=3 update

    # Don't run sshd out of the box.
    touch /etc/ssh/sshd_not_to_be_run
}

main "$@"
