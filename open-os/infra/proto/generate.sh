#!/bin/bash -e
#
# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# Runs protoc over the protos in this repo to produce generated proto code.

CROS_CONFIG_REPO="https://chromium.googlesource.com/chromiumos/config"

readonly golden_file="gen/descriptors.json"

die() {
  1>&2 printf '%s\n' "$@"
  exit 1
}

regenerate_golden() {
    # We want to split --path from the filenames so silence warning.
    # shellcheck disable=2068
    buf build --exclude-imports -o -#format=json ${proto_paths[@]} \
        | jq -S > ${golden_file}
}


allow_breaking=0
regen_golden=0
while [[ $# -gt 0 ]]; do
    case $1 in
        --allow-breaking)
            allow_breaking=1
            shift
            ;;
        --force-regen-golden)
            regen_golden=1
            shift
            ;;
        *)
            break
            ;;
    esac
done

readonly script_dir="$(dirname "$(realpath -e "${BASH_SOURCE[0]}")")"

# By default we'll use the symlink to src/config.
config_dir=extern/
cros_config_subdir=""

if [[ "$(uname -s)" != "Linux" || "$(uname -m)" != "x86_64" ]]; then
  echo "Error: currently generate.sh can only run on Linux x86_64 systems."
  echo "This is because we use checked-in binaries for protoc-gen-go(-grpc)?."
  echo "This will change soon though. See https://crbug.com/1174238"
  exit 1
fi

cd "${script_dir}"
source "./setup_cipd.sh"

if [[ $regen_golden -eq 1 ]]; then
  echo "Forcing regenerating of golden proto descriptors."
  regenerate_golden
  exit 0
fi

# If we don't have src/config checked out too, then check out our own copy and
# stash it in the .generate directory.
if [ ! -e "extern/chromiumos" ]; then
  config_dir=./.generate
  cros_config_subdir="config/proto"

  # config available in infra checkout, use softlink
  if [ -e "../../config/proto" ]; then
    echo "Using local config dir at ../../config"
    mkdir -p .generate/config
    ln -s ../../../../config/proto .generate/config/proto
  # use repo version otherwise
  else
    echo "Creating a shallow clone of ${CROS_CONFIG_REPO}"
    git clone -q --depth=1 --shallow-submodules "${CROS_CONFIG_REPO}" \
        ".generate/config"
  fi

  trap "rm -rf .generate/*" EXIT
fi

echo "protoc version: $(protoc --version)"
echo "buf version:    $(buf --version 2>&1)"

#### protobuffer checks
mapfile -t proto_files < <(find src -type f -name '*.proto')
mapfile -t proto_paths < \
        <(find src -type f -name '*.proto' -exec echo '--path {} ' \;)

echo
echo "== Checking for breaking protobuffer changes"
if ! buf breaking --against ${golden_file}; then
    if [[ $allow_breaking -eq 0 ]]; then
      (
          echo
          cat <<-EOF
One or more breaking changes detected.  If these are intentional, re-run
with --allow-breaking to allow the changes.
EOF
      ) >&2
      exit 1
    else
      cat <<EOF >&2
One or more breaking changes, but --allow-breaking specified, continuing.
EOF
    fi
fi

echo "No breaking changes, regenerating '${golden_file}'"
# We want to split --path from the filenames so supress warning about quotes.
# shellcheck disable=2068
buf build --exclude-imports --exclude-source-info \
    -o -#format=json ${proto_paths[@]}            \
    | jq -S > ${golden_file}

# Check if golden file changed and offer to submit it for the user.
if ! git diff --quiet "${golden_file}"; then
  echo
  echo "Please commit ${golden_file} with your change"
else
  echo "Clean diff on ${golden_file}, nothing else to do." >&2
fi

echo
echo "== Linting protobuffers"

# We want to split --path from the filenames so supress warning about quotes.
# shellcheck disable=2068
if ! buf lint ${proto_paths[@]}; then
  echo "One or more files need cleanup" >&2
  exit 1 # failing lint should block presubmit as it prevents pb file gen
else
  echo "Files are clean"
fi

echo
echo "== Generating go bindings..."
# Clean up existing go bindings.
find go -name '*.pb.go' -exec rm '{}' \;
# Go files need to be processed individually until this is fixed:
# https://github.com/golang/protobuf/issues/39
for file in "${proto_files[@]}"; do
    protoc -Isrc -I"${config_dir}/${cros_config_subdir}" \
           --go_out=go/ --go_opt=paths=source_relative \
           --go-grpc_out=go/ --go-grpc_opt=paths=source_relative "${file}";
done
echo "== Formatting everything..."
test -d "./go" || die 'go dir does not exist'
gofmt -s -w "./go" || die 'failed to format source directory'

chromite_root="$(readlink -f "$(dirname "$0")/../..")"
chromite_api_compiler="${chromite_root}/api/compile_build_api_proto"
if [[ ${USER} = chrome-bot ]]; then
  echo "Not running chromite compiler"
elif [[ -x "${chromite_api_compiler}" ]]; then
  echo "Running chromite compiler"
  "${chromite_api_compiler}"
  echo "Don't forget to upload changes generated in ${chromite_root}, if any"
fi
