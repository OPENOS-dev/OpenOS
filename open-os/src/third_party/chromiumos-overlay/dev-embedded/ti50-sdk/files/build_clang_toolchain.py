#!/usr/bin/env python3
# Copyright 2021 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Builds the clang toolchain for Ti50."""

import argparse
import logging
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
from typing import List


# CFlags used when building RISCV runtime libraries.
RISCV_RUNTIME_CFLAGS = (
    "-O2",
    "--target=riscv32-unknown-elf",
    "-DVISIBILITY_HIDDEN",
    "-DNDEBUG",
    "-fno-builtin",
    "-fvisibility=hidden",
    "-fomit-frame-pointer",
    "-mrelax",
    "-fforce-enable-int128",
    "-DCRT_HAS_INITFINI_ARRAY",
    "-march=rv32imc",
    "-ffunction-sections",
    "-fdata-sections",
    "-fstack-size-section",
    "-mcmodel=medlow",
    "-Wno-unused-command-line-argument",
)

# CFlags used when building ARM runtime libraries.
ARM_RUNTIME_CFLAGS = (
    "-O2",
    "--target=arm-none-eabi",
    "-march=armv7-m",
    "-mthumb",
    "-DVISIBILITY_HIDDEN",
    "-DNDEBUG",
    "-fno-builtin",
    "-fvisibility=hidden",
    "-fomit-frame-pointer",
    "-ffunction-sections",
    "-fdata-sections",
    "-Wno-unused-command-line-argument",
)

# Flags passed to cmake command for building LLVM
CMAKE_FLAGS = (
    "-G",
    "Ninja",
    "-DCMAKE_BUILD_TYPE=Release",
    "-DCLANG_VENDOR=ChromiumOS Ti50",
    "-DCMAKE_C_COMPILER=/usr/bin/clang",
    "-DCMAKE_CXX_COMPILER=/usr/bin/clang++",
    "-DCMAKE_C_COMPILER_LAUNCHER=ccache",
    "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache",
    "-DLLVM_ENABLE_LIBCXX=ON",
    "-DLLVM_ENABLE_LLD=ON",
    "-DLLVM_OPTIMIZED_TABLEGEN=ON",
    "-DLLVM_BUILD_TESTS=OFF",
    "-DCLANG_ENABLE_STATIC_ANALYZER=ON",
    '-DCLANG_DEFAULT_RTLIB="compiler-rt"',
    "-DLLVM_DEFAULT_TARGET_TRIPLE=riscv32-unknown-elf",
    "-DLLVM_INSTALL_BINUTILS_SYMLINKS=ON",
    "-DLLVM_INSTALL_CCTOOLS_SYMLINKS=ON",
    "-DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=OFF",
    "-DLLVM_BUILD_RUNTIME=OFF",
    "-DLLVM_BUILD_RUNTIMES=OFF",
    "-DCLANG_DEFAULT_LINKER=ld.lld",
    "-DLLVM_ENABLE_BACKTRACES=OFF",
    "-DLLVM_INCLUDE_EXAMPLES=OFF",
    '-DLLVM_DYLIB_COMPONENTS=""',
    "-DCLANG_ENABLE_CLANGD=ON",
    "-DLLVM_LINK_LLVM_DYLIB=OFF",
    "-DLLVM_BUILD_STATIC=OFF",
    "-DLLVM_INSTALL_UTILS=ON",
    "-DLLVM_ENABLE_Z3_SOLVER=OFF",
    "-DLLVM_ENABLE_LIBPFM=OFF",
    "-DLLVM_ENABLE_LIBXML2=OFF",
    "-DLLVM_ENABLE_LIBEDIT=OFF",
    "-DLLVM_ENABLE_OCAMLDOC=OFF",
    "-DLLVM_INCLUDE_BENCHMARKS=OFF",
    "-DLLVM_INCLUDE_DOCS=OFF",
    "-DLLVM_INCLUDE_TESTS=OFF",
    "-DLLVM_USE_RELATIVE_PATHS_IN_FILES=ON",
    "-DCLANG_DEFAULT_STD_C=gnu17",
    "-DENABLE_X86_RELAX_RELOCATIONS=ON",
    "-DLLVM_TARGETS_TO_BUILD=RISCV;X86;ARM",
    "-DLLVM_ENABLE_PROJECTS=clang;clang-tools-extra;lld",
)


