# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

#!/bin/bash

print_message() {
    cat <<-END
NOTICE:
    Recently it has become necessary to deprecate some features of crostini,
    including:
      * UI-based installation of .deb packages. Installation via aptitude is
        the default path, but there are also visual package managers which
        may be used.
      * UI-based Debian release upgrade. Please refer to release notes for
        the release in question for upgrade instructions.
      * Multi-container support.

    For the most up-to-date news, please visit
    https://developers.google.com/chromeos/app-development/develop/news

END
}

deliver_motd() {
    # don't display in ssh-controlled shell
    [[ -n "${SSH_TTY:-}" ]] && return 0

    local COUNTER_INITIAL=5
    local COUNTER_MAX=10
    local user_data=${XDG_DATA_DIR:-"${HOME}/.local/share"}
    local motd_file="$user_data/cros-motd"

    # ensure exists
    mkdir -p "$user_data" 2>/dev/null || exit 1

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
            echo "    (to silence this message, run the following in this terminal):"
            echo "        echo $COUNTER_MAX >\"$motd_file\""
        fi
    fi
};

deliver_motd
