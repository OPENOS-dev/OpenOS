#!/bin/sh
# Copyright 2026 OCS (Open Code Studio)
# License: GPL-3.0

# Only set PAGER if it's not set already.
export PAGER="${PAGER:-$(command -v less)}"