def remove_path_if_exists(file_or_dir: Path):
    """Removes the given file/directory/symlink if it exists."""
    if file_or_dir.is_file() or file_or_dir.is_symlink():
        file_or_dir.unlink()
    elif file_or_dir.is_dir():
        shutil.rmtree(file_or_dir)
    else:
        assert not file_or_dir.exists(), file_or_dir


def find_clang_runtime_dir(llvm_install_dir: Path, target: str = None) -> Path:
    """Locates the runtime directory of the clang in |llvm_install_dir|."""
    cmd = [str(llvm_install_dir / "bin" / "clang")]
    if target:
        cmd += [f"--target={target}"]
    cmd += ["-print-runtime-dir"]
    return Path(subprocess.check_output(cmd, encoding="utf-8").strip())


def build_llvm(llvm_dir: Path, build_dir: Path, install_dir: Path):
    """Builds and installs LLVM to install_dir, crt{begin,end}."""
    build_dir.mkdir(parents=True, exist_ok=True)

    if not (build_dir / "build.ninja").exists():
        logging.info("Configuring LLVM")
        # Use sysroot relative to installation, since {install_dir}
        # points to temporary directory used by ebuild.
        cmd = [
            "cmake",
            f"-DCMAKE_INSTALL_PREFIX={install_dir}",
            "-DDEFAULT_SYSROOT=..",
            str(llvm_dir / "llvm"),
        ]
        cmd += CMAKE_FLAGS
        subprocess.check_call(cmd, cwd=build_dir)

    logging.info("Building + installing LLVM")
    subprocess.check_call(["ninja", "install"], cwd=build_dir)

    # Clean /lib directory which is not needed
    for f in (install_dir / "lib").glob("*.a"):
        f.unlink()

    remove_path_if_exists(install_dir / "lib" / "cmake")

    # Build RISC-V crtbegin/end
    riscv_clang_rtlib_prefix = [
        str(install_dir / "bin" / "clang"),
        "-c",
        "-v",
    ] + list(RISCV_RUNTIME_CFLAGS)

    riscv_libdir = find_clang_runtime_dir(install_dir)
    riscv_libdir.mkdir(parents=True, exist_ok=True)

    subprocess.check_call(
        riscv_clang_rtlib_prefix
        + [
            str(llvm_dir / "compiler-rt" / "lib" / "builtins" / "crtbegin.c"),
            "-o",
            str(riscv_libdir / "clang_rt.crtbegin-riscv32.o"),
        ]
    )

    subprocess.check_call(
        riscv_clang_rtlib_prefix
        + [
            str(llvm_dir / "compiler-rt" / "lib" / "builtins" / "crtend.c"),
            "-o",
            str(riscv_libdir / "clang_rt.crtend-riscv32.o"),
        ]
    )

    # Build ARM crtbegin/end
    arm_clang_rtlib_prefix = [
        str(install_dir / "bin" / "clang"),
        "-c",
        "-v",
    ] + list(ARM_RUNTIME_CFLAGS)

    arm_libdir = find_clang_runtime_dir(install_dir, "arm-none-eabi")
    arm_libdir.mkdir(parents=True, exist_ok=True)

    subprocess.check_call(
        arm_clang_rtlib_prefix
        + [
            str(llvm_dir / "compiler-rt" / "lib" / "builtins" / "crtbegin.c"),
            "-o",
            str(arm_libdir / "clang_rt.crtbegin-arm.o"),
        ]
    )

    subprocess.check_call(
        arm_clang_rtlib_prefix
        + [
            str(llvm_dir / "compiler-rt" / "lib" / "builtins" / "crtend.c"),
            "-o",
            str(arm_libdir / "clang_rt.crtend-arm.o"),
        ]
    )


