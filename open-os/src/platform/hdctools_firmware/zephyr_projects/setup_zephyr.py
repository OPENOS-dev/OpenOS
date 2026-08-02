# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import pathlib
import subprocess

DIR = pathlib.Path(__file__).resolve().parent
SDK_DIR = DIR / "sdk"

VENV_PATH = DIR / ".venv"
WEST_PATH = DIR / ".west"
SDK_PATH = SDK_DIR / "zephyr-sdk-0.16.1"

# SDK file
URL = "https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.16.1"
TAR = "zephyr-sdk-0.16.1_linux-x86_64.tar.xz"

VENV_PIP_BIN = VENV_PATH / "bin/pip"
VENV_WEST_BIN = VENV_PATH / "bin/west"


def run_cmd(cmd, cwd=None):
    if cwd is None:
        cwd = DIR
    cmd = [str(x) for x in cmd]
    cmd_str = " ".join(cmd)
    print(f"Running '{cmd_str}' in '{cwd}'")
    subprocess.check_output(cmd, cwd=cwd)


def venv_setup():
    if VENV_PATH.is_dir():
        return True
    print("Setting up venv")
    venv_setup_cmd = ["python3", "-m", "venv", VENV_PATH]
    run_cmd(venv_setup_cmd)
    # Install Pip requirements
    requirements_path = DIR / "requirements.txt"
    pip_install_cmd = [VENV_PIP_BIN, "install", "-r", requirements_path]
    run_cmd(pip_install_cmd)


def west_setup():
    if WEST_PATH.is_dir():
        return True

    print("Setting up west")
    west_init_cmd = [VENV_WEST_BIN, "init", "-l", "west_config"]
    run_cmd(west_init_cmd)
    west_update_cmd = [VENV_WEST_BIN, "update"]
    run_cmd(west_update_cmd)

    zephyr_export_cmd = [VENV_WEST_BIN, "zephyr-export"]
    run_cmd(zephyr_export_cmd)

    # Install Zephyr's pip artifacts
    zephyr_pip_req_cmd = [
        VENV_PIP_BIN,
        "install",
        "-r",
        SDK_DIR / "zephyr/scripts/requirements.txt",
    ]
    run_cmd(zephyr_pip_req_cmd)


def sdk_setup():
    if SDK_PATH.is_dir():
        return True
    print("Setting up sdk")
    download_tar_cmd = ["wget", "{}/{}".format(URL, TAR)]
    run_cmd(download_tar_cmd, cwd=SDK_DIR)
    tar_file = SDK_DIR / TAR
    extract_cmd = ["tar", "xvf", tar_file]
    run_cmd(extract_cmd, cwd=SDK_DIR)
    # Cleanup the tar
    tar_file.unlink()
    # Run the SDK setup command
    setup_path = SDK_PATH / "setup.sh"
    FLAGS = ["-t", "all", "-h", "-c"]
    setup_sdk_cmd = ["sh", setup_path] + FLAGS
    run_cmd(setup_sdk_cmd)


venv_setup()
west_setup()
sdk_setup()
print("Setup complete")
