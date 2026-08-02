#!/bin/sh
# Copyright 2026 OCS (Open Code Studio)
# License: GPL-3.0

if [ "${EDITOR:+set}" != "set" ]; then
  EDITOR=$(
    # Ordered list of editors to check, based on user-experience.
    for editor in vim vi nano; do
      command -v "${editor}" && break
    done
  )
  export EDITOR
fi
