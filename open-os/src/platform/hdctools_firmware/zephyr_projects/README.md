# HDC Tools Zephyr Projects

This contains source code and a build system to develop HDC Tools Firmware
with the Zephyr RTOS.

## Quick start environment setup

### Run Setup Script

A bare git installation can be setup with the install script `setup_zephyr.py`.
This script will create a virtual environment, install west and other build
dependencies.

Run `python3 setup_zephyr.py`.

### Activate venv

Activate the virtual environment using the shortcut.

`source activate`

### Build example project to validate everything is working

Build an example project or Twinkie_v2

`west build -p always -b nucleo_g474re sdk/zephyr/samples/basic/blinky`

`west build -p always -b google_twinkie_v2 sdk/zephyr/samples/boards/google_twinkie_v2_pda`

### Build Starfish

`west build -p always -b starfish_v1 starfish`

### Flash Starfish

`sudo dfu-util -a 0 -s 0x8000000:leave -D build/zephyr/zephyr.bin`

### Build Twinkie

`west build -p always twinkie_v2`
