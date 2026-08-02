# Copyright 2023 The ChromiumOS Authors
# Distributed under the terms of the BSD license.

EAPI=7

PYTHON_COMPAT=( python3_{7..11} )
# shellcheck disable=SC2034
DOTNET_PKG_COMPAT=8.0
inherit python-r1 multiprocessing cros-subtool nuget dotnet-pkg-base

DESCRIPTION="Renode is an open source software development framework with
commercial support from Antmicro that lets you develop, debug and test
multi-node device systems reliably, scalably and effectively."
HOMEPAGE="https://renode.io"

GIT_COMMIT=a3a273a80
MY_PV=${PV/_p/+}git${GIT_COMMIT}
RELEASE_PV="${PV%_*}"
SRC_URI="https://dl.antmicro.com/projects/renode/builds/renode_${MY_PV}_source.tar.xz"

LICENSE="MIT"
SLOT="0"
KEYWORDS="*"

DEPEND="
	dev-dotnet/dotnet-sdk
	dev-util/cmake
	$(python_gen_cond_dep '
		=dev-python/robotframework-4.0.1[${PYTHON_USEDEP}]
		dev-python/netifaces[${PYTHON_USEDEP}]
		dev-python/requests[${PYTHON_USEDEP}]
		dev-python/psutil[${PYTHON_USEDEP}]
		dev-python/pyyaml[${PYTHON_USEDEP}]
	')
"
RDEPEND="
	${DEPEND}
"

S="${WORKDIR}/renode_${MY_PV}_source"
PORTABLE_PACKAGE="${S}/output/packages/renode-${RELEASE_PV}.linux-portable.tar.gz"

pkg_setup() {
	dotnet-pkg-base_setup
}

src_unpack() {
	default

	(cd "${S}" && eapply "${FILESDIR}/Build-a-non-single-file-portable.patch")

	cat << EOF > "${S}/Directory.Build.targets"
<Project>
	<PropertyGroup>
		<TargetFrameworks>net8.0</TargetFrameworks>
	</PropertyGroup>
</Project>
EOF

	# shellcheck disable=SC2154
	cat << EOF > "${S}/NuGet.Config"
<?xml version="1.0" encoding="utf-8"?>
<configuration>
	<packageSources>
		<clear />
		<add key="local-packages" value="./nuget" />
		<add key="system-packages" value="${EPREFIX}${NUGET_SYSTEM_NUGETS}" />
	</packageSources>
</configuration>
EOF
}

src_compile() {
	tc-export BUILD_CC BUILD_PKG_CONFIG BUILD_AS

	export DOTNET_CLI_TELEMETRY_OPTOUT="1"
	export DOTNET_SKIP_FIRST_TIME_EXPERIENCE="1"
	export MSBUILDDISABLENODEREUSE="1"
	export UseSharedCompilation="false"

	# Passing -t (upstream flag) instructs build.sh to create a portable package
	# that can be distributed without the CrOS SDK.
	./build.sh -B linux-x64 --no-gui --net -t || die
}

src_install() {
	mkdir -p "${ED}/opt/renode"
	tar xf "${PORTABLE_PACKAGE}" -C "${ED}/opt/renode" --strip-components=1 \
		|| die
	dosym "${EPREFIX}/opt/renode/renode" /usr/bin/renode

	# Wrap renode-test: it's a script that doesn't resolve its $0.
	printf '#!/bin/sh\nexec "%s/opt/renode/renode-test" "$@"\n' "${EPREFIX}" \
		| newbin - renode-test

	cros-subtool_src_install
}

src_test() {
	# Unpack and test the portable package.
	tar xf "${PORTABLE_PACKAGE}" || die
	cd "renode_${RELEASE_PV}-portable" || die
	./renode --version || die
	./renode --console -e 'help; version; quit' || die

	# Run such unit tests that don't need to download any binaries.
	local test_script="${PWD}/renode-test"
	cd tests/unit-tests || die
	local tests=(
		AdHocCompiler/adhoc-compiler.robot
		arm-thumb.robot
		big-endian-watchpoint.robot
		emulation-environment.robot
		host-uart.robot
		llvm-disassemble.robot
		log-tests.robot
		memory-invalidation.robot
		opcodes-counting.robot
		riscv-custom-instructions.robot
		riscv-interrupt-mode.robot
		riscv-unit-tests.robot
		tb_overwrite.robot
	)
	${test_script} --stop-on-error -j "$(makeopts_jobs)" "${tests[@]}" || die
}
