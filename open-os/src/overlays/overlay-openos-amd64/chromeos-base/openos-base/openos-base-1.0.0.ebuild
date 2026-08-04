# Copyright 2026 OCS (Open Code Studio)
# Distributed under the terms of the GNU General Public License v2

EAPI="7"

DESCRIPTION="OPENOS base system configuration (de-Googled ChromiumOS)"
HOMEPAGE="https://openos.org"
LICENSE="GPL-3 BSD-Google"
SLOT="0"
KEYWORDS="*"

RDEPEND="
	chromeos-base/common-assets
	chromeos-base/chromiumos-assets
"

DEPEND="${RDEPEND}"

src_install() {
	# === System Identity: lsb-release ===
	insinto /etc
	newins "${FILESDIR}/lsb-release" lsb-release

	# === OPENOS Logo ===
	insinto /usr/share/chromeos-assets/images
	doins "${FILESDIR}"/openos-logo_*.png
	doins "${FILESDIR}"/openos-logo.svg
	
	# === Product Logo for UI ===
	insinto /usr/share/chromeos-assets/images_100_percent
	doins "${FILESDIR}"/openos-logo_*.png
	
	insinto /usr/share/chromeos-assets/images_200_percent
	doins "${FILESDIR}"/openos-logo_*.png

	# === OPENOS Config ===
	insinto /etc/openos
	doins "${FILESDIR}/openos.conf"

	# === OPT Package Tool Config ===
	insinto /etc/opt
	doins "${FILESDIR}/opt-repos.conf"

	# === Desktop Entry ===
	insinto /usr/share/applications
	doins "${FILESDIR}/openos-about.desktop"

	# === Chrome Initial Preferences (Bing search + OPENOS defaults) ===
	insinto /opt/ocs/chrome
	newins "${FILESDIR}/initial_preferences" initial_preferences

	# === Chrome Device Policy (Bing search, privacy, homepage) ===
	insinto /etc/opt/ocs/policies/recommended
	newins "${FILESDIR}/openos-policy.json" openos.json

	# === Disable metrics consent by default ===
	dodir /etc/opt/ocs/policies/managed
	cat > "${D}/etc/opt/ocs/policies/managed/openos-metrics.json" <<- 'EOF'
	{
	  "MetricsReportingEnabled": false,
	  "UrlKeyedAnonymizedDataCollectionEnabled": false,
	  "SafeBrowsingExtendedReportingEnabled": false
	}
	EOF
}
