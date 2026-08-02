# Copyright 2022 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

#******************************************************************************
#                                                                       initial
#******************************************************************************
# The initial image is a very slightly modified archlinux standard image
# with the extra repos necessary to install our software.
# TODO(pobega): Find a solution for historic builds that doesn't require
#               manual upreving. This tag will eventually disappear.
FROM archlinux:base-20250824.0.410029 AS initial

ARG disable_arch_sig_validation

# Most of /etc is added later on (to avoid invalidating the entire Docker
# cache too frequently), but we need pacman.conf ASAP.
COPY etc/pacman.conf* /etc/
COPY etc/pacman.d/* /etc/pacman.d/
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
#                                                                       builder
#******************************************************************************
# builder has tooling to do basic 32-bit and 64-bit builds.
FROM initial AS builder
RUN pacman -S --noconfirm \
        base-devel \
        clang \
        cmake \
        git \
        llvm \
        meson \
        multilib-devel \
        ninja \
        python3 \
        sudo \
        vim
WORKDIR /scratch
COPY sudoers.d/* /etc/sudoers.d/
ENV ARTIFACTS=/artifacts
RUN mkdir -p /artifacts
RUN chgrp nobody /scratch /artifacts && \
    chmod g+ws /scratch /artifacts

#******************************************************************************
#                                                                  rust-builder
#******************************************************************************
FROM builder AS rust-builder
RUN pacman -S --noconfirm rust

#******************************************************************************
#                                                              perfetto-builder
#******************************************************************************
# perfetto-builder has libraries to build perfetto daemons.
FROM builder AS perfetto-builder
WORKDIR /scratch
# checkout a pinned Perfetto release available as git tag vNNN.N
RUN git clone --depth 1 --branch v48.0 \
    https://android.googlesource.com/platform/external/perfetto
WORKDIR /scratch/perfetto
RUN tools/write_version_header.py --stdout > /artifacts/version && \
    tools/install-build-deps

#******************************************************************************
#                                                                perfetto-build
#******************************************************************************
# perfetto-build builds perfetto daemons.
# TODO(davidriley): Convert this into a proper package.
FROM perfetto-builder AS perfetto-build
WORKDIR /scratch/perfetto
RUN tools/gn gen out/release \
        --args=" \
            is_clang=true \
            is_debug=false \
            enable_perfetto_zlib=false" \
# TODO(b/241286650) Re-add use_custom_libcxx=false to save rootfs space.
    && tools/ninja -C out/release tracebox \
    && mkdir -p /artifacts && cp out/release/stripped/tracebox /artifacts/

#******************************************************************************
#                                                            perfetto-sdk-build
#******************************************************************************
# perfetto-sdk-build builds the perfetto sdk.
FROM perfetto-builder AS perfetto-sdk-build
WORKDIR /scratch/perfetto
RUN CFLAGS='-fdata-sections -ffunction-sections' meson /scratch/builddir \
    && ninja -C /scratch/builddir \
    && cp sdk/perfetto.h /artifacts/ \
    && cp /scratch/builddir/libperfetto.a /artifacts/
COPY pkgconfig/perfetto.pc.sh /scratch/
RUN version=$(cat /artifacts/version) \
    && bash /scratch/perfetto.pc.sh ${version:1} lib64 > /artifacts/perfetto.pc

#******************************************************************************
#                                                      lib32-perfetto-sdk-build
#******************************************************************************
# lib32-perfetto-sdk-build builds the lib32 perfetto sdk.
FROM perfetto-builder AS lib32-perfetto-sdk-build
WORKDIR /scratch/perfetto
RUN CXXFLAGS='-m32' CFLAGS='-fdata-sections -ffunction-sections -m32' \
    meson /scratch/builddir \
    && ninja -C /scratch/builddir \
    && cp sdk/perfetto.h /artifacts/ \
    && cp /scratch/builddir/libperfetto.a /artifacts/
COPY pkgconfig/perfetto.pc.sh /scratch/
RUN version=$(cat /artifacts/version) \
    && bash /scratch/perfetto.pc.sh ${version:1} lib32 > /artifacts/perfetto.pc

#******************************************************************************
#                                                               wayland-builder
#******************************************************************************
# wayland-builder has prerequisites to build wayland
FROM builder AS wayland-builder
RUN pacman -Su --noconfirm \
    docbook-xsl \
    graphviz

#******************************************************************************
#                                                                 wayland-build
#******************************************************************************
FROM wayland-builder AS wayland-build
COPY packages/cros-wayland-git/ /scratch/cros-wayland-git/
WORKDIR /scratch/cros-wayland-git

# Allow makepkg to be run as nobody.
RUN chgrp -R nobody . && chmod -R g+ws .

# Perform the build as nobody.
USER nobody
RUN makepkg && cp *.zst "$ARTIFACTS"

#******************************************************************************
#                                                         lib32-wayland-builder
#******************************************************************************
# lib32-wayland-builder has prerequisites to build lib32-wayland
FROM builder AS lib32-wayland-builder
COPY --from=wayland-build /artifacts/*.zst pkgs/
RUN pacman -U --noconfirm pkgs/*.zst \
    && pacman -S --noconfirm \
        docbook-xsl \
        graphviz \
        lib32-expat \
        lib32-glibc \
        lib32-libffi \
        lib32-libxml2

#******************************************************************************
#                                                           lib32-wayland-build
#******************************************************************************
FROM lib32-wayland-builder AS lib32-wayland-build
COPY packages/lib32-cros-wayland-git/ /scratch/lib32-cros-wayland-git/
WORKDIR /scratch/lib32-cros-wayland-git

# Allow makepkg to be run as nobody.
RUN chgrp -R nobody . && chmod -R g+ws .

# Perform the build as nobody.
USER nobody
RUN makepkg && cp *.zst "$ARTIFACTS"

#******************************************************************************
#                                                                   gfx-builder
#******************************************************************************
# gfx-builder has prerequisites to build the graphics stack.
FROM builder AS gfx-builder
# Install packages already built.
WORKDIR /scratch
COPY --from=lib32-wayland-build /artifacts/*.zst pkgs/
COPY --from=wayland-build /artifacts/*.zst pkgs/
RUN pacman -U --noconfirm pkgs/*.zst \
    && pacman -S --noconfirm \
        elfutils \
        glslang \
        libclc \
        libdrm \
        libunwind \
        libx11 \
        libxdamage \
        libxrandr \
        libxshmfence \
        libxxf86vm \
        lm_sensors \
        python-mako \
        python-packaging \
        python-ply \
        python-yaml \
        vulkan-icd-loader \
        vulkan-headers \
        vulkan-utility-libraries \
        wayland-protocols \
        xorgproto

#******************************************************************************
#                                                                  mesa-builder
#******************************************************************************
# mesa-builder has prerequisites to build mesa.
FROM gfx-builder AS mesa-builder
RUN pacman -S --noconfirm \
        libglvnd \
        ocl-icd \
        spirv-llvm-translator

COPY --from=perfetto-sdk-build /artifacts/perfetto.h /usr/include/
COPY --from=perfetto-sdk-build /artifacts/libperfetto.a /usr/lib64/
COPY --from=perfetto-sdk-build /artifacts/perfetto.pc /usr/lib64/pkgconfig/

#******************************************************************************
#                                                                    mesa-build
#******************************************************************************
# mesa-build builds cros-mesa-git package.
FROM mesa-builder AS mesa-build

# Set to 1 to build native drivers, for borealis_chroot.
ARG BUILD_NATIVE_DRIVERS=0

# Sync package.
COPY packages/cros-mesa-git/ /scratch/cros-mesa-git/
WORKDIR /scratch/cros-mesa-git

# Allow makepkg to be run as nobody.
RUN chgrp -R nobody . && chmod -R g+ws .

# Perform the build as nobody.
USER nobody
RUN env BUILD_NATIVE_DRIVERS="${BUILD_NATIVE_DRIVERS}" makepkg
RUN cp *.zst "$ARTIFACTS"

#******************************************************************************
#                                                            lib32-mesa-builder
#******************************************************************************
# lib32-mesa-builder has prerequisites to build lib32-mesa.
FROM gfx-builder AS lib32-mesa-builder

# Install packages already built.
WORKDIR /scratch
COPY --from=lib32-wayland-build /artifacts/*.zst pkgs/
COPY --from=mesa-build /artifacts/*.zst pkgs/
RUN pacman -U --noconfirm pkgs/*.zst \
    && pacman -S --noconfirm \
        glslang \
        lib32-clang \
        lib32-libdrm \
        lib32-libelf \
        lib32-libglvnd \
        lib32-libunwind \
        lib32-libx11 \
        lib32-libxdamage \
        lib32-libxml2 \
        lib32-libxrandr \
        lib32-libxshmfence \
        lib32-libxxf86vm \
        lib32-llvm \
        lib32-llvm-libs \
        lib32-lm_sensors \
        lib32-spirv-llvm-translator \
        lib32-vulkan-icd-loader \
        lib32-zstd \
        wayland-protocols

COPY --from=lib32-perfetto-sdk-build /artifacts/perfetto.h /usr/include/
COPY --from=lib32-perfetto-sdk-build /artifacts/libperfetto.a /usr/lib32/
COPY --from=lib32-perfetto-sdk-build \
     /artifacts/perfetto.pc /usr/lib32/pkgconfig/

#******************************************************************************
#                                                              lib32-mesa-build
#******************************************************************************
# lib32-mesa-build builds 32-bit lib32-cros-mesa-git package.
FROM lib32-mesa-builder AS lib32-mesa-build

# Set to 1 to build native drivers, for borealis_chroot.
ARG BUILD_NATIVE_DRIVERS=0

# Sync package.
COPY packages/lib32-cros-mesa-git/ /scratch/lib32-cros-mesa-git/
WORKDIR /scratch/lib32-cros-mesa-git

# Allow makepkg to be run as nobody.
RUN chgrp -R nobody . && chmod -R g+ws .

# Perform the build as nobody.
USER nobody
RUN env BUILD_NATIVE_DRIVERS="${BUILD_NATIVE_DRIVERS}" makepkg
RUN cp *.zst "$ARTIFACTS"

#******************************************************************************
#                                                              xwayland-builder
#******************************************************************************
# xwayland-builder has prerequisites to build xwayland.
FROM gfx-builder AS xwayland-builder

# Install packages already built.
WORKDIR /scratch
COPY --from=mesa-build /artifacts/*.zst pkgs/
RUN pacman -U --noconfirm pkgs/*.zst \
    && pacman -S --noconfirm \
        egl-wayland \
        libepoxy \
        libxcvt \
        libxfont2 \
        libxkbfile \
        pixman \
        xorg-font-util \
        xorg-server-common \
        xtrans

# One of our xwayland patches needs virtwl.h.
# Putting it in /scratch/xwayland/include/linux causes the source package to
# fail to untar correctly, so install it system-wide instead. This should be
# fine since this container is only used to build this single package.
COPY include/linux/virtwl.h /usr/include/linux/

#******************************************************************************
#                                                                xwayland-build
#******************************************************************************
# xwayland-build builds cros-xwayland package.
FROM xwayland-builder AS xwayland-build

# Sync package.
COPY packages/cros-xwayland/ /scratch/cros-xwayland/
WORKDIR /scratch/cros-xwayland

# Allow makepkg to be run as nobody.
RUN chgrp -R nobody . \
    && chmod -R g+ws . \
    && chmod a+r /usr/include/linux/virtwl.h

# Perform the build as nobody.
# TODO(davidriley): Avoid --skippgpcheck.
USER nobody
RUN makepkg --skippgpcheck
RUN cp *.zst "$ARTIFACTS"

#******************************************************************************
#                                                                waffle-builder
#******************************************************************************
# waffle is used for apitrace, glbench and should be included by default.
# waffle-builder has prerequisites to build waffle.
FROM gfx-builder AS waffle-builder

# Install packages already built.
WORKDIR /scratch
COPY --from=mesa-build /artifacts/*.zst pkgs/
RUN pacman -U --noconfirm pkgs/*.zst

#******************************************************************************
#                                                                  waffle-build
#******************************************************************************
# waffle-build builds waffle and lib32-waffle packages.
FROM waffle-builder AS waffle-build

# Sync packages.
COPY packages/waffle/ /scratch/waffle/

# Allow makepkg to be run as nobody.
RUN chgrp -R nobody /scratch && chmod -R g+ws /scratch

USER nobody

# Perform the waffle build.
WORKDIR /scratch/waffle
# TODO(davidriley): Avoid --skippgpcheck.
RUN makepkg --skippgpcheck --noconfirm

RUN cp /scratch/waffle/*.zst "$ARTIFACTS"

#******************************************************************************
#                                                          lib32-waffle-builder
#******************************************************************************
# waffle is used for apitrace, glbench and should be included by default.
# lib32-waffle-builder has prerequisites to build lib32-waffle.
FROM gfx-builder AS lib32-waffle-builder

# Install packages already built.
WORKDIR /scratch
COPY --from=lib32-mesa-build /artifacts/*.zst pkgs/
COPY --from=mesa-build /artifacts/*.zst pkgs/
COPY --from=waffle-build /artifacts/*.zst pkgs/
RUN pacman -U --noconfirm pkgs/*.zst \
    && pacman -S --noconfirm \
        lib32-libgl \
        lib32-libgles \
        lib32-systemd

#******************************************************************************
#                                                            lib32-waffle-build
#******************************************************************************
# lib32-waffle-build builds lib32-waffle packages.
FROM lib32-waffle-builder AS lib32-waffle-build

# Sync packages.
COPY packages/waffle/ /scratch/waffle/
COPY packages/lib32-waffle/ /scratch/lib32-waffle/

# Allow makepkg to be run as nobody.
RUN chgrp -R nobody /scratch && chmod -R g+ws /scratch

USER nobody

# Perform the lib32-waffle build.
WORKDIR /scratch/lib32-waffle
# TODO(davidriley): Avoid --skippgpcheck.
RUN makepkg --skippgpcheck

RUN cp /scratch/lib32-waffle/*.zst "$ARTIFACTS"

#******************************************************************************
#                                                              apitrace-builder
#******************************************************************************
# apitrace is used for development but should be included by default.
# apitrace-builder has prerequisites to build apitrace.
FROM gfx-builder AS apitrace-builder

# Install packages already built.
WORKDIR /scratch
COPY --from=lib32-waffle-build /artifacts/*.zst pkgs/
COPY --from=mesa-build /artifacts/*.zst pkgs/
COPY --from=waffle-build /artifacts/*.zst pkgs/
RUN pacman -U --noconfirm pkgs/*.zst \
    && pacman -S --noconfirm \
        lib32-procps-ng

#******************************************************************************
#                                                                apitrace-build
#******************************************************************************
# apitrace-build builds cros-apitrace-git and lib32-cros-apitrace-git packages.
FROM apitrace-builder AS apitrace-build

# Sync packages.
COPY packages/cros-apitrace-git/ /scratch/cros-apitrace-git/
COPY packages/lib32-cros-apitrace-git/ /scratch/lib32-cros-apitrace-git/

# Allow makepkg to be run as nobody.
RUN chgrp -R nobody /scratch && chmod -R g+ws /scratch

USER nobody

# Perform the cros-apitrace-git build.
WORKDIR /scratch/cros-apitrace-git
# TODO(davidriley): Avoid --skippgpcheck.
RUN makepkg --skippgpcheck

# Perform the lib32-cros-apitrace-git build.
WORKDIR /scratch/lib32-cros-apitrace-git
# TODO(davidriley): Avoid --skippgpcheck.
RUN makepkg --skippgpcheck

RUN cp /scratch/cros-apitrace-git/*.zst /scratch/lib32-cros-apitrace-git/*.zst \
    "$ARTIFACTS"

#******************************************************************************
#                                                          apitrace-win-builder
#******************************************************************************
# Currently this just ensures that p7zip is installed
FROM builder AS apitrace-win-builder
RUN pacman -S --noconfirm --disable-download-timeout \
    p7zip

#******************************************************************************
#                                                            apitrace-win-build
#******************************************************************************
# This will simply fetch the release Windows binaries and perform the extraction
# The renaming of the directory is so that later stages which depend
# on this one can use a defined name to refer to both 32 and 64-bit builds
# of Windows apitrace
FROM apitrace-win-builder AS apitrace-win-build
WORKDIR /scratch
ENV APITRACE_WIN_VER=10.0
ENV APITRACE_W32_NAME=apitrace-${APITRACE_WIN_VER}-win32.7z
ENV APITRACE_W64_NAME=apitrace-${APITRACE_WIN_VER}-win64.7z
ADD http://commondatastorage.googleapis.com/chromeos-localmirror/distfiles/${APITRACE_W32_NAME} \
 apitracew32.7z
ADD http://commondatastorage.googleapis.com/chromeos-localmirror/distfiles/${APITRACE_W64_NAME} \
 apitracew64.7z
RUN 7z x apitracew32.7z
RUN 7z x apitracew64.7z
RUN mv apitrace-${APITRACE_WIN_VER}-win32 apitracew32
RUN mv apitrace-${APITRACE_WIN_VER}-win64 apitracew64
#
# Delete all the PDB files found in the Windows apitrace packages.
# This line can be commented out if the apitrace Windows debug symbols
# are needed.
RUN find . -name "*.pdb" -delete

#******************************************************************************
#                                                          gfxreconstruct-build
#******************************************************************************
FROM gfx-builder AS gfxreconstruct-build

COPY packages/cros-gfxreconstruct/ /scratch/cros-gfxreconstruct/
# Allow makepkg to be run as nobody.
RUN chgrp -R nobody /scratch && chmod -R g+ws /scratch
USER nobody
WORKDIR /scratch/cros-gfxreconstruct
# TODO(davidriley): Avoid --skippgpcheck.
RUN makepkg --skippgpcheck --syncdeps --noconfirm
RUN cp /scratch/cros-gfxreconstruct/*.zst "$ARTIFACTS"

#******************************************************************************
#                                                    lib32-gfxreconstruct-build
#******************************************************************************
FROM gfx-builder AS lib32-gfxreconstruct-build

COPY packages/lib32-cros-gfxreconstruct/ /scratch/lib32-cros-gfxreconstruct/
# Allow makepkg to be run as nobody.
RUN chgrp -R nobody /scratch && chmod -R g+ws /scratch
USER nobody
WORKDIR /scratch/lib32-cros-gfxreconstruct
# TODO(davidriley): Avoid --skippgpcheck.
RUN makepkg --skippgpcheck --syncdeps --noconfirm
RUN cp /scratch/lib32-cros-gfxreconstruct/*.zst "$ARTIFACTS"

#******************************************************************************
#                                                    gfxreconstruct-win-builder
#******************************************************************************
# Similar to Windows apitrace builder, this ensures that unzip is installed
# We will use this to extract the Windows gfxreconstruct binaries
FROM builder AS gfxreconstruct-win-builder
RUN pacman -S --noconfirm --disable-download-timeout \
    unzip

#******************************************************************************
#                                                      gfxreconstruct-win-build
#******************************************************************************
FROM gfxreconstruct-win-builder AS gfxreconstruct-win-build
ENV GFXRECONSTRUCT_WIN_VERSION=v0.9.18
ADD http://commondatastorage.googleapis.com/chromeos-localmirror/distfiles/gfxreconstruct-${GFXRECONSTRUCT_WIN_VERSION}-windows-msvc.zip \
 gfxreconstruct.zip
RUN unzip gfxreconstruct.zip
RUN mv gfxreconstruct-${GFXRECONSTRUCT_WIN_VERSION} gfxreconstruct-win

#******************************************************************************
#                                                               glbench-builder
#******************************************************************************
# glbench-builder has prerequisites to build glbench.
FROM gfx-builder AS glbench-builder

# Install packages already built.
WORKDIR /scratch
COPY --from=lib32-waffle-build /artifacts/*.zst pkgs/
COPY --from=mesa-build /artifacts/*.zst pkgs/
COPY --from=waffle-build /artifacts/*.zst pkgs/
RUN pacman -U --noconfirm pkgs/*.zst \
    && pacman -S --noconfirm \
        gflags \
        libepoxy \
        libpng

#******************************************************************************
#                                                                 glbench-build
#******************************************************************************
# glbench-build builds glbench-git package.
FROM glbench-builder AS glbench-build

# Sync package.
COPY packages/glbench-git/ /scratch/glbench-git/
WORKDIR /scratch/glbench-git

# Allow makepkg to be run as nobody.
RUN chgrp -R nobody . && chmod -R g+ws .

# Perform the build as nobody.
USER nobody
RUN makepkg
RUN cp *.zst "$ARTIFACTS"

#******************************************************************************
#                                                                    xray-build
#******************************************************************************
# xray-build builds xray-git package.
FROM glbench-builder AS xray-build

# Sync package.
COPY packages/xray-git/ /scratch/xray-git/
WORKDIR /scratch/xray-git

# Allow makepkg to be run as nobody.
RUN chgrp -R nobody . && chmod -R g+ws .

# Perform the build as nobody.
USER nobody
RUN makepkg
RUN cp *.zst "$ARTIFACTS"

#******************************************************************************
#                                                               vkbench-builder
#******************************************************************************
# vkbench-builder has prerequisites to build vkbench.
FROM glbench-builder AS vkbench-builder

# Install packages already built.
WORKDIR /scratch
COPY --from=lib32-waffle-build /artifacts/*.zst pkgs/
COPY --from=mesa-build /artifacts/*.zst pkgs/
COPY --from=waffle-build /artifacts/*.zst pkgs/
RUN pacman -U --noconfirm pkgs/*.zst \
    && pacman -S --noconfirm \
        fmt \
        vulkan-headers

#******************************************************************************
#                                                                 vkbench-build
#******************************************************************************
# vkbench-build builds vkbench-git package.
FROM vkbench-builder AS vkbench-build

# Sync package.
COPY packages/vkbench-git/ /scratch/vkbench-git/
WORKDIR /scratch/vkbench-git

# Allow makepkg to be run as nobody.
RUN chgrp -R nobody . && chmod -R g+ws .

# Perform the build as nobody.
USER nobody
RUN makepkg
RUN cp *.zst "$ARTIFACTS"

#******************************************************************************
#                                                                   cpuid-build
#******************************************************************************
FROM builder AS cpuid-build

# Sync package.
COPY packages/cpuid/ /scratch/cpuid/
WORKDIR /scratch/cpuid

RUN chgrp -R nobody . && chmod -R g+ws .

USER nobody
RUN makepkg
RUN cp *.zst "$ARTIFACTS"

#******************************************************************************
#                                                                  deqp-builder
#******************************************************************************
# deqp-builder has prerequisites to build deqp.
FROM gfx-builder AS deqp-builder
COPY --from=lib32-waffle-build /artifacts/*.zst pkgs/
COPY --from=waffle-build /artifacts/*.zst pkgs/
RUN pacman -U --noconfirm pkgs/*.zst \
    && pacman -S --noconfirm cargo

#******************************************************************************
#                                                                    deqp-build
#******************************************************************************
# build deqp-vk and deqp-runner binaries
FROM deqp-builder AS deqp-build
ARG COMMIT_deqp
ARG COMMIT_spirv_tools
ARG COMMIT_spirv_headers
ARG COMMIT_glslang
ARG COMMIT_amber
ARG COMMIT_nvidia_video_samples
ARG VERSION_deqp_runner
ARG ZLIB_VERSION
ARG ZLIB_CHECKSUM
WORKDIR /scratch
RUN git clone https://github.com/KhronosGroup/VK-GL-CTS.git
WORKDIR /scratch/VK-GL-CTS
# Replace the commit values in external/fetch_sources.py with the ones
# from ARGS, which are taken from ebuilds. (see build-deqp.sh)
RUN git checkout "$COMMIT_deqp" \
    && sed '/SPIRV-Tools.git/!b;n;n;c\\t\t\"'$COMMIT_spirv_tools'\",' \
        -i external/fetch_sources.py \
    && sed '/glslang.git/!b;n;n;c\\t\t\"'$COMMIT_glslang'\",' \
        -i external/fetch_sources.py \
    && sed '/SPIRV-Headers.git/!b;n;n;c\\t\t\"'$COMMIT_spirv_headers'\",' \
        -i external/fetch_sources.py \
    && sed '/amber.git/!b;n;n;c\\t\t\"'$COMMIT_amber'\",' \
        -i external/fetch_sources.py \
    && sed '/vk_video_samples.git/!b;n;n;c\\t\t\"'$COMMIT_nvidia_video_samples'\",' \
        -i external/fetch_sources.py \
    && sed 's/zlib-.*.tar.gz/zlib-'$ZLIB_VERSION'.tar.gz/g' \
        -i external/fetch_sources.py \
    && sed '/zlib.net/!b;n;c\\t\t\"'$ZLIB_CHECKSUM'\",' \
        -i external/fetch_sources.py \
    && sh -c 'python3 external/fetch_sources.py' \
    && mkdir build
WORKDIR /scratch/VK-GL-CTS/build
RUN mkdir caselists \
    && echo "$(git describe --tags | grep -P -o '\d+.\d.\d.\d')" > version.txt \
    && touch flakes.txt && touch skips.txt && touch baseline.txt \
    && sh -c 'cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS=-m64 \
        -DCMAKE_CXX_FLAGS=-m64' \
    && sh -c 'make -j' \
    && find ../external/vulkancts/mustpass/main/vk-default/ -name "*.txt" \
        -exec cp "{}" caselists/ \; \
    && cat caselists/* | sort > caselists/vk.txt \
    && grep -i "\.smoke\." caselists/vk.txt > caselists/smoke.txt \
    && grep -i "\.wayland\." caselists/vk.txt > caselists/wayland.txt
RUN sh -c 'cargo install deqp-runner --version=$VERSION_deqp_runner'

#******************************************************************************
#                                                             sommelier-builder
#******************************************************************************
# sommelier-builder has prerequisites to build sommelier.
FROM builder AS sommelier-builder

# Install packages already built.
WORKDIR /scratch
COPY --from=mesa-build /artifacts/*.zst pkgs/
COPY --from=wayland-build /artifacts/*.zst pkgs/
RUN pacman -U --noconfirm pkgs/*.zst \
    && pacman -S --noconfirm \
        gtest \
        libevdev \
        libxkbcommon \
        pixman \
        protobuf \
        python-jinja

COPY --from=perfetto-sdk-build /artifacts/perfetto.h /usr/include/
COPY --from=perfetto-sdk-build /artifacts/libperfetto.a /usr/lib64/
COPY --from=perfetto-sdk-build /artifacts/perfetto.pc /usr/lib64/pkgconfig/

#******************************************************************************
#                                                               sommelier-build
#******************************************************************************
# sommelier-build builds sommelier package.
FROM sommelier-builder AS sommelier-build

# Sync package.
COPY packages/sommelier-git/ /scratch/sommelier-git/
WORKDIR /scratch/sommelier-git
COPY sommelier/ ./src/sommelier
COPY stamp/sommelier ./src/VERSION

# Allow makepkg to be run as nobody.
RUN chgrp -R nobody . && chmod -R g+ws . && chown -R nobody ./src/

# Perform the build as nobody.
USER nobody
RUN makepkg --noextract
RUN cp *.zst "$ARTIFACTS"

#******************************************************************************
#                                                                piglit-builder
#******************************************************************************
# piglit-builder has prerequisites to build piglit.
FROM gfx-builder AS piglit-builder

ARG PIGLIT_COMMIT
# Install packages already built.
WORKDIR /scratch
COPY --from=lib32-waffle-build /artifacts/*.zst pkgs/
COPY --from=mesa-build /artifacts/*.zst pkgs/
COPY --from=waffle-build /artifacts/*.zst pkgs/
RUN pacman -U --noconfirm pkgs/*.zst \
    && pacman -S --noconfirm \
        python-numpy \
        glu \
        libxkbcommon \
        ocl-icd \
        opencl-headers \
        vulkan-headers \
        cargo
COPY packages/piglit/*.patch "/scratch/"
RUN git clone https://gitlab.freedesktop.org/mesa/piglit.git \
    && cd piglit \
    && git checkout "${PIGLIT_COMMIT}" \
    && patch -p1 < /scratch/disable-vs_in.patch

#******************************************************************************
#                                                                  piglit-build
#******************************************************************************
# piglit-build builds cros-piglit packages.
FROM piglit-builder AS piglit-build

ARG VERSION_deqp_runner
RUN sh -c 'cargo install deqp-runner --version=$VERSION_deqp_runner'

# Sync packages.
WORKDIR piglit
RUN cmake \
        -DCMAKE_INSTALL_PREFIX=/scratch/piglit-build \
        -DCMAKE_INSTALL_LIBDIR=lib \
        . \
    && make -j \
    && make install -j \
    && cp /root/.cargo/bin/piglit-runner $ARTIFACTS \
    && cp -r /scratch/piglit-build/lib/piglit/* $ARTIFACTS \
    && cp $ARTIFACTS/tests/sanity.xml.gz $ARTIFACTS/tests/smoke.xml.gz

#******************************************************************************
#                                                                  vkmark-build
#******************************************************************************
# vkmark-build installs vkmark and dependency packages.
FROM gfx-builder AS vkmark-build

RUN rm /var/cache/pacman/pkg/*.zst \
    && pacman -Sw --noconfirm \
        assimp \
        xcb-util-wm \
        vkmark \
    && cp /var/cache/pacman/pkg/*.zst $ARTIFACTS/

#******************************************************************************
#                                                             tast-binary-build
#******************************************************************************
# tast-binary-build collects test binaries we built in Docker but not
# preinstalled in borealis runtime.
FROM initial AS tast-binary-build

WORKDIR /scratch
COPY --from=piglit-build /artifacts/ piglit/
COPY --from=deqp-build /scratch/VK-GL-CTS/build/ vulkancts/
COPY --from=deqp-build /root/.cargo/bin/deqp-runner vulkancts/
COPY --from=glbench-build /artifacts/*.zst glbench/
COPY --from=xray-build /artifacts/*.zst xray/
COPY --from=vkbench-build /artifacts/*.zst vkbench/
COPY --from=vkmark-build /artifacts/*.zst vkmark/

RUN find glbench -name "*.zst" -exec tar -xvf {} -C glbench \; \
    && rm glbench/*.zst \
    && mv glbench/usr/bin/* glbench/ \
    && find vkbench -name "*.zst" -exec tar -xvf {} -C vkbench \; \
    && rm vkbench/*.zst \
    && mv vkbench/usr/local/graphics/vkbench/* vkbench \
    && ls vkmark/*.zst | xargs -n1 tar -C vkmark -xvf \
    && rm vkmark/*.zst \
    && find xray -name "*.zst" -exec tar -xvf {} -C xray \; \
    && rm xray/*.zst \
    && mv xray/usr/bin/* xray

#******************************************************************************
#                                                                breakpad-build
#******************************************************************************
# breakpad-build builds breakpad tools.  Not included by default to avoid
# unneccesary builds.  Build breakpad-build and then use "docker cp" to copy
# the artifacts out of /scratch/install and to a VM.
FROM builder AS breakpad-build
WORKDIR /scratch
RUN git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
ENV PATH=/scratch/depot_tools:$PATH
RUN mkdir breakpad
WORKDIR /scratch/breakpad
RUN fetch breakpad
WORKDIR /scratch/breakpad/src
RUN ./configure && make -j && make install prefix=/scratch/install

#******************************************************************************
#                                                          gamemodetest-builder
#******************************************************************************
# gamemodetest-builder has prerequisites to build gamemodetest.
FROM gfx-builder AS gamemodetest-builder
# The mesa packages built above need to be installed prior to any other
# dependencies are fulfilled from the apt repos.
WORKDIR /scratch
COPY --from=mesa-build /artifacts/*.zst pkgs/
RUN pacman -U --noconfirm pkgs/*.zst \
    && pacman -S --noconfirm \
        glfw-x11 \
        sdl2 \
        libepoxy \
    && rm -rf pkgs
COPY gamemodetest /scratch/gamemodetest

#******************************************************************************
#                                                            gamemodetest-build
#******************************************************************************
# gamemodetest-build builds gamemodtest test program.
FROM gamemodetest-builder AS gamemodetest-build
WORKDIR /scratch/gamemodetest
RUN make all

#******************************************************************************
#                                                            results-db-builder
#******************************************************************************
# results-db-builder has prerequisites to build results-db.
FROM builder AS results-db-builder
WORKDIR /scratch
# TODO(davidriley): Mount from build_full.py
RUN git clone https://chromium.googlesource.com/chromiumos/config
RUN git clone https://chromium.googlesource.com/chromiumos/platform/graphics

#******************************************************************************
#                                                              results-db-build
#******************************************************************************
FROM results-db-builder AS results-db-build
WORKDIR /scratch
RUN cp -r \
    config/python \
    graphics/src/results_database \
    "$ARTIFACTS"
WORKDIR /scratch/graphics/src/results_database
RUN mkdir "$ARTIFACTS"/bin \
    && cp \
        record_machine_info.py \
        record_software_config.py \
        "$ARTIFACTS"/bin

#******************************************************************************
#                                                             audiotest-builder
#******************************************************************************
# audiotest-builder has prerequisites to build audiotest.
FROM builder AS audiotest-builder
WORKDIR /scratch
RUN pacman -S --noconfirm \
    alsa-lib

#******************************************************************************
#                                                               audiotest-build
#******************************************************************************
# audiotest-build builds loopback-latency and copies cyclic bench script
FROM audiotest-builder AS audiotest-build
COPY packages/audiotest-git /scratch/audiotest-git
WORKDIR /scratch/audiotest-git

# Allow makepkg to be run as nobody.
RUN chgrp -R nobody . && chmod -R g+ws .

# Perform the build as nobody
USER nobody
RUN makepkg && cp *.zst "$ARTIFACTS"

#******************************************************************************
#                                                                  xtrace-build
#******************************************************************************
FROM builder AS xtrace-build
COPY packages/xtrace-git /scratch/xtrace-git
WORKDIR /scratch/xtrace-git

# Allow makepkg to be run as nobody.
RUN chgrp -R nobody . && chmod -R g+ws .

# Perform the build as nobody.
USER nobody
RUN makepkg
RUN cp *.zst "$ARTIFACTS"

#******************************************************************************
#                                                                     vdf-build
#******************************************************************************
FROM builder AS vdf-build

# Build dependencies needed by vdf, not required for release.
RUN pacman -S --noconfirm \
    python-coverage \
    python-nose

COPY packages/vdf-git /scratch/vdf-git

WORKDIR /scratch/vdf-git
RUN chgrp -R nobody . && chmod -R g+ws .

# Perform the build as nobody.
USER nobody
RUN makepkg
RUN cp *.zst "$ARTIFACTS"

#******************************************************************************
#                                                           setup_bitmaps-build
#******************************************************************************
# setup_bitmaps-build builds setup_bitmaps
# TODO(b/244271286): add tests.
FROM builder AS setup_bitmaps-build

COPY setup_bitmaps /scratch/setup_bitmaps/
WORKDIR /scratch/setup_bitmaps
RUN make all

#******************************************************************************
#                                                      detailed-metrics-builder
#******************************************************************************
FROM rust-builder AS detailed-metrics-builder
COPY detailed_metrics /scratch/detailed_metrics

#******************************************************************************
#                                                        detailed-metrics-build
#******************************************************************************
FROM detailed-metrics-builder AS detailed-metrics-build
RUN cargo install --path /scratch/detailed_metrics --root /usr

#******************************************************************************
#                                                           insight-layer-build
#******************************************************************************
FROM gfx-builder AS insight-layer-build

COPY insight_layer /scratch/insight_layer
WORKDIR /scratch/insight_layer
RUN make

#******************************************************************************
#                                                                steamcmd-build
#******************************************************************************
FROM builder AS steamcmd-build
COPY packages/steamcmd /scratch/steamcmd
WORKDIR /scratch/steamcmd

# Allow makepkg to be run as nobody.
RUN chgrp -R nobody . && chmod -R g+ws .

# Perform the build as nobody.
USER nobody
RUN makepkg
RUN cp *.zst "$ARTIFACTS"

#******************************************************************************
#                                                            cros-trace-builder
#******************************************************************************
# cros-trace-builder has prerequisites to build cros-trace
FROM builder AS cros-trace-builder
RUN pacman -S --noconfirm protobuf

#******************************************************************************
#                                                              cros-trace-build
#******************************************************************************
# cros-trace-build builds cros-trace package
FROM cros-trace-builder AS cros-trace-build
COPY packages/cros-trace-git /scratch/cros-trace-git
WORKDIR /scratch/cros-trace-git

# Allow makepkg to be run as nobody.
RUN chgrp -R nobody . && chmod -R g+ws .

# perform the build as nobody
USER nobody
RUN makepkg && cp *.zst "$ARTIFACTS"

#******************************************************************************
#                                                                weston-builder
#******************************************************************************
# weston-builder has prerequisites to build weston
FROM gfx-builder AS weston-builder
COPY --from=mesa-build /artifacts/*.zst pkgs/
COPY --from=xwayland-build /artifacts/*.zst pkgs/
RUN pacman -U --noconfirm pkgs/*.zst \
    && pacman -S --noconfirm \
        cairo \
        colord \
        lcms2 \
        libinput \
        libjpeg-turbo \
        libva \
        libwebp \
        libxcursor \
        libxkbcommon \
        mtdev \
        pango \
        pipewire \
        seatd \
        xcb-util-cursor

#******************************************************************************
#                                                                  weston-build
#******************************************************************************
# weston-build builds weston and its test clients
FROM weston-builder AS weston-build
COPY packages/weston /scratch/weston
WORKDIR /scratch/weston

# Allow makepkg to be run as nobody.
RUN chgrp -R nobody . && chmod -R g+ws .

# perform the build as nobody
USER nobody
RUN makepkg && cp *.zst "$ARTIFACTS"

#******************************************************************************
#                                                             runtime_base_pkgs
#******************************************************************************
# runtime_base_pkgs contains all the base packages copied into /scratch/pkgs.
FROM initial AS runtime_base_pkgs
WORKDIR /scratch
COPY --from=apitrace-build /artifacts/*.zst pkgs/
COPY --from=audiotest-build /artifacts/*.zst pkgs/
COPY --from=lib32-mesa-build /artifacts/*.zst pkgs/
COPY --from=lib32-waffle-build /artifacts/*.zst pkgs/
COPY --from=lib32-wayland-build /artifacts/*.zst pkgs/
COPY --from=mesa-build /artifacts/*.zst pkgs/
COPY --from=vdf-build /artifacts/*.zst pkgs/
COPY --from=waffle-build /artifacts/*.zst pkgs/
COPY --from=wayland-build /artifacts/*.zst pkgs/
COPY --from=xwayland-build /artifacts/*.zst pkgs/

#******************************************************************************
#                                                                  runtime_pkgs
#******************************************************************************
# runtime_pkgs contains all the packages for symbolification copied into
# /scratch/pkgs.
FROM runtime_base_pkgs AS runtime_pkgs
WORKDIR /scratch
COPY --from=sommelier-build /artifacts/*.zst pkgs/

#******************************************************************************
#                                                                       symbols
#******************************************************************************
FROM runtime_pkgs AS symbols
COPY --from=breakpad-build /scratch/install/* install/

#******************************************************************************
#                                                                  runtime_base
#******************************************************************************
# The runtime base image has all the 3rd party software installed which
# we need for the borealis runtime. We separate the base runtime from
# the "true" runtime image so that the later parts of the build can be
# done hermetically
FROM runtime_base_pkgs AS runtime_base
WORKDIR /scratch
RUN rm pkgs/*-debug-*.zst && pacman -U --noconfirm pkgs/*.zst && rm -r pkgs

# xinput, evtest, joystick, xtrace
#  - Included for debugging purposes, not needed for correct operation.
#    TODO(cpelling): Consider removing xtrace/others to reduce image size.
# strace, lsof
#  - Included for tracing the X server, not needed for correct operation.

# Game-specific dependencies:
# - alsa-plugins - Don't Starve Together and games from Klei Entertainment
#   (pulled in along with lib32-alsa-plugins by steam)
# - pipewire-alsa - Don't Starve Together and games from Klei Entertainment
# - at-spi2-atk - Europa Universalis IV (pulled in by gtk3)
# - glu - Stoneshard, and likely other GameMaker Studio games
# - gtk3 - Europa Universalis IV
# - lib32-pipewire - Don't Starve, and games that use FMOD
# - libbsd - Dead Cells
# - libibus - Metro Exodus (pulled in by sdl2)
# - libnghttp2 - Loop Hero, Hero Siege
# - libjpeg-turbo - Needs to be reinstalled to populate /usr/share/doc
# - libxss - Dead Cells
# - libvorbis - Dead Cells
# - libxcrypt-compat - Loop Hero
# - openal - audio for Stoneshard, and likely other GameMaker Studio games
# - pulseaudio - Proton, and games that use PulseAudio
# - rtmpdump - Loop Hero (librtmp.so.1)
# - speex - Master of Orion 2 (Dosbox)
# - systemd - Needs to be reinstalled to populate /usr/share/doc
# - xorg-xrandr - Slay the Spire, LWJGL (Lightweight Java Game Library)
#   (pulled in by steam)
# - xz - Needs to be reinstalled to populate /usr/share/doc

# Test-specific dependencies:
# - fmt - GLBench and VKBench string formatting.
# - gflags - GLBench and VKBench argument parsing tool.
# - numactl - Dependency for cyclictest from rt-tests.
# - rt-tests - Real-time test utilities.
# - sox - Sound processing utilities.

# Other dependencies:
# - rsync - required for shader cache migration from the guest to host virtiofs
#           TODO(b/282080100): deprecate this dependency

## Remove some unnecessary packages
##  - packagekit is actually expected by garcon, but in borealis we want that
##    expectation to fail quickly rather than succeed/fail slowly.
#RUN apt-get -y remove \
#        packagekit \
#    && apt-get -y autoremove
RUN pacman -Su --noconfirm --disable-download-timeout \
    acpid \
    alsa-utils \
    btrfs-progs \
    dbus \
    evtest \
    fmt \
    gflags \
    glfw-x11 \
    glu \
    gtk3 \
    joystick \
    lib32-pipewire \
    libbsd \
    libevdev \
    libjpeg-turbo \
    libnghttp2 \
    libvorbis \
    libxcb \
    libxcrypt-compat \
    lib32-libxcrypt-compat \
    libxss \
    lsof \
    mesa-utils \
    numactl \
    openal \
    pciutils \
    pulseaudio \
    pipewire \
    pipewire-alsa \
# For the BDT report uploader.
    python-google-auth-oauthlib \
# For results database.
    python-protobuf \
    python-requests-toolbelt \
    python-six \
# For running X11 app fakes.
    python-xlib \
    rsync \
    rtkit \
    rtmpdump \
    rt-tests \
    sdl2 \
# TODO(b/325093294): Remove socat once missing game logs is fixed in the
# latest Proton stable version.
    socat \
    sox \
    speex \
    strace \
    systemd \
    ttf-liberation \
# For vulkaninfo and vkcube
    vulkan-tools \
    wget \
    which \
    wireplumber \
    xdg-utils \
    xorg-xdpyinfo \
    xorg-xdriinfo \
    xorg-xev \
    xorg-xinput \
    xorg-xkbutils \
# For trace_window_system.sh
    xorg-xlsatoms \
# For xwindump.py, used in feedback reports
    xorg-xwininfo \
    xz \
# For file choosing
    xdg-desktop-portal \
    xdg-desktop-portal-gtk
# b/315759185: Delete command line tool frequently implicated in CVEs.
RUN rm -fv /usr/sbin/gif2rgb /usr/bin/gif2rgb

# TODO(b/333416855): Temporarily work around incompatible Steam package
# with -chromeos option.
ARG STEAM_PKG=steam-1.0.0.78-2-x86_64.pkg.tar.zst
RUN pacman --noconfirm -U \
    "https://storage.googleapis.com/chromeos-mirror/archlinux/multilib/os/x86_64/$STEAM_PKG"

# Copy over the binaries and scripts for traces
ENV APITRACEW32RETRACE=/scratch/apitracew32/bin/d3dretrace.exe
ENV APITRACEW64RETRACE=/scratch/apitracew64/bin/d3dretrace.exe
COPY --from=apitrace-win-build ${APITRACEW32RETRACE} \
     /opt/win_tools/apitrace/i386/
COPY --from=apitrace-win-build ${APITRACEW64RETRACE} \
     /opt/win_tools/apitrace/x86_64/
COPY win_tools/bin/exerun.py /opt/win_tools/bin/
COPY win_tools/bin/trace_capture.py /opt/win_tools/bin/

# Install first-party packages last, since they change often and won't be
# pulled in as dependencies of third-party packages. This allows us to cache
# more of the previous steps when iterating locally.
COPY --from=sommelier-build /artifacts/*.zst pkgs/
RUN rm pkgs/*-debug-*.zst && pacman -U --noconfirm pkgs/*.zst && rm -r pkgs

#******************************************************************************
#                                                                       runtime
#******************************************************************************
# The runtime image is the actual borealis product
FROM runtime_base AS runtime

ARG disable_arch_sig_validation

# Create users and groups for various services.
#
# Note: When started from the UI, Steam and games run under the set of groups
# specified in etc/init.d/cros-garcon. Be sure to update that file when
# adding chronos to a new group.
#
RUN useradd chronos -m \
    && passwd -d chronos \
    && groupadd traced-producer \
    && groupadd traced-consumer \
    && groupadd crash-access \
    && usermod -a -G audio,input,render,video,traced-producer,traced-consumer \
        chronos \
    && useradd metrics \
    && passwd -d metrics \
    && useradd syslog \
    && passwd -d syslog \
    && echo "Users created" \
# Move systemd to backup folder to allow maitred as init
    && mkdir -v systemd \
    && mv -v /sbin/init systemd \
    && ln -sv /opt/google/cros-containers/bin/maitred /sbin/init \
    && echo "systemd replaced with maitred" \
# Install links to Termina tools (these are initially broken links, the
# actual binaries are added later)
    && ln -sv /opt/google/cros-containers/bin/core2md /usr/bin/core2md \
    && ln -sv /opt/google/cros-containers/bin/vm_syslog /bin/vm_syslog \
    && ln -sv /opt/google/cros-containers/bin/vshd /bin/vshd \
# On VM boot, we'll generate a machine-id for DBus and store it in stateful,
# if one doesn't already exist. To make it available to apps, create a symlink
# from its standard location in /etc.
    && ln -sv /mnt/stateful/machine-id /etc/machine-id \
# Remove all non-Steam desktop icons.
    && mv -v /usr/share/applications/steam.desktop ./steam_tmp \
    && rm -v /usr/share/applications/*.desktop \
    && mv -v ./steam_tmp /usr/share/applications/steam.desktop \
    && echo "Non-Steam desktop icons removed" \
# /mnt/empty: for jailing the logging daemon
# /mnt/stateful: writable home dir
# /mnt/external: for mounting external drives
    && mkdir -vp \
         /mnt/empty \
         /mnt/external \
         /mnt/shared \
         /mnt/stateful \
         /usr/local/share/fonts \
         /var/lib/crash_reporter \
         /var/spool/crash \
# allow vshd to log in as root
    && echo "CHROMEOS_RELEASE_TRACK=testimage-channel" >> /etc/lsb-release \
    && echo "CHROMEOS_RELEASE_TRACK set" \
# Identify the guest board for crash_reporter.
# Note this relates to the guest image ("borealis", analogous to Crostini's
# "tatl" and "tael"), not the host board (volteer-borealis/volteer/brya/etc.)
# As with tatl/tael, this lets us identify Borealis guest crashes.
    && echo "CHROMEOS_RELEASE_BOARD=borealis" >> /etc/lsb-release \
    && echo "CHROMEOS_RELEASE_BOARD set" \
# Ensure Steam is the only desktop icon
    && bash -c '[ "$(ls /usr/share/applications/*.desktop)" == \
                  "/usr/share/applications/steam.desktop" ] || \
                { echo "Extra desktop icons present"; exit 1; }' \
# Ensure Steam is installed at the expected location.
    && bash -c '[[ -L "/usr/bin/steam" ]] || \
                { echo "/usr/bin/steam missing"; exit 1; }' \
    && bash -c '[[ "$(readlink -f /usr/bin/steam)" == \
                   "/usr/bin/steam-runtime" ]] || \
                { echo "Steam symlink bad"; exit 1; }' \
    && bash -c '[[ -f "/usr/lib/steam/bin_steam.sh" ]] || \
                { echo "bin_steam.sh missing"; exit 1; }' \
    && echo "Steam entry points checked" \
# Swap in our Steam wrapper script.
    && rm -v /usr/bin/steam-runtime \
    && echo "Steam wrapper swapped in" \
# Set default web browser to be garcon.
    && xdg-mime default garcon_host_browser.desktop x-scheme-handler/http \
    && xdg-mime default garcon_host_browser.desktop x-scheme-handler/https \
    && xdg-mime default garcon_host_browser.desktop scheme-handler/http \
    && xdg-mime default garcon_host_browser.desktop scheme-handler/https \
    && echo "garcon set as default web browser" \
# Fix the name of the steam application (see b/216192661).
    && sed -i 's/Steam (Runtime)/Steam/' /usr/share/applications/steam.desktop \
    && echo "Steam application name fixed" \
    && echo "LC_ALL=en_US.UTF-8" >> /etc/environment \
    && echo "en_US.UTF-8 UTF-8" >> /etc/locale.gen \
    && echo "LANG=en_US.UTF-8" > /etc/locale.conf \
    && locale-gen en_US.UTF-8 \
    && echo "Locale set and generated"
# Add host browser desktop file.
COPY garcon_host_browser.desktop /usr/share/applications
# Add the termina tools. We do this last so that we can re-use the
# above phases when the tools change.
ADD termina_tools.tbz /opt/google/cros-containers
COPY --from=perfetto-build /artifacts/tracebox /usr/sbin/
RUN ln -s /usr/sbin/tracebox /usr/bin/perfetto
COPY --from=gamemodetest-build /scratch/gamemodetest/gamemodealloc /usr/bin
COPY --from=gamemodetest-build /scratch/gamemodetest/gamemodeglalloc /usr/bin
COPY --from=gamemodetest-build /scratch/gamemodetest/gamemodewin /usr/bin
COPY --from=setup_bitmaps-build /scratch/setup_bitmaps/setup_bitmaps /usr/bin
COPY --from=results-db-build /artifacts/bin/* /usr/bin
COPY --from=detailed-metrics-build /usr/bin/detailed_metrics /usr/bin/
COPY --from=insight-layer-build \
    /scratch/insight_layer/libInsightLayer.so /usr/lib/
COPY --from=insight-layer-build /scratch/insight_layer/layer.json \
    /usr/share/vulkan/explicit_layer.d/VkLayer_INSIGHT_layer.json
# Set up /etc
COPY etc/ /etc/
# If --disable-arch-sig-validation is set change the siglevel symlink
# to point to the more permissive siglevel.trustall
RUN if [[ ! -z "${disable_arch_sig_validation}" \
          && "${disable_arch_sig_validation}" == 'true' ]]; then \
  ln -sf /etc/pacman.d/siglevel.trustall /etc/pacman.d/siglevel; fi
RUN mv /opt/google/cros-containers/etc/* /etc \
    && rmdir /opt/google/cros-containers/etc
# Set up custom scripts
COPY usr/bin/ /usr/bin/
# Install bdt Python module (using setuptools, to avoid needing to install pip)
RUN pacman -S --noconfirm python-setuptools

# workaround (b/382327769) - Remove the LICENSE file as it symbolic
# link to a non-existent file (and will stop the license tar package
# step). Since this package is under an MIT license, it is not needed
RUN rm /usr/share/licenses/python-setuptools/LICENSE

COPY bdt /scratch/bdt
RUN cd /scratch/bdt \
    && protoc --proto_path=src/ --python_out=src/ bdt_lib/quirks.proto \
    && python3 -c 'import setuptools; setuptools.setup()' install \
    && rm -r /scratch/bdt
COPY --from=results-db-build /artifacts/python /scratch/python
COPY --from=results-db-build /artifacts/results_database \
    /scratch/results_database
RUN cd /scratch/python \
    && python3 setup.py install \
    && rm -r /scratch/python
RUN cd /scratch/results_database \
    && python3 setup.py install \
    && rm -r /scratch/results_database
# Steam does not support chained-symlinks
# (https://github.com/ValveSoftware/steam-for-linux/issues/5609).
# Therefore, we bind-mount zoneinfo files onto /etc/localtime in runtime in
# maitred. In order to bind-mount, we need a starting file.
# TODO(b/237960004): Clean this up after Steam supports chained symlinks.
RUN cp /usr/share/zoneinfo/UTC /etc/localtime

#******************************************************************************
#                                                               runtime_systemd
#******************************************************************************
# The runtime image as systemd running as PID 1, instead of maitred
FROM runtime AS runtime_systemd
# Enable maitred for bootup
RUN mv /scratch/systemd/init /sbin/ \
    && systemctl enable maitred vshd \
# Remove maitred's journald serivce files
    && rm /etc/maitred/06-journal-dir.textproto \
        /etc/maitred/07-journal-syslog.textproto \
        /etc/maitred/08-journald.textproto \
        /etc/maitred/09-wait-for-journald.textproto \
    && /usr/bin/stamp-lsb-release.sh --prod runtime_systemd

#******************************************************************************
#                                                                  runtime_prod
#******************************************************************************
# The production image merely adds some metadata, and cleans up development
# headers and static libraries, and the package cache.
FROM runtime AS runtime_prod
# Remove systemd from production image
RUN rm -rf /scratch/systemd \
    && /usr/bin/stamp-lsb-release.sh --prod runtime_prod
#******************************************************************************
#                                                                  runtime_test
#******************************************************************************
# The test image adds a few useful packages and different metadata
# from the production one.
# gfxreconstruct - Vulkan tracing
FROM runtime AS runtime_test
COPY --from=gfxreconstruct-build /artifacts/*.zst pkgs/
COPY --from=xtrace-build /artifacts/*.zst pkgs/
COPY --from=steamcmd-build /artifacts/*.zst pkgs/
COPY --from=glbench-build /artifacts/*.zst pkgs/
COPY --from=xray-build /artifacts/*.zst pkgs/
COPY --from=vkbench-build /artifacts/*.zst pkgs/
COPY --from=vkmark-build /artifacts/*.zst pkgs/
COPY --from=weston-build /artifacts/*.zst pkgs/
COPY --from=lib32-gfxreconstruct-build /artifacts/*.zst pkgs/
COPY --from=cpuid-build /artifacts/*.zst pkgs/
RUN pacman -U --noconfirm pkgs/*.zst && rm -r pkgs
RUN pacman -S --noconfirm \
        binutils \
        gamescope \
        gdb \
        iperf \
        iperf3 \
        fio \
        lib32-mangohud \
        mangohud \
        net-tools \
        openssh \
        python-pefile \
        python-pip \
        tmux \
        valgrind \
        vim

# TODO(b/322038583) Remove this workaround when the fix is merged:
#   https://gitlab.archlinux.org/archlinux/packaging/packages/python-pillow/-/merge_requests/1
RUN cd /usr/share/licenses/python-pillow; \
    ln -sfv /usr/lib/python*/site-packages/pillow-*/LICENSE

