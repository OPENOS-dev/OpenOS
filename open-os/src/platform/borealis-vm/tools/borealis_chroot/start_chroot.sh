#!/bin/bash
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Starts up the Borealis chroot

# Apps in borealis attempt to connect to X0, but since XWayland
# already exists on host this needs to be symlinked.
mkdir -p /tmp/.X11-unix
ln -s /tmp/.X11-unix/X2 /tmp/.X11-unix/X0

# Smoke test.
sommelier \
    --display=/tmp/wayland-0 \
    --noop-driver \
    --force-drm-device=/dev/dri/renderD128 \
    -X \
    --glamor \
    --enable-linux-dmabuf \
    -- \
    glxinfo -B

# Start pulseaudio for games that require the server to be active.
# (b/267505902) Audio will still not work in these games, though.
pulseaudio &
PULSEAUDIO_PID="$!"

# Touch the dbus session flag file so /usr/bin/steam doesn't wait forever.
# We should probably actually start the dbus session instead.
touch /tmp/.chronos_dbus_session_env.sh

# Start a Steam-capable shell!
echo -e "\n\n\n**************************************************\n"
echo "Opening a shell; note that the Xwayland startup messages may"
echo "obscure the prompt."
echo "To run Steam, execute: steam -no-cef-sandbox"
echo -e "\n**************************************************\n\n\n"
sommelier \
    --display=/tmp/wayland-0 \
    --noop-driver \
    --force-drm-device=/dev/dri/renderD128 \
    -X \
    --glamor \
    --trace-system \
    --enable-linux-dmabuf \
    -- \
    /bin/bash

# Kill background processes before exiting
kill -9 "${PULSEAUDIO_PID}"
