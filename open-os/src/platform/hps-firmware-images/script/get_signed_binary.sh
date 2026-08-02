#!/usr/bin/env bash

set -euo pipefail
# set -x

MILESTONE="$1"
VERSION="$2"

ROOT_DIR=$(git rev-parse --show-toplevel)

mkdir -p "$ROOT_DIR/firmware-signed/$MILESTONE-$VERSION"
cd "$ROOT_DIR/firmware-signed/$MILESTONE-$VERSION"

GSUTIL_ROOT="gs://chromeos-releases/canary-channel/brya/$VERSION"
gsutil.py cp \
    "$GSUTIL_ROOT/chromeos_${VERSION}_brya_hps_firmware_canary-channel_hps-accessory-mp.bin" \
    mcu_stage1.bin
gsutil.py cp \
    "$GSUTIL_ROOT/ChromeOS-hps_firmware-$MILESTONE-$VERSION-brya.tar.bz2" \
    .

tar --strip-components=1 -xvf \
    "ChromeOS-hps_firmware-$MILESTONE-$VERSION-brya.tar.bz2" \
    hps/fpga_bitstream.bin hps/fpga_application.bin hps/manifest.txt

cp mcu_stage1.bin fpga_bitstream.bin fpga_application.bin manifest.txt ..
cd ..

git add mcu_stage1.bin fpga_bitstream.bin fpga_application.bin manifest.txt
git commit -m "Update signed binaries from $MILESTONE-$VERSION

BUG=
TEST=None
"

cat <<EOF
Reminder to test these files:
cros workon --board=brya start hps-firmware-images
emerge-brya hps-firmware-images
cros deploy dut hps-firmware-images && ssh dut restart hpsd
ssh dut stop hpsd ; start hpsd
ssh dut tail -f /var/log/messages | grep hps
EOF
