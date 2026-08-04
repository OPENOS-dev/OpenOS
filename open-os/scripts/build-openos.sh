#!/bin/bash
# OPENOS ebuild builder — runs inside gentoo/stage3:arm64 Docker container
set -euo pipefail

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

PASS=0
FAIL=0
TOTAL=0

build_one() {
    local ebuild="$1"
    local name
    name="$(basename "$(dirname "$(dirname "$ebuild")")")/$(basename "$ebuild")"
    TOTAL=$((TOTAL + 1))
    
    echo ""
    echo "━━━ Building: ${name} ━━━"
    
    local tmpdir
    tmpdir=$(mktemp -d)
    trap "rm -rf '$tmpdir'" RETURN
    
    if ebuild "$ebuild" clean install 2>&1; then
        local count
        count=$(find "$tmpdir" -type f -o -type l 2>/dev/null | wc -l | tr -d ' ')
        echo "  ${GREEN}PASS${NC} (${count} files)"
        PASS=$((PASS + 1))
    else
        echo "  ${RED}FAIL${NC}"
        FAIL=$((FAIL + 1))
        return 1
    fi
}

# ── Setup minimal Gentoo environment ──
echo "=== Setting up Portage environment ==="

mkdir -p /var/db/repos/gentoo/{profiles,metadata}
echo "gentoo" > /var/db/repos/gentoo/profiles/repo_name
cat > /var/db/repos/gentoo/metadata/layout.conf << 'EOF'
masters =
repo-name = gentoo
EOF

# Minimal profile
mkdir -p /var/db/repos/gentoo/profiles/default/linux/arm64
echo "arm64" > /var/db/repos/gentoo/profiles/arch.list

mkdir -p /etc/portage/repos.conf /etc/portage/profile
cat > /etc/portage/repos.conf/gentoo.conf << 'EOF'
[DEFAULT]
main-repo = gentoo

[gentoo]
location = /var/db/repos/gentoo
auto-sync = no
EOF

cat > /etc/portage/repos.conf/openos.conf << 'EOF'
[openos]
location = /var/db/repos/openos
auto-sync = no
EOF

# Disable sandbox for Docker
mkdir -p /etc/portage/env
echo 'FEATURES="-sandbox -usersandbox -ipc-sandbox -network-sandbox -pid-sandbox -userpriv -usersync"' > /etc/portage/env/no-sandbox
cat > /etc/portage/package.env << 'EOF'
*/* no-sandbox
EOF

mkdir -p /var/cache/distfiles /var/tmp/portage
chmod 1777 /var/cache/distfiles 2>/dev/null || true

export ACCEPT_KEYWORDS="amd64 arm64 ~amd64 ~arm64 **"
export ACCEPT_LICENSE="*"
export FEATURES="-sandbox -usersandbox -ipc-sandbox -network-sandbox -pid-sandbox -userpriv -usersync"
export PORTAGE_USERNAME="root"
export PORTAGE_GRPNAME="root"

echo ""
echo "Portage $(portageq --version 2>/dev/null || emerge --version)"
echo "ARCH: $(portageq envvar ARCH 2>/dev/null || echo arm64)"
echo "ACCEPT_KEYWORDS: $ACCEPT_KEYWORDS"
echo ""

# ── Manifests pre-generated on host (overlay mounted read-only) ──
echo "=== Manifests (pre-generated) ==="
for dir in $(find /var/db/repos/openos -name Manifest | sort); do
    echo "  OK: $dir"
done

# ── Build all ebuilds ──
echo ""
echo "=== Building ebuilds ==="

for ebuild in $(find /var/db/repos/openos -name '*.ebuild' | sort); do
    build_one "$ebuild" || true
done

# ── Summary ──
echo ""
echo "========================================"
if [ $FAIL -eq 0 ]; then
    echo -e "  Result: ${GREEN}ALL ${PASS} PASSED${NC}"
else
    echo -e "  Result: ${GREEN}${PASS} passed${NC}, ${RED}${FAIL} failed${NC}"
fi
echo "========================================"
exit $FAIL
