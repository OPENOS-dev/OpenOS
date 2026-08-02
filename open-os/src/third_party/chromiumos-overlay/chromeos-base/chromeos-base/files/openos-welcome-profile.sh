# OPENOS — welcome banner (animated on first boot, static thereafter)
# Sourced from /etc/profile.d/ on interactive login (VT2, SSH).
#
# Copyright 2026 OCS (Open Code Studio)
# License: GPL-3.0

[ -t 0 ] || return 0

if [ ! -f /var/lib/openos/first-boot-done ]; then
    bash /usr/bin/openos-welcome 2>/dev/null || true
else
    bash /usr/bin/openos-welcome --static 2>/dev/null || true
fi
