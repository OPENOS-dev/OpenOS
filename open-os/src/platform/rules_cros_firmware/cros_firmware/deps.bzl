# Copyright 2024 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

load("@bazel_tools//tools/build_defs/repo:git.bzl", _git_repository = "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", _http_archive = "http_archive")
load("//bazel/repo_defs:cipd.bzl", _cipd_zip_repo = "cipd_zip_repo")
load("//bazel/repo_defs:new_local_repository.bzl", _new_local_repository = "new_local_repository")
load("//platform/rules_cros_firmware/repositories:depot_tools.bzl", _depot_tools = "depot_tools")

# b/474065976: http_archive fails when extracting some sparse tar files.
# Workaround this by using the tar executable to do the extracting.
def _workaround_http_archive_impl(ctx):
    tarfile = ctx.attr.url.rsplit("/", 1)[-1]
    ctx.download(sha256 = ctx.attr.sha256, url = ctx.attr.url, output = tarfile)
    result = ctx.execute([ctx.which("tar"), "xf", tarfile])
    if result.return_code:
        fail("Couldn't extract tar {}:\nSTDOUT:\n{}\nSTDERR:\n{}".format(tarfile, result.stdout, result.stderr))
    ctx.file("BUILD.bazel", ctx.read(ctx.attr.build_file))

_workaround_http_archive = repository_rule(
    _workaround_http_archive_impl,
    doc = "Workaround for sparse tar errors b/474065976",
    attrs = {
        "build_file": attr.label(allow_single_file = True),
        "sha256": attr.string(),
        "url": attr.string(),
    },
)

def _opentitantool_binary_repo_impl(repo_ctx):
    # Find opentitan path
    opentitan_path = repo_ctx.workspace_root.get_child("third_party/lowrisc/opentitan")

    # Run bazel build //sw/host/opentitantool
    bazelisk = opentitan_path.get_child("bazelisk.sh")

    repo_ctx.report_progress("Building opentitantool...")

    # We need to pass PATH to execute so it can find tools like curl, tar,
    # python, etc.
    env = {}
    if "PATH" in repo_ctx.os.environ:
        env["PATH"] = repo_ctx.os.environ["PATH"]
    if "HOME" in repo_ctx.os.environ:
        env["HOME"] = repo_ctx.os.environ["HOME"]

    result = repo_ctx.execute(
        [bazelisk, "build", "--symlink_prefix=/", "//sw/host/opentitantool"],
        working_directory = str(opentitan_path),
        environment = env,
    )

    if result.return_code != 0:
        fail("Failed to build opentitantool:\nSTDOUT:\n%s\nSTDERR:\n%s" %
             (result.stdout, result.stderr))

    # Run bazel info bazel-bin to find the actual output directory
    info_result = repo_ctx.execute(
        [bazelisk, "info", "bazel-bin"],
        working_directory = str(opentitan_path),
        environment = env,
    )

    if info_result.return_code != 0:
        fail("Failed to get bazel-bin info:\nSTDOUT:\n%s\nSTDERR:\n%s" %
             (info_result.stdout, info_result.stderr))

    bazel_bin_path_str = info_result.stdout.strip()
    bazel_bin_path = repo_ctx.path(bazel_bin_path_str)

    built_dir = bazel_bin_path.get_child("sw").get_child("host").get_child("opentitantool")

    # Copy the contents of built_dir to the repository root instead of symlinking.
    # This avoids "Inconsistent filesystem operations" errors when underlying files are cleaned.
    cp_result = repo_ctx.execute(
        ["bash", "-c", "cp -R -L \"$1\"/. .", "--", str(built_dir)],
        environment = env,
    )
    if cp_result.return_code != 0:
        fail("Failed to copy opentitantool files:\nSTDOUT:\n%s\nSTDERR:\n%s" %
             (cp_result.stdout, cp_result.stderr))

    repo_ctx.symlink(repo_ctx.attr.build_file, "BUILD.bazel")

_opentitantool_binary_repo = repository_rule(
    implementation = _opentitantool_binary_repo_impl,
    attrs = {
        "build_file": attr.label(mandatory = True, allow_single_file = True),
    },
    local = True,
)

def _codesigner_binary_repo_impl(repo_ctx):
    # Find cr50-utils path
    cr50_utils_path = repo_ctx.workspace_root.get_child("platform/cr50-utils")
    codesigner_dir = cr50_utils_path.get_child("software").get_child("tools").get_child("codesigner")

    # Run make codesigner
    repo_ctx.report_progress("Building codesigner from source...")

    env = {}
    if "PATH" in repo_ctx.os.environ:
        env["PATH"] = repo_ctx.os.environ["PATH"]
    if "HOME" in repo_ctx.os.environ:
        env["HOME"] = repo_ctx.os.environ["HOME"]

    result = repo_ctx.execute(
        ["make", "codesigner"],
        working_directory = str(codesigner_dir),
        environment = env,
    )

    if result.return_code != 0:
        fail("Failed to build codesigner:\nSTDOUT:\n%s\nSTDERR:\n%s" % (result.stdout, result.stderr))

    # Create bin and lib directories in the repository root
    repo_ctx.execute(["mkdir", "-p", "bin"])
    repo_ctx.execute(["mkdir", "-p", "lib"])

    # Copy the built codesigner to bin/cr50-codesigner
    built_binary = codesigner_dir.get_child("codesigner")
    cp_result = repo_ctx.execute(["cp", str(built_binary), "bin/cr50-codesigner"])
    if cp_result.return_code != 0:
        fail("Failed to copy codesigner binary: %s" % cp_result.stderr)

    repo_ctx.symlink(repo_ctx.attr.build_file, "BUILD.bazel")