def install_c_headers(install_dir: Path, include_dir: Path):
    """Builds and installs C headers into |install_dir|/include."""
    installed_include_dir = install_dir / "include"

    # Remove all LLVM development headers installed in {install_dir}/include
    remove_path_if_exists(installed_include_dir)

    # Copy baremetal C headers
    shutil.copytree(include_dir, installed_include_dir)


def build_compiler_rt(
    llvm_path: Path,
    compiler_rt_build_dir: Path,
    install_dir: Path,
    target: str,
    cflags: tuple,
    arch: str,
    extra_sources: list = None,
    expected_failures: set = None,
):
    """Builds compiler_rt and installs artifacts in |install_dir|."""
    if extra_sources is None:
        extra_sources = []
    if expected_failures is None:
        expected_failures = set()
    compiler_rt_build_dir.mkdir(parents=True)
    run_clang = [str(install_dir / "bin" / "clang"), "-c"]
    run_clang += cflags
    compiler_rt_builtins = llvm_path / "compiler-rt" / "lib" / "builtins"

    for src in extra_sources:
        subprocess.check_call(run_clang + [str(src)], cwd=compiler_rt_build_dir)

    had_unexpected_results = False
    for f in compiler_rt_builtins.glob("*.c"):
        # Build the expected failures, since doing so is quick, and if they _do_
        # successfully build, that sounds like smoke to me.
        command = run_clang + [str(f)]
        should_fail = f.name in expected_failures
        command_string = " ".join(shlex.quote(x) for x in command)
        try:
            stdout = subprocess.check_output(
                command,
                cwd=compiler_rt_build_dir,
                stderr=subprocess.STDOUT,
                encoding="utf-8",
            )
        except subprocess.CalledProcessError as e:
            if should_fail:
                logging.info("Running %s failed as expected", command_string)
                continue
            logging.error(
                "Running %s failed unexpectedly; output:\n%s",
                command_string,
                e.stdout,
            )
            had_unexpected_results = True
        else:
            if not should_fail:
                logging.info("Running %s succeeded as expected", command_string)
                continue
            logging.error(
                "Running %s succeeded unexpectedly; output:\n%s",
                command_string,
                stdout,
            )
            had_unexpected_results = True

    if had_unexpected_results:
        raise ValueError(
            "Manual builds of compiler-rt bits didn't go as planned; please"
            " see logging output"
        )

    libdir = find_clang_runtime_dir(install_dir, target)
    link_command = [
        str(install_dir / "bin" / "llvm-ar"),
        "rc",
        str(libdir / f"libclang_rt.builtins-{arch}.a"),
    ]
    link_command += (str(x) for x in compiler_rt_build_dir.glob("*.o"))
    subprocess.check_call(link_command)


def log_install_paths(install_dir: Path):
    """Prints install paths to stdout."""
    clang = str(install_dir / "bin" / "clang")
    subprocess.check_call([clang, "-E", "-x", "c++", "/dev/null", "-v"])
    subprocess.check_call([clang, "-print-search-dirs"])
    subprocess.check_call([clang, "-print-runtime-dir"])


def get_parser():
    """Creates a parser for commandline args."""
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "--work-dir",
        required=True,
        type=Path,
        help="Path to put build artifacts in.",
    )
    parser.add_argument(
        "--llvm-dir",
        required=True,
        type=Path,
        help="Path to the LLVM checkout.",
    )
    parser.add_argument(
        "--include-dir",
        required=True,
        type=Path,
        help="Path to the C headers.",
    )
    parser.add_argument(
        "--no-clean-llvm",
        action="store_false",
        dest="clean_llvm",
        help="Don't wipe out LLVM's build directory if it exists.",
    )
    parser.add_argument(
        "--install-dir",
        required=True,
        type=Path,
        help="Path to the place to install artifacts.",
    )
    return parser


