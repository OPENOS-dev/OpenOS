#!/bin/bash
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# borealis_chroot.sh: Run the Borealis rootfs in a chroot.
# A bit like Crouton, but just for Borealis.

# To run this:

# ./run_on_dut.sh address-of-your-dut

set -euo pipefail

CHROOT="/tmp/borealis"
HOMEDIR="/mnt/stateful_partition"

# Running on Linux (not ChromeOS).
LINUX=false
# Mount the stateful image file from an existing Borealis install.
MOUNT_BOREALIS_STATEFUL=false
# Btrfs external disk image to mount on /mnt/external/0.
EXTERNAL_DISK_IMAGE=""

while getopts 'l:mx:' flag; do
  case "${flag}" in
    l)
      LINUX=true
      HOMEDIR="${OPTARG}"
      ;;
    m) MOUNT_BOREALIS_STATEFUL=true ;;
    x) EXTERNAL_DISK_IMAGE="${OPTARG}" ;;
    *) error "Unexpected option ${flag}" ;;
  esac
done
shift $((OPTIND - 1))

ROOTFS_IMAGE="${HOMEDIR}/img_borealis/borealis_chroot_rootfs.img"

echo "Running Borealis in a chroot (${CHROOT})"

unmount_everything() {
    VTEST_SERVER_PID=$(cat /tmp/borealis/tmp/vtest-server-pid)
    if [ -n "${VTEST_SERVER_PID}" ]; then
        # A running vtest server can prevent unmounting of the borealis chroot
        echo "Stopping leftover vtest server in ${CHROOT}"
        pkill -P "${VTEST_SERVER_PID}"
        sleep 1 # Wait for vtest to shutdown
    fi
    echo "Unmounting everything under ${CHROOT}"
    if [ -d "${CHROOT}" ]; then
        umount -vR "${CHROOT}"
        rmdir "${CHROOT}"
    fi
}

# Clean up anything from a previous run.
unmount_everything || true

echo "Creating our mount point."
mkdir -p "${CHROOT}"
mount -o ro "${ROOTFS_IMAGE}" "${CHROOT}"

# Handles /mnt/stateful under reference Linux and prevents
# issues with bwrap under chroot by rebinding the root.
if [ "${LINUX}" = true ]; then
    mount --make-private "${CHROOT}"
    mount --bind "${CHROOT}" "${CHROOT}"
    mount -t tmpfs tmpfs "${CHROOT}/mnt/stateful"
fi

mount -t tmpfs tmpfs "${CHROOT}/tmp"
mount -t tmpfs tmpfs "${CHROOT}/run"
mount -t devtmpfs devtmpfs "${CHROOT}/dev"
mount -t proc proc "${CHROOT}/proc"
mount -t sysfs sysfs "${CHROOT}/sys"

# Allow mounting an external disk as /mnt/external/0
mount -t tmpfs tmpfs "${CHROOT}/mnt/external"
mkdir "${CHROOT}/mnt/external/0"
chown 1000:1000 "${CHROOT}/mnt/external/0"
if [ "${EXTERNAL_DISK_IMAGE}" != "" ]; then
    mount "${EXTERNAL_DISK_IMAGE}" "${CHROOT}/mnt/external/0"
fi

if [ "${LINUX}" = true ]; then
    touch "${CHROOT}/mnt/stateful/machine-id"
    mount --bind /etc/machine-id "${CHROOT}/mnt/stateful/machine-id"
fi

# Bind-mount Chrome's Wayland socket.  (Won't support pointer lock.)
touch "${CHROOT}/tmp/wayland-0"

if [ "${LINUX}" = true ]; then
    mount --bind /run/user/1000/wayland-0 "${CHROOT}/tmp/wayland-0"
else
    mount --bind /run/chrome/wayland-0 "${CHROOT}/tmp/wayland-0"
fi

# Copy groups so chronos has access to /dev/dri/renderD128.
cp /etc/group "${CHROOT}/tmp/group"
mount --bind "${CHROOT}/tmp/group" "${CHROOT}/etc/group"

# Bind-mount /home/chronos from dev_image, which is exempt from
# kernel symfollow protection.
echo "Setting up /home/chronos"
STATEFUL_HOME_CHRONOS="${HOMEDIR}/dev_image/borealis_home_chronos"
mkdir -p "${STATEFUL_HOME_CHRONOS}"
chown 1000:1000 "${STATEFUL_HOME_CHRONOS}"
mount --bind "${STATEFUL_HOME_CHRONOS}" "${CHROOT}/home/chronos"
mount -o remount,exec,symfollow,suid "${CHROOT}/home/chronos"