ENV APITRACEW32=/scratch/apitracew32
ENV APITRACEW64=/scratch/apitracew64
COPY --from=apitrace-win-build ${APITRACEW32} /opt/win_tools/apitrace/i386
COPY --from=apitrace-win-build ${APITRACEW64} /opt/win_tools/apitrace/x86_64
COPY --from=gfxreconstruct-win-build /scratch/gfxreconstruct-win \
 /opt/win_tools/gfxreconstruct
COPY win_tools/bin /opt/win_tools/bin

RUN echo "chronos:test0000" | chpasswd
COPY etc_test/ /etc/
RUN /usr/bin/ssh-keygen -A
RUN /usr/bin/stamp-lsb-release.sh runtime_test

#******************************************************************************
#                                                                 runtime_debug
#******************************************************************************
# The debug image sets everything needed to build mesa in the VM and connect
# with a remote debugger (VSCode's Remote Development extension + SSH works
# very well).
FROM runtime_test AS runtime_debug
# Flag this as a debug build, so stamp-lsb-release.sh won't delete /usr/include
RUN echo "BOREALIS_DEBUG=1" >> /etc/lsb-release
RUN pacman -S --noconfirm \
        # Dev tools, for building Mesa and apitrace
        base-devel \
        cmake \
        git \
        multilib-devel \
        ninja \
        # Mesa dependencies
        libxrandr \
        lib32-libxrandr \
        llvm \
        lib32-llvm \
        meson \
        python-mako \
        python-yaml \
        # Xwayland dependencies
        xtrans \
        # gobject cairo and urllib are needed to launch steam with
        # PRESSURE_VESSEL_WRAP_GUI=1
        python-gobject \
        python-cairo \
        python-urllib3 \
        wayland-protocols \
        # Terminfo for Kitty terminal emulator (3 KB) [philpearson]
        kitty-terminfo \
        # Tiny emacs-like text editor (238 KB) [philpearson]
        mg \
        # Multithreaded gzip
        pigz \
        # iostat, vmstat, etc
        sysstat \
        # Vulkan debug tools
        vulkan-validation-layers \
        lib32-vulkan-validation-layers \
        # perfetto merge tracing (via cros-trace-tool)
        protobuf \
        ncurses

# Install packages already built.
WORKDIR /scratch
COPY --from=cros-trace-build /artifacts/*.zst pkgs/
RUN pacman -U --noconfirm pkgs/*.zst && rm -r pkgs

# Install debug packages.
WORKDIR /scratch
COPY --from=runtime_pkgs /scratch/pkgs/*-debug-*.zst pkgs/
RUN pacman -U --noconfirm pkgs/*.zst && rm -r pkgs

ENV DEV_TOOLS_DIR=/opt/dev_tools
COPY dev_tools/ "${DEV_TOOLS_DIR}"
COPY packages/cros-mesa-git/*.patch "${DEV_TOOLS_DIR}/patches/mesa/"
RUN chmod -R a+rx "${DEV_TOOLS_DIR}"
RUN : NOTE: Run '/opt/dev_tools/bin/build_mesa ~/mesa' in the Borealis VM to \
retain builds/settings across reboots.
RUN /usr/bin/stamp-lsb-release.sh runtime_debug
