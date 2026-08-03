# Copyright 1999-2024 Gentoo Authors
# Distributed under the terms of the GNU General Public License v2

EAPI=7

# shellcheck disable=SC2034
DOTNET_PKG_COMPAT=$(ver_cut 1-2)
GENERIC_PKG_VERSION=4.3.0
NUGETS="
microsoft.aspnetcore.app.ref@${PV}
microsoft.aspnetcore.app.runtime.linux-arm@${PV}
microsoft.aspnetcore.app.runtime.linux-arm64@${PV}
microsoft.aspnetcore.app.runtime.linux-musl-arm@${PV}
microsoft.aspnetcore.app.runtime.linux-musl-arm64@${PV}
microsoft.aspnetcore.app.runtime.linux-musl-x64@${PV}
microsoft.aspnetcore.app.runtime.linux-x64@${PV}
microsoft.dotnet.ilcompiler@${PV}
microsoft.net.illink.tasks@${PV}
microsoft.net.sdk.webassembly.pack@${PV}
microsoft.netcore.app.host.linux-arm@${PV}
microsoft.netcore.app.host.linux-arm64@${PV}
microsoft.netcore.app.host.linux-musl-arm@${PV}
microsoft.netcore.app.host.linux-musl-arm64@${PV}
microsoft.netcore.app.host.linux-musl-x64@${PV}
microsoft.netcore.app.host.linux-x64@${PV}
microsoft.netcore.app.ref@${PV}
microsoft.netcore.app.runtime.linux-arm@${PV}
microsoft.netcore.app.runtime.linux-arm64@${PV}
microsoft.netcore.app.runtime.linux-musl-arm@${PV}
microsoft.netcore.app.runtime.linux-musl-arm64@${PV}
microsoft.netcore.app.runtime.linux-musl-x64@${PV}
microsoft.netcore.app.runtime.linux-x64@${PV}
runtime.linux-arm64.microsoft.dotnet.ilcompiler@${PV}
runtime.linux-musl-arm64.microsoft.dotnet.ilcompiler@${PV}
runtime.linux-musl-x64.microsoft.dotnet.ilcompiler@${PV}
runtime.linux-x64.microsoft.dotnet.ilcompiler@${PV}
runtime.any.system.collections@${GENERIC_PKG_VERSION}
runtime.any.system.diagnostics.tools@${GENERIC_PKG_VERSION}
runtime.any.system.globalization@${GENERIC_PKG_VERSION}
runtime.any.system.io@${GENERIC_PKG_VERSION}
runtime.any.system.reflection@${GENERIC_PKG_VERSION}
runtime.any.system.reflection.extensions@${GENERIC_PKG_VERSION}
runtime.any.system.reflection.primitives@${GENERIC_PKG_VERSION}
runtime.any.system.resources.resourcemanager@${GENERIC_PKG_VERSION}
runtime.any.system.runtime@${GENERIC_PKG_VERSION}
runtime.any.system.runtime.handles@${GENERIC_PKG_VERSION}
runtime.any.system.runtime.interopservices@${GENERIC_PKG_VERSION}
runtime.any.system.text.encoding@${GENERIC_PKG_VERSION}
runtime.any.system.text.encoding.extensions@${GENERIC_PKG_VERSION}
runtime.any.system.threading.tasks@${GENERIC_PKG_VERSION}
runtime.unix.microsoft.win32.primitives@${GENERIC_PKG_VERSION}
runtime.unix.system.diagnostics.debug@${GENERIC_PKG_VERSION}
runtime.unix.system.io.filesystem@${GENERIC_PKG_VERSION}
runtime.unix.system.runtime.extensions@${GENERIC_PKG_VERSION}
system.buffers@${GENERIC_PKG_VERSION}
system.private.uri@${GENERIC_PKG_VERSION}
runtime.unix.system.private.uri@${GENERIC_PKG_VERSION}
runtime.native.system.security.cryptography.openssl@${GENERIC_PKG_VERSION}
runtime.any.system.diagnostics.tracing@${GENERIC_PKG_VERSION}
runtime.debian.8-x64.runtime.native.system.security.cryptography.openssl@${GENERIC_PKG_VERSION}
runtime.fedora.23-x64.runtime.native.system.security.cryptography.openssl@${GENERIC_PKG_VERSION}
runtime.fedora.24-x64.runtime.native.system.security.cryptography.openssl@${GENERIC_PKG_VERSION}
runtime.opensuse.13.2-x64.runtime.native.system.security.cryptography.openssl@${GENERIC_PKG_VERSION}
runtime.opensuse.42.1-x64.runtime.native.system.security.cryptography.openssl@${GENERIC_PKG_VERSION}
runtime.osx.10.10-x64.runtime.native.system.security.cryptography.openssl@${GENERIC_PKG_VERSION}
runtime.rhel.7-x64.runtime.native.system.security.cryptography.openssl@${GENERIC_PKG_VERSION}
runtime.ubuntu.14.04-x64.runtime.native.system.security.cryptography.openssl@${GENERIC_PKG_VERSION}
runtime.ubuntu.16.04-x64.runtime.native.system.security.cryptography.openssl@${GENERIC_PKG_VERSION}
runtime.ubuntu.16.10-x64.runtime.native.system.security.cryptography.openssl@${GENERIC_PKG_VERSION}
system.diagnostics.tracing@${GENERIC_PKG_VERSION}
"

inherit dotnet-pkg-base

DESCRIPTION=".NET runtime nugets"
HOMEPAGE="https://dotnet.microsoft.com/"
# shellcheck disable=SC2154
SRC_URI="${NUGET_URIS}"
S="${WORKDIR}"

LICENSE="MIT-dotnet"
SLOT="${PV}/${PV}"
KEYWORDS="-* amd64 arm arm64"

src_unpack() {
	:
}

src_install() {
	local nuget
	for nuget in ${NUGETS} ; do
		nuget_donuget "${DISTDIR}/${nuget/@/.}.nupkg"
	done
}