_codesigner_binary_repo = repository_rule(
    implementation = _codesigner_binary_repo_impl,
    attrs = {
        "build_file": attr.label(mandatory = True, allow_single_file = True),
    },
    local = True,
)

def _fwsdk_deps_impl(module_ctx):
    generated_repos = []

    # Calls the repo rule, but also adds an annotation to MODULE.bazel so that
    # we can automatically add use_repo on it.
    def repo_rule_wrapper(repo_rule):
        def new_fn(*, name, **kwargs):
            repo_rule(name = name, **kwargs)
            generated_repos.append(name)

        return new_fn

    http_archive = repo_rule_wrapper(_http_archive)
    workaround_http_archive = repo_rule_wrapper(_workaround_http_archive)
    depot_tools = repo_rule_wrapper(_depot_tools)
    git_repository = repo_rule_wrapper(_git_repository)
    new_local_repository = repo_rule_wrapper(_new_local_repository)
    cipd_zip_repo = repo_rule_wrapper(_cipd_zip_repo)
    opentitantool_binary_repo = repo_rule_wrapper(_opentitantool_binary_repo)
    codesigner_binary_repo = repo_rule_wrapper(_codesigner_binary_repo)

    http_archive(
        name = "ec_devutils",
        add_prefix = "bundle",
        build_file = "//platform/rules_cros_firmware/cros_firmware:BUILD.bundle",
        sha256 = "4626ca7ef3d53c182b7517fe7e67d123291ed39c0196b4efb83fb4cb24bf3b37",
        urls = [
            "https://storage.googleapis.com/chromeos-localmirror/fwsdk/bundles/chromeos-base/ec-devutils-0.0.2-r13228.tar.zst",
        ],
    )
    http_archive(
        name = "shflags",
        add_prefix = "bundle",
        build_file = "//platform/rules_cros_firmware/cros_firmware:BUILD.bundle",
        sha256 = "0f829b2ee406630a08f5ee60905a2d5d52ee9154b0133c7185405a479aa58286",
        urls = [
            "https://storage.googleapis.com/chromeos-localmirror/fwsdk/bundles/dev-util/shflags-1.2.3-r1.tar.zst",
        ],
    )

    new_local_repository(
        name = "ec",
        build_file = "//platform/rules_cros_firmware/repositories:BUILD.ec",
        path = "platform/ec",
    )

    new_local_repository(
        name = "zephyr",
        build_file = "//platform/rules_cros_firmware/repositories:BUILD.zephyr",
        path = "third_party/zephyr/main",
    )

    new_local_repository(
        name = "u_boot",
        build_file = "//platform/rules_cros_firmware/repositories:BUILD.u-boot",
        path = "third_party/u-boot",
    )

    new_local_repository(
        name = "hdctools",
        build_file = "//platform/rules_cros_firmware/repositories:BUILD.hdctools",
        path = "third_party/hdctools",
    )

    new_local_repository(
        name = "cmsis",
        build_file = "//platform/rules_cros_firmware/repositories:BUILD.cmsis",
        path = "third_party/zephyr/cmsis",
    )

    # ti50-sdk is built by the subtools builder:
    # https://ci.chromium.org/ui/p/chromeos/builders/infra/build-chromiumos-sdk-subtools-ti50-sdk
    # To uprev, visit the above link and copy the URL to the latest sucessful
    # tarball, and update below.  Additionally, this should be kept in sync
    # with the ebuild:
    # src/private-overlays/chromeos-overlay/chromeos-base/ti50-emulator
    workaround_http_archive(
        name = "ti50-sdk",
        sha256 = "c4129dcd2fc39e801094e6bcfa1896f42ea31623890ce1a6de252faf432575ed",
        build_file = "//platform/ti50/common/toolchain:toolchain.BUILD",
        url = "https://storage.googleapis.com/chromiumos-sdk/toolchains/ti50-sdk/0.0.1-r5/9a8e76c58f731daa92c314ab966fed30e8dbd768.tar.zst",
    )

    workaround_http_archive(
        name = "cros-sdk",
        sha256 = "ee31e319756ced00842829210c8275dae98b3883978194a59a979a512910e696",
        build_file = "//platform/ti50/common/toolchain:cros_sdk.BUILD",
        url = "https://storage.googleapis.com/chromiumos-sdk/2026/01/x86_64-cros-linux-gnu-2026.01.01.81831.tar.xz",
    )
    git_repository(
        name = "compiler-builtins",
        remote = "https://github.com/rust-lang/compiler-builtins",
        tag = "compiler_builtins-v0.1.160",
        build_file = "//platform/ti50/common/toolchain:compiler-builtins.BUILD",
    )
    codesigner_binary_repo(
        name = "codesigner",
        build_file = "//platform/ti50/common/toolchain:codesigner.BUILD",
    )
    opentitantool_binary_repo(
        name = "opentitantool",
        build_file = "//platform/ti50/common/tools:opentitantool.BUILD",
    )

    depot_tools(
        name = "depot_tools",
        build_file = "//platform/rules_cros_firmware/repositories:BUILD.depot_tools",
    )

    return module_ctx.extension_metadata(
        root_module_direct_deps = generated_repos,
        root_module_direct_dev_deps = [],
        reproducible = True,
    )

fwsdk_deps = module_extension(
    implementation = _fwsdk_deps_impl,
)
