# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This Dockerfile builds images for running tools on developer machines.
#
# For the Dockerfile that builds production Borealis DLC images, see
# build/Dockerfile instead.

#******************************************************************************
#                                                                       initial
#******************************************************************************
# The initial image is a very slightly modified archlinux standard image
# with the extra repos necessary to install our software.
# This is a fork of the same stage from the production image Dockerfile.
# TODO(cpelling): Find a way to do this without forking the entire stage.
# TODO(pobega): Find a solution for historic builds that doesn't require
#               manual upreving. This tag will eventually disappear.
FROM archlinux:base-20230702.0.161694 AS initial

ARG disable_arch_sig_validation

COPY etc/pacman.conf* /etc/
COPY etc/pacman.d/* /etc/pacman.d/
# This tools image uses a different borealis-historic-mirror file.
# It's the same as the main one but uprevs separately (currently manually)
# so that breakages to the main image and tools image don't impact each other.
# TODO(pobega): Uprev this borealis-historic-mirror automatically as well.
COPY etc/pacman.d.for-tools-image/* /etc/pacman.d/
# Symlinks don't properly copy into the container so manually create symlink
RUN ln -sf /etc/pacman.d/siglevel.default /etc/pacman.d/siglevel
# If --disable-arch-sig-validation is set change the siglevel symlink
# to point to the more permissive siglevel.trustall
RUN if [[ ! -z "${disable_arch_sig_validation}" \
          && "${disable_arch_sig_validation}" == 'true' ]]; then \
  ln -sf /etc/pacman.d/siglevel.trustall /etc/pacman.d/siglevel; fi
# Chain together these tasks to avoid Docker cache consistency issues
RUN chmod a+r /etc/pacman.conf* \
              /etc/pacman.d/* \
    # Force update our databases using our historical mirror URL.
    && pacman -Syy --config=/etc/pacman.conf.db \
    # Update the Archlinux keyring to get new package maintainer keys
    && pacman -S --noconfirm archlinux-keyring \
    && pacman-key --init && pacman-key --populate archlinux \
    # Perform a full system upgrade using our package mirror.
    && pacman -Su --noconfirm \
        arch-audit \
        diffutils

#******************************************************************************
#                                                      trace-window-system-deps
#******************************************************************************
# trace-window-system-deps installs dependencies for trace-window-system.
FROM initial AS trace-window-system-deps
RUN pacman -S --noconfirm \
    python3 \
    wireshark-cli

#******************************************************************************
#                                                           trace-window-system
#******************************************************************************
# trace-window-system runs trace_window_system.py inside a Docker container, to
# minimize required setup and ensure consistent dependency versions.
# Assumes ../tools is mounted at /tools.
FROM trace-window-system-deps AS trace-window-system
ENTRYPOINT ["/tools/trace_window_system.py", "--no-container"]