if [ "${MOUNT_BOREALIS_STATEFUL}" = true ]; then
    # Mount the existing stateful in the usual location.
    echo "Mounting stateful image from existing Borealis installation:"
    file /run/daemon-store/crosvm/*/Ym9yZWFsaXM=.img
    echo "If the mount fails and the previous line mentions that the image"
    echo "needs journal recovery, try:"
    echo "  fsck /run/daemon-store/crosvm/*/Ym9yZWFsaXM=.img"
    mount /run/daemon-store/crosvm/*/Ym9yZWFsaXM=.img "${CHROOT}/mnt/stateful"
    # Use ~/.local/share from the stateful rather than the local homedir.
    mkdir -p "${CHROOT}/home/chronos/.local/share"
    mount --bind "${CHROOT}/mnt/stateful/chronos/.local/share" \
        "${CHROOT}/home/chronos/.local/share"
fi

# Set up .config
DOTCONFIG="${CHROOT}/home/chronos/.config"
mkdir -p "${DOTCONFIG}"
chown chronos:chronos "${DOTCONFIG}"

# Bind-mount /run/perfetto so tracing works.
if [ "${LINUX}" = false ]; then
    echo "Perfetto tracing"
    mkdir "${CHROOT}/run/perfetto"
    mount --bind /run/perfetto/ "${CHROOT}/run/perfetto"
fi

# Bind-mounts for pulseaudio.
echo "Audio"

if [ "${LINUX}" = true ]; then
    mount --bind /var/lib/dbus "${CHROOT}/var/lib/dbus"
    mount --bind /dev/shm "${CHROOT}/dev/shm"
else
    # Bind-mount /run/cras so audio works.
    mkdir "${CHROOT}/run/cras"
    mount --bind /run/cras "${CHROOT}/run/cras"

    # Configure PulseAudio
    PULSEHOME="${CHROOT}/home/chronos/.config/pulse"
    mkdir -p "${PULSEHOME}"
    cat >"${PULSEHOME}/default.pa" <<EOF
#!/usr/bin/pulseaudio -nF
# Copyright (c) 2016 The crouton Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Include default configuration first
.include /etc/pulse/default.pa

# Forward audio to Chromium OS audio server
load-module module-alsa-sink device=cras sink_name=cras-sink
load-module module-alsa-source device=cras source_name=cras-source
set-default-sink cras-sink
set-default-source cras-source
EOF
    chown -R chronos:chronos "${PULSEHOME}"
fi


# Bind-mount resolv.conf so networking works.
echo "Networking"
touch "${CHROOT}/run/resolv.conf"
mount --bind /etc/resolv.conf "${CHROOT}/run/resolv.conf"

# Move bwrap and make it setuid root, as it is in Crouton.
echo "bwrap"
cp -a "${CHROOT}/usr/bin/bwrap" "${CHROOT}/home/chronos/"
mount --bind "${CHROOT}"/home/chronos/bwrap "${CHROOT}/usr/bin/bwrap"
chown root:root "${CHROOT}/usr/bin/bwrap"
chmod u+s "${CHROOT}/usr/bin/bwrap"

# Move strace and make it setuid root, so we can strace bwrap'd games.
echo "strace"
cp -a "${CHROOT}/usr/bin/strace" "${CHROOT}/home/chronos/"
mount --bind "${CHROOT}"/home/chronos/strace "${CHROOT}/usr/bin/strace"
chown root:root "${CHROOT}/usr/bin/strace"
chmod u+s "${CHROOT}/usr/bin/strace"

# Run cleanup code even if the chroot fails.
set +e

# The start_chroot.sh script was previously copied over by run_on_dut.sh so...
# Here we go!
# Note: `prlimit --nofile` raises the max number of open fds to match the VM.
#       This enables support of esync synchronization in Proton games.
#       This should match the limits in src/platform2/vm_tools/maitred/init.cc.
chroot "${CHROOT}" /bin/bash -c \
  'prlimit --nofile=1048576 su - chronos ./start_chroot.sh'

# Clean up after ourselves.
sleep 1
unmount_everything
