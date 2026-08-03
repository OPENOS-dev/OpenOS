# Copyright 2014 The ChromiumOS Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

DESCRIPTION="Init script to run agetty on selected terminals."

inherit systemd

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"

USE_PREFIX="tty_console_"

ALL_PORTS=(
	hvc{0..15}
	ttyAMA{0..5}
	ttyHSL{0..5}
	ttyMSM{0..5}
	ttymxc{0..5}
	ttyO{0..5}
	ttyPS{0..5}
	ttyS{0..5}
	ttySAC{0..5}
	ttyUSB{0..5}
	tty{0..5}
)

IUSE_PORTS=( "${ALL_PORTS[@]/#/${USE_PREFIX}}" )
IUSE="${IUSE_PORTS[*]} systemd tty-baud-1500000"

RDEPEND="
	systemd? ( sys-apps/systemd )
	!systemd? ( sys-apps/upstart )
	!chromeos-base/tty1
	!chromeos-base/serial-tty
"

S="${WORKDIR}"

getbaud() {
	usex tty-baud-1500000 "1500000" "115200"
}

src_compile() {
	# Generate a file for each activated tty console.
	local item

	elog "Using baud rate $(getbaud)"

	if use !systemd; then
		for item in "${IUSE_PORTS[@]}"; do
			use "${item}" && generate_init_script "${item}"
		done
	fi
}

generate_init_script() {
	# Creates an init script per activated console by copying the base script and
	# changing the port number.
	local port="${1#"${USE_PREFIX}"}"

	sed -e "s|%PORT%|${port}|g; s|%BAUD%|$(getbaud)|g" \
		"${FILESDIR}"/tty-base.conf \
		> "console-${port}.conf" || die "failed to generate ${port}"
}

src_install() {
	if [[ -n ${TTY_CONSOLE} ]]; then
		if use systemd; then
			sed -e "s|%PORT%|${port}|g; s|%BAUD%|$(getbaud)|g" \
				"${FILESDIR}/chromeos-tty@.service" \
				> chromeos-tty@.service || die "failed to generate systemd"

			systemd_dounit "chromeos-tty@.service"
			local item
			for item in "${IUSE_PORTS[@]}"; do
				if use "${item}"; then
					local port="${item#"${USE_PREFIX}"}"
					local unit_dir=$(systemd_get_systemunitdir)
					dosym  "../chromeos-tty@.service" \
						"${unit_dir}/boot-services.target.wants/chromeos-tty@${port}.service"
				fi
			done
		else
			insinto /etc/init
			doins console-*.conf
		fi
	fi
}
