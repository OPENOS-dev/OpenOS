# ti50-sdk

This package is the toolchain for the ti50 effort (go/ti50). It's composed of a
riscv-enabled C/C++ toolchain, and a riscv-enabled Rust toolchain. It's
currently supported by the ti50 team.

# Upgrading

`dev-embedded/ti50-sdk` is composed of two parts: clang and rust.
It's possible to upgrade each of these independently.

In some cases a dev uses `files/pack_git_tarball.py` to pack a source tarball,
then uploads said tarball to `gs://chromeos-localmirror/distfiles`.

Example per-project invocations of `files/pack_git_tarball.py` are available
below. It's important to keep in mind that **once you upload a new tarball and
point the ti50-sdk ebuild at it, you need to run `FEATURES=-force-mirror ebuild
$(equery w dev-embedded/ti50-sdk) manifest`**. Otherwise, when you try to
download these files from `gs://chromeos-localmirror`, you'll get file integrity
errors.

It's important to note that `chromeos-localmirror` is a large, shared bucket.
Files can be created there but not modified or deleted. You can read more
[here](https://chromium.googlesource.com/chromiumos/docs/+/HEAD/archive_mirrors.md).

Additionally, any patches done to upstream sources should be done *explicitly*
in the ebuild. Tarballs uploaded to chromeos-localmirror are expected to be
clean and true mirrors of the sets of sources available upstream.

## Upgrading clang

In order to upgrade clang, you'll need a tarball of [clang's and LLVM's
sources](https://github.com/llvm/llvm-project) at the SHA you're interested in.
Once you have that at `${dir}`, you can create a git tarball:

```
files/pack_git_tarball.py --git-dir "${dir}" --output-prefix /tmp/llvm
```

This should give you a path that looks like `/tmp/llvm-${sha}-src.tar.xz`. You
can now upload that to gs:

```
gsutil cp -n -a public-read /tmp/llvm-${sha}-src.tar.xz \
    gs://chromeos-localmirror/distfiles/llvm-${sha}-src.tar.xz
```

Update the `LLVM_SHA` variable in the ebuild file to ${sha}.

After running `ebuild manifest` as described in the section above, you should be
able to start testing these changes via `sudo emerge dev-embedded/ti50-sdk`.

## Upgrading rust

First, determine which build of rust you wish to use. Stable versions can be
found at [build tags](https://github.com/rust-lang/rust/tags). This is a preferred
way to go, as the nightly channel can be enabled in `config.toml` created by a build script.

Check [How to Build and Run the Rust Compiler](https://rustc-dev-guide.rust-lang.org/building/how-to-build-and-run.html)
for more details about the process.

`{build_date}` is in the format yyyy-mm-dd and `{channel}` will be one of
`stable|beta|nightly`.  They are related to rustup's `RUST_TOOLCHAIN_VERSION`
variable via `{channel}-{build_date}`.

### Upgrading ChromeOS patches for Rust

Check http://cs/src/third_party/chromiumos-overlay/dev-lang/rust-host/files/cros-rustc/
for potential updates for `rustc`.

### Preparing `rustc` source code package

In the past, this was done by cloning github.com/rust-lang/rust and creating a
git tarball using pack_git_tarball.py.

Now, those extra steps are avoided by using the standard src tarball from rust-lang.org.
See `RUST_SRC_TARBALL_NAME` in the ebuild file.

### Updating .ebuild script

Set `RUST_SRC_TARBALL_NAME` to the new version.

In the new tarball, check `rustc/src/stage0` to set `RUST_STAGE0_DATE` and
`RUST_STAGE0_VERSION` in the ebuild file. For example:

```
compiler_date=2025-08-07
compiler_version=1.89.0
```

After running `ebuild ${ti50-sdk.ebuild} manifest` as described in the section above, you should be
able to start testing these changes via `sudo emerge dev-embedded/ti50-sdk`.

Once this is complete, you can submit a CL with these changes to update
the subtool builder (see `src/third_party/chromiumos-overlay/dev-embedded/ti50-sdk-subtool/ti50-sdk-subtool-9999.ebuild` and https://ci.chromium.org/ui/p/chromeos/builders/infra/build-chromiumos-sdk-subtools-ti50-sdk)

Then update `src/platform/rules_cros_firmware/cros_firmware/deps.bzl` to use the
new ti50-sdk tarball that the subtool builder has uploaded.

## Iterative development

Standard ebuild development practices apply here: `sudo emerge
dev-embedded/ti50-sdk` will clean everything up and start all builds from
scratch. This is desirable in many cases, but not so much when trying to iterate
with a broken toolchain.

The flow the author (gbiv@) used boiled down to `sudo ebuild $(equery w
dev-embedded/ti50-sdk) compile`, which is much more lightweight when e.g.,
trying to figure out why Rust is broken, since it doesn't require a full, fresh
build of LLVM on every iteration.
