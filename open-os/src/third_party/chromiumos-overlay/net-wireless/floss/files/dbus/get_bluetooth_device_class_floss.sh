#!/bin/bash
# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# This script queries D-Bus about the Bluetooth HID device type.

PROG_GET_ADAPTER_ENABLED="parse_adapter_enabled.awk"
PROG_GET_CLASS="parse_bluetooth_class.awk"

# Refer to the follow URL for bluetooth class of device and service fields.
# https://www.bluetooth.com/specifications/assigned-numbers/baseband
# Note that a mask is 24 bits long.
PERIPHERAL_MAJOR_MASK="0x000500"
PERIPHERAL_MINOR_MASK="0x0000C0"
KEYBOARD_DEVICE="0x000040"
POINTING_DEVICE="0x000080"
COMBO_DEVICE="0x0000C0"

# If Floss is not enabled by default, there is nothing to do.
ADAPTER_ENABLED_CMD="dbus-send --system --type=method_call --print-reply \
  --dest=org.chromium.bluetooth.Manager \
  /org/chromium/bluetooth/Manager \
  org.chromium.bluetooth.Manager.GetAdapterEnabled \
  int32:0"
ADAPTER_ENABLED="$(${ADAPTER_ENABLED_CMD} | ${PROG_GET_ADAPTER_ENABLED})"
if [[ ! ${ADAPTER_ENABLED} == "true" ]]; then exit; fi

# Remove the prefix and suffix quotes of the bluetooth device address,
# and convert it to upper case.
BD_ADDR="$1"
BD_ADDR="${BD_ADDR#\"}"
BD_ADDR="${BD_ADDR%\"}"
BD_ADDR="${BD_ADDR^^}"

# Construct the command to fetch the given device's class.
SEND_CMD_ARGS=(-c "device info ${BD_ADDR}")
BD_CLASS="$(btclient "${SEND_CMD_ARGS[@]}" \
          | ${PROG_GET_CLASS} addr="${BD_ADDR}")"
if [[ -n "${BD_CLASS}" &&
      $((BD_CLASS & PERIPHERAL_MAJOR_MASK)) -ne 0 ]]; then
  MINOR_DEVICE=$((BD_CLASS & PERIPHERAL_MINOR_MASK))
  case "${MINOR_DEVICE}" in
    $((KEYBOARD_DEVICE))) echo keyboard ;;
    $((POINTING_DEVICE))) echo mouse ;;
    $((COMBO_DEVICE))) echo combo ;;
  esac
fi
