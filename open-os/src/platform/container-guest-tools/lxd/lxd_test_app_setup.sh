#!/bin/bash
# Copyright 2020 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Set up a Debian container for test.
# This is run from inside the container as root.

set -eux -o pipefail

# To update this, run apt-cache policy code on your DUT with this
# container installed, and look at the latest version.
# Note that updating this package is likely to break screendiffs.
# Releases for different architectures have different build numbers.
VSCODE_VERSION_AMD64="1.77.3-1681292746"
VSCODE_VERSION_ARM64="1.77.3-1681295476"

main() {
    local release=$1
    local arch=$2
    export DEBIAN_FRONTEND=noninteractive

    local -a packages
    packages=(
        audacity
        docker.io
        emacs
        firefox-esr
        gedit
        vlc
        # For crostini.AppLibreOffice.*
        fonts-liberation2
        libreoffice
        libreoffice-gtk3
        # For crostini.Podman*
        podman
        fuse-overlayfs
        slirp4netns
    )

    # for testing Visual Studio Code.
    curl -sSL https://packages.microsoft.com/keys/microsoft.asc \
        | gpg --dearmor > /etc/apt/trusted.gpg.d/packages.microsoft.gpg
    cat > /etc/apt/sources.list.d/vscode.list << EOF
deb [arch=amd64,arm64 signed-by=/etc/apt/trusted.gpg.d/packages.microsoft.gpg] https://packages.microsoft.com/repos/code stable main
EOF

    apt-get -o Acquire::Retries=3 -q update

    if [ "${arch}" = "amd64" ]; then
        # for testing Eclipse.
        if [[ "${release}" == "bookworm" ]]; then
          # Workaround for installing JRE on bookworm
          eatmydata apt-get install -y default-jre-headless
        fi

        packages+=( default-jre )
        wget -q https://storage.googleapis.com/chromiumos-test-assets-public/crostini_test_files/eclipse.tar.gz
        tar -xf eclipse.tar.gz -C /usr/
        ln -s /usr/eclipse/eclipse /usr/bin/eclipse
        rm -f eclipse.tar.gz

        packages+=( "code=${VSCODE_VERSION_AMD64}" )
    elif [ "${arch}" = "arm64" ]; then
        packages+=( "code=${VSCODE_VERSION_ARM64}" )
    fi

    eatmydata apt-get -o Acquire::Retries=3 -q -y --no-install-recommends \
      install "${packages[@]}"

    apt-get clean
    rm -rf /var/lib/apt/lists
}

main "$@"