def main(argv: List[str]):
    """Build clang for RISC-V"""
    logging.basicConfig(
        format="%(asctime)s: %(levelname)s: %(filename)s:%(lineno)d: "
        "%(message)s",
        level=logging.INFO,
    )

    opts = get_parser().parse_args(argv)
    work_dir = opts.work_dir
    install_dir = opts.install_dir.resolve()
    llvm_path = opts.llvm_dir.resolve()
    include_dir = opts.include_dir.resolve()

    work_dir.mkdir(parents=True, exist_ok=True)

    remove_path_if_exists(install_dir)
    install_dir.mkdir(parents=True)

    llvm_build_dir = work_dir / "llvm-build" / "Release"
    if opts.clean_llvm:
        remove_path_if_exists(llvm_build_dir)
    build_llvm(llvm_path, llvm_build_dir, install_dir)

    install_c_headers(install_dir=install_dir, include_dir=include_dir)

    # It's expected that some of these builds fail for various reasons

    # Common expected failures (mostly long double, TLS, etc.)
    common_expected_failures = {
        "divxc3.c",
        "emutls.c",
        "enable_execute_stack.c",
        "eprintf.c",
        "extendhfxf2.c",
        "fixunsxfdi.c",
        "fixunsxfsi.c",
        "fixxfdi.c",
        "floatdixf.c",
        "floatundixf.c",
        "mulxc3.c",
        "powixf2.c",
        "truncxfhf2.c",
    }

    # RISC-V specific expected failures
    riscv_expected_failures = common_expected_failures | {
        "atomic.c",
        "fixunsxfti.c",
        "fixxfti.c",
        "floattixf.c",
        "floatuntixf.c",
    }

    # ARM specific expected failures
    arm_expected_failures = common_expected_failures | {
        "truncdfbf2.c",
        "truncsfbf2.c",
    }

    # Build compiler-rt for RISC-V
    compiler_rt_riscv_build_dir = work_dir / "compiler-rt-riscv-build"
    remove_path_if_exists(compiler_rt_riscv_build_dir)
    build_compiler_rt(
        llvm_path=llvm_path,
        compiler_rt_build_dir=compiler_rt_riscv_build_dir,
        install_dir=install_dir,
        target="riscv32-unknown-elf",
        cflags=RISCV_RUNTIME_CFLAGS,
        arch="riscv32",
        extra_sources=[
            llvm_path
            / "compiler-rt"
            / "lib"
            / "builtins"
            / "riscv"
            / "mulsi3.S"
        ],
        expected_failures=riscv_expected_failures,
    )

    # Build compiler-rt for ARM
    compiler_rt_arm_build_dir = work_dir / "compiler-rt-arm-build"
    remove_path_if_exists(compiler_rt_arm_build_dir)
    build_compiler_rt(
        llvm_path=llvm_path,
        compiler_rt_build_dir=compiler_rt_arm_build_dir,
        install_dir=install_dir,
        target="arm-none-eabi",
        cflags=ARM_RUNTIME_CFLAGS,
        arch="arm",
        extra_sources=[
            llvm_path
            / "compiler-rt"
            / "lib"
            / "builtins"
            / "arm"
            / "aeabi_memcpy.S",
            llvm_path
            / "compiler-rt"
            / "lib"
            / "builtins"
            / "arm"
            / "aeabi_memmove.S",
            llvm_path
            / "compiler-rt"
            / "lib"
            / "builtins"
            / "arm"
            / "aeabi_memset.S",
        ],
        expected_failures=arm_expected_failures,
    )

    log_install_paths(install_dir)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
