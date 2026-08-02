# Copyright 2025 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

EAPI=7

CROS_WORKON_COMMIT="fdc5a7a7ec892f31ea4890b32fe2866c2c7265c9"
CROS_WORKON_TREE=("49b8b8cffcb9128d5b6b30e14bc2f78a35426905" "cb757d15fd6934f9e3811963942ae131e10bbb4a" "1eb3f377059d37f4a0bba32c493546a4ce04441b" "3817a1e54f940e2acd53be054df167d847b7c5e1")
CROS_WORKON_PROJECT="chromium/src/components/policy"
CROS_WORKON_LOCALNAME="chromium/src/components/policy"
CROS_WORKON_SUBTREE="tools/fake_dmserver resources/templates proto tools/generate_policy_source.py"

PYTHON_COMPAT=( python3_11 )
inherit cros-workon python-any-r1 cros-protobuf

DESCRIPTION="A tool to simplify local policy testing with fake_dmserver."
HOMEPAGE="https://chromium.googlesource.com/chromium/src/+/main/components/policy/tools/fake_dmserver/"

LICENSE="BSD-Google"
SLOT="0"
KEYWORDS="*"

RDEPEND="
	dev-python/protobuf-python
	chromeos-base/chrome-binary-tests
	chromeos-base/protofiles:=
	$(python_gen_any_dep '
		dev-python/pyyaml[${PYTHON_USEDEP}]
	')
"

DEPEND="
	chromeos-base/protofiles:=
"

# Staging directory for our compiled Python protos
PROTO_OUT_DIR="${WORKDIR}/local_tool_protos"
src_compile() {
	mkdir -p "${PROTO_OUT_DIR}"

	# Generate chrome_settings.proto.
	"${S}/tools/generate_policy_source.py" \
		--chrome-settings-protobuf="${WORKDIR}/chrome_settings.proto" \
		--policy-templates-file="${SYSROOT}/usr/share/policy_resources/generated_policy_templates.json" \
		--target-platform="chrome_os" \
		--all-chrome-versions \
		|| die "Failed to generate chrome_settings.proto"

	protoc \
		--proto_path="${S}/proto" \
		--proto_path="${SYSROOT}/usr/share/protofiles" \
		--proto_path="${WORKDIR}" \
		--python_out="${PROTO_OUT_DIR}" \
		"${WORKDIR}/chrome_settings.proto" \
		"${S}/proto/chrome_device_policy.proto" \
		"${S}/proto/device_management_backend.proto" \
		"${S}/proto/policy_common_definitions.proto" \
		"${SYSROOT}/usr/share/protofiles/private_membership_rlwe.proto" \
		"${SYSROOT}/usr/share/protofiles/private_membership.proto" \
		"${SYSROOT}/usr/share/protofiles/serialization.proto" \
		|| die "Proto compilation failed"
}

src_install() {
	local install_root="/usr/share/${PN}"

	insinto "${install_root}"
	doins "${S}/tools/fake_dmserver/README.md"
	doins "${S}/resources/templates/manual_device_policy_proto_map.yaml"

	exeinto "${install_root}"
	doexe "${S}/tools/fake_dmserver/orchestrator.py"
	doexe "${S}/tools/fake_dmserver/blob_generator.py"
	doexe "${S}/tools/fake_dmserver/policy_dump_converter.py"

	# Install our compiled protos. They are imported by the scripts.
	insinto "${install_root}"
	doins "${PROTO_OUT_DIR}"/*.py

	# Create __init__.py to make it a package.
	touch "${D}${install_root}/__init__.py" || die

	# The orchestrator is the main entry point.
	dosym "../share/${PN}/orchestrator.py" "/usr/bin/policy-test-tool-orchestrator.py" || die
}
