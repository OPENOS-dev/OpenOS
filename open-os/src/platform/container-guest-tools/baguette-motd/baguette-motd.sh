#!/bin/bash

# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

print_message() {
    cat <<-END
NOTICE:
    To simplify system architecture and maintenance, Crostini has switched
    by default to a containerless design for new environments starting in
    ChromeOS version 143 and newer.

    If you experience unexptected issues with the new design, please report
    them using the instructions available at
    https://www.chromium.org/chromium-os/developer-library/guides/bugs/platform-public-tracker/.

    If you would like to revert to the previous system architecture, you may
    visit chrome://flags#containerless-crostini in your Chrome browser and set
    the flag to "Disabled", then restart your device, and un- and reinstall
    Crostini.

END
}

deliver_motd() {
    # don't display in ssh-controlled shell
    [[ -n "${SSH_TTY:-}" ]] && return 0

    local COUNTER_INITIAL=0
    local COUNTER_MAX=5
    local user_data=${XDG_DATA_DIR:-"${HOME}/.local/share"}
    local motd_file="$user_data/baguette-motd"

    # ensure exists
    mkdir -p "$user_data" 2>/dev/null || return 1

    local counter=-1
    if [[ -f "$motd_file" ]]; then
        # get counter from file and ensure valid integer
        {
            counter=$(<"$motd_file") 2>/dev/null && [[ "$counter" =~ ^[0-9]+$ ]]
        } 2>/dev/null || counter=-1
    fi

    # ensure counter is within limits, or re-initialize
    if ((counter < COUNTER_INITIAL || counter > COUNTER_MAX)); then
        echo "$COUNTER_INITIAL" > ${motd_file}
        counter=$COUNTER_INITIAL
    fi

    if ((counter < COUNTER_MAX)); then
        counter=$((counter + 1))
        echo $counter > "$motd_file"

        print_message

        if ((counter == COUNTER_MAX)); then
            echo "    (this message will not be repeated again)"
        else
            ((counter == (COUNTER_MAX-1))) && s='' || s='s'
            remaining=$((COUNTER_MAX-counter))
            echo "    (this message will be repeated $remaining more time$s)."
            echo "    (to silence this message, run the following command):"
            echo "        echo $COUNTER_MAX >\"$motd_file\""
        fi
    fi
};

deliver_motd
