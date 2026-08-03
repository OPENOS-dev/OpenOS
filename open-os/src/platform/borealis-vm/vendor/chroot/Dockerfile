# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

#******************************************************************************
#                                                                runtime_chroot
#******************************************************************************
## The chroot image includes native video drivers and audio config, suitable for
## running under src/platform/borealis/tools/borealis_chroot/borealis_chroot.sh.
ARG PARENT_IMAGE="cros_borealis:runtime_debug"
FROM $PARENT_IMAGE as initial

# Make sure we have native Mesa drivers.
RUN if [[ ! -f /usr/lib/dri/iris_dri.so ]] || \
       [[ ! -f /usr/lib/dri/radeonsi_dri.so ]] || \
       [[ ! -f /usr/lib/dri/zink_dri.so ]] || \
       [[ ! -f /usr/lib/libvulkan_intel.so ]] || \
       [[ ! -f /usr/lib/libvulkan_radeon.so ]]; then \
        echo -n "Native Mesa drivers not built; this image will not "; \
        echo "work in the chroot :("; \
        echo -n "Please see tools/borealis_chroot/README.md "; \
        echo "for chroot image build instructions."; \
        exit 1; \
    fi

COPY etc_chroot/ /etc/

# bazel requires access to ~/.cache/bazel, so we can't build as nobody.
RUN useradd -m -d /scratch builduser

ENV ARTIFACTS=/artifacts
RUN mkdir -p /artifacts
RUN chgrp builduser /scratch /artifacts && \
    chmod g+ws /scratch /artifacts

#******************************************************************************
#                                                                  cras-builder
#******************************************************************************
# CRAS build deps
FROM initial AS cras-builder
RUN pacman -Sy --noconfirm \
    # CRAS build deps
    bazel \
    clang \
    iniparser \
    ladspa \
    rust

#******************************************************************************
#                                                                    cras-build
#******************************************************************************
# Build 64-bit CRAS
FROM cras-builder AS cras-build
COPY packages/cras-git/ /scratch/cras-git/
WORKDIR /scratch/cras-git

# Allow makepkg to be run as builduser.
RUN chgrp -R builduser . && chmod -R g+ws .

# Perform the build as builduser.
USER builduser
RUN makepkg && cp *.zst "$ARTIFACTS"

#******************************************************************************
#                                                              lib32-cras-build
#******************************************************************************
# Build 32-bit CRAS
FROM cras-builder AS lib32-cras-build
COPY packages/lib32-cras-git/ /scratch/lib32-cras-git/
WORKDIR /scratch/lib32-cras-git

# Allow makepkg to be run as builduser.
RUN chgrp -R builduser . && chmod -R g+ws .

# Perform the build as builduser.
USER builduser
RUN makepkg && cp *.zst "$ARTIFACTS"

#******************************************************************************
#                                                                runtime_chroot
#******************************************************************************
FROM initial as runtime_chroot

# Install CRAS packages
COPY --from=cras-build /artifacts/*.zst pkgs/
COPY --from=lib32-cras-build /artifacts/*.zst pkgs/
RUN pacman -U --noconfirm pkgs/*.zst \
    && pacman -Sy --noconfirm \
    # Pressure-vessel in the chroot requires bubblewrap in setuid mode.
    # Install the package now so setuid can be configured in borealis_chroot.sh.
    # The non-setuid mode built into pressure-vessel (used in the VM) requires
    # creating a user namespace and this operation is not permitted in a chroot.
    bubblewrap \
    # 32-bit video libs
    lib32-gdk-pixbuf2 \
    lib32-gtk2 \
    lib32-libpulse \
    lib32-libva \
    lib32-libvdpau \
    lib32-libxi \
    lib32-libxtst \
    # Audio testing
    mpg123
