# Copyright 2024 OCS (Open Code Studio)
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Platform-specific SDK chroot enter abstraction.

Provides a unified interface for entering the OPENOS SDK environment
across different host platforms:

- Linux: Uses kernel chroot + pivot_root (original behavior)
- macOS: Uses QEMU user-mode chroot emulation
- Other: Extensible for future platforms
"""

import enum
import logging
import os
import platform as _platform
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Dict, List, Optional, Tuple

from chromite.lib import cros_build_lib


class Platform(enum.Enum):
    """Supported host platforms."""
    LINUX = "linux"
    MACOS = "darwin"
    WINDOWS = "win32"  # Reserved, not yet implemented


class Arch(enum.Enum):
    """Supported host architectures."""
    X86_64 = "x86_64"
    AMD64 = "amd64"  # macOS name for x86_64
    ARM64 = "aarch64"
    ARM64E = "arm64"  # macOS name for ARM64
    ARM64E_PAC = "arm64e"  # macOS with pointer auth


# Map macOS arch names to QEMU targets
_QEMU_TARGET_MAP = {
    Arch.X86_64: "qemu-x86_64",
    Arch.AMD64: "qemu-x86_64",
    Arch.ARM64: "qemu-aarch64",
    Arch.ARM64E: "qemu-aarch64",
    Arch.ARM64E_PAC: "qemu-aarch64",
    Arch.ARM64: "qemu-aarch64",
}

# The chroot contains x86_64 Linux binaries, so always use qemu-x86_64
_QEMU_CHROOT_ARCH = "qemu-x86_64"


def detect_platform() -> Platform:
    """Detect the current host platform."""
    sysname = sys.platform
    if sysname == "linux":
        return Platform.LINUX
    elif sysname == "darwin":
        return Platform.MACOS
    elif sysname == "win32":
        return Platform.WINDOWS
    else:
        raise cros_build_lib.die(
            "Unsupported platform: %s. "
            "cros_sdk supports Linux and macOS." % sysname
        )


def detect_arch() -> Arch:
    """Detect the current host CPU architecture."""
    machine = _platform.machine()
    try:
        return Arch(machine)
    except ValueError:
        raise cros_build_lib.die(
            "Unsupported CPU architecture: %s. "
            "cros_sdk supports x86_64, aarch64, arm64." % machine
        )


def is_linux() -> bool:
    """Return True if running on Linux."""
    return detect_platform() == Platform.LINUX


def is_macos() -> bool:
    """Return True if running on macOS."""
    return detect_platform() == Platform.MACOS


def is_apple_silicon() -> bool:
    """Return True if running on Apple Silicon (arm64)."""
    return is_macos() and detect_arch() in (
        Arch.ARM64, Arch.ARM64E, Arch.ARM64E_PAC
    )


def find_qemu_binary(target_arch: str = _QEMU_CHROOT_ARCH) -> Optional[str]:
    """Find the QEMU user-mode binary for the given target architecture.

    Args:
        target_arch: QEMU target name, e.g. 'qemu-x86_64'.

    Returns:
        Path to the QEMU binary, or None if not found.
    """
    # Check common install locations
    candidates = [
        shutil.which(target_arch),
        shutil.which(target_arch + "-static"),
        # Homebrew (Apple Silicon)
        Path("/opt/homebrew/bin") / target_arch,
        Path("/opt/homebrew/bin") / (target_arch + "-static"),
        # Homebrew (Intel Mac)
        Path("/usr/local/bin") / target_arch,
        Path("/usr/local/bin") / (target_arch + "-static"),
        # MacPorts
        Path("/opt/local/bin") / target_arch,
        # Nix
        shutil.which("qemu-" + target_arch.removeprefix("qemu-")),
    ]

    for candidate in candidates:
        if candidate and Path(str(candidate)).exists():
            return str(candidate)

    return None


class PlatformEnter:
    """Abstract base for platform-specific SDK enter implementations.

    Subclasses must implement enter() to provide the platform-specific
    mechanism for running commands inside the SDK chroot.
    """

    def __init__(self, chroot_path: Path):
        """Initialize.

        Args:
            chroot_path: Path to the chroot directory.
        """
        self.chroot_path = chroot_path.resolve()

    @staticmethod
    def create(chroot_path: Path) -> "PlatformEnter":
        """Factory: create the appropriate PlatformEnter for the current host.

        Args:
            chroot_path: Path to the chroot directory.

        Returns:
            PlatformEnter subclass instance.
        """
        platform = detect_platform()
        arch = detect_arch()
        logging.info("Detected platform: %s / %s", platform.value, arch.value)

        if platform == Platform.LINUX:
            return LinuxEnter(chroot_path)
        elif platform == Platform.MACOS:
            return MacOSEnter(chroot_path)
        else:
            raise cros_build_lib.die(
                "No PlatformEnter implementation for %s" % platform.value
            )

    def needs_root(self) -> bool:
        """Return True if this platform requires root to enter the chroot."""
        return True

    def needs_mount_setup(self) -> bool:
        """Return True if this platform requires mount setup before enter."""
        return True

    def enter(
        self,
        cmd: Optional[List[str]] = None,
        cwd: Optional[Path] = None,
        env: Optional[Dict[str, str]] = None,
        read_only: bool = False,
    ) -> cros_build_lib.CompletedProcess:
        """Enter the SDK chroot and run a command.

        Args:
            cmd: Command to run inside the chroot (None = interactive shell).
            cwd: Working directory inside the chroot.
            env: Environment variables to set inside the chroot.
            read_only: Whether to mount the chroot read-only.

        Returns:
            CompletedProcess with returncode and output.
        """
        raise NotImplementedError


class LinuxEnter(PlatformEnter):
    """Linux: Uses kernel chroot + pivot_root (original OPENOS behavior).

    The existing ChrootEnteror._enter_chroot() logic is preserved here.
    This is the "gold standard" path used for production builds.
    """

    def needs_root(self) -> bool:
        return True

    def needs_mount_setup(self) -> bool:
        return True

    def enter(
        self,
        cmd: Optional[List[str]] = None,
        cwd: Optional[Path] = None,
        env: Optional[Dict[str, str]] = None,
        read_only: bool = False,
    ) -> cros_build_lib.CompletedProcess:
        """Enter via chroot(2) + pivot_root(2).

        This is called by ChrootEnteror which handles the actual chroot/pivot
        logic. This class just signals that the native path should be used.
        """
        # Return sentinel to indicate native chroot should be used
        return cros_build_lib.CompletedProcess(
            returncode=-1,
            stdout="",
            stderr="",
        )


class MacOSEnter(PlatformEnter):
    """macOS: Uses QEMU user-mode chroot emulation.

    QEMU user-mode can simulate Linux syscalls on macOS, including chroot.
    On Apple Silicon, Rosetta 2 automatically translates the QEMU x86_64
    binary for reasonable performance.

    Requirements:
        brew install qemu   (provides qemu-x86_64)
    """

    # Default SDK paths inside the chroot
    SDK_PATH = Path("/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:"
                     "/sbin:/bin:/opt/bin")

    def needs_root(self) -> bool:
        return False  # QEMU user-mode doesn't need root

    def needs_mount_setup(self) -> bool:
        return False  # QEMU handles virtual filesystem

    def _ensure_qemu(self) -> str:
        """Find the QEMU binary or report how to install it.

        Returns:
            Path to qemu-x86_64 binary.

        Raises:
            SystemExit: If QEMU is not installed.
        """
        qemu = find_qemu_binary(_QEMU_CHROOT_ARCH)
        if qemu:
            logging.info("Found QEMU: %s", qemu)
            return qemu

        # QEMU not found - provide install instructions
        if is_apple_silicon():
            install_cmd = "brew install qemu"
        else:
            install_cmd = "brew install qemu"

        raise cros_build_lib.die(
            "QEMU user-mode emulation is required for macOS builds.\n"
            "Install it with:\n"
            "  %s\n"
            "Then re-run cros_sdk." % install_cmd
        )

    def enter(
        self,
        cmd: Optional[List[str]] = None,
        cwd: Optional[Path] = None,
        env: Optional[Dict[str, str]] = None,
        read_only: bool = False,
    ) -> cros_build_lib.CompletedProcess:
        """Enter the SDK chroot via QEMU user-mode emulation.

        Args:
            cmd: Command to run inside the chroot.
            cwd: Working directory inside the chroot.
            env: Environment variables.
            read_only: Ignored on macOS (QEMU provides read-write).

        Returns:
            CompletedProcess with returncode and output.
        """
        qemu_bin = self._ensure_qemu()

        if not self.chroot_path.exists():
            raise cros_build_lib.die(
                "Chroot path does not exist: %s" % self.chroot_path
            )

        # Build the QEMU command
        # qemu-x86_64 -chroot <dir> sets the root to <dir> before running
        qemu_cmd = [qemu_bin, "-chroot", str(self.chroot_path)]

        # Set up environment for the emulated process
        run_env = os.environ.copy()
        run_env["PATH"] = str(self.SDK_PATH)
        run_env["LANG"] = "C.UTF-8"
        run_env["HOME"] = "/home/" + os.environ.get("USER", "builder")

        # Apply any additional env vars
        if env:
            run_env.update(env)

        # Determine the command to run
        if cmd:
            # Run the specified command
            qemu_cmd += cmd
        else:
            # Interactive shell
            shell = self._find_shell_in_chroot()
            qemu_cmd += [shell]

            if cwd:
                qemu_cmd = [qemu_bin, "-chroot", str(self.chroot_path),
                           shell, "-c", "cd %s && exec %s" % (cwd, shell)]

        logging.info("Entering chroot via QEMU: %s", " ".join(qemu_cmd[:4]))

        result = cros_build_lib.dbg_run(
            qemu_cmd,
            env=run_env,
            check=False,
            cwd=self.chroot_path,
        )

        return result

    def _find_shell_in_chroot(self) -> str:
        """Find an available shell in the chroot.

        Returns:
            Path to a usable shell (inside chroot namespace).
        """
        for shell in ["/bin/bash", "/bin/sh", "/bin/zsh"]:
            shell_path = self.chroot_path / shell.lstrip("/")
            if shell_path.exists():
                return shell
        return "/bin/sh"


class PlatformGuard:
    """Context manager / decorator for guarding Linux-specific code."""

    @staticmethod
    def skip_on_macos(message: str = ""):
        """Decorator: skip the decorated function on macOS.
        
        Args:
            message: Log message to emit when skipping.
        """
        def decorator(func):
            def wrapper(*args, **kwargs):
                if is_macos():
                    if message:
                        logging.debug("[macOS skip] %s: %s",
                                    func.__name__, message)
                    return None
                return func(*args, **kwargs)
            return wrapper
        return decorator

    @staticmethod
    def warn_on_macos(message: str = ""):
        """Decorator: warn but still execute on macOS (best-effort).

        Args:
            message: Warning message to emit.
        """
        def decorator(func):
            def wrapper(*args, **kwargs):
                if is_macos():
                    logging.warning("[macOS] %s: %s", func.__name__, message)
                return func(*args, **kwargs)
            return wrapper
        return decorator
