#!/bin/bash
# Open-OS QEMU Launcher
# 
# 用法: ./run-openos.sh [image_path]
#
# 需要一个 ChromiumOS/Open-OS 系统镜像。
# 镜像可以在 Linux 环境下通过以下命令构建:
#   cd chromiumos/src/scripts
#   ./build_packages --board=amd64-generic
#   ./build_image --board=amd64-generic --noenable_rootfs_verification dev
#   cros flash usb:// ../build/images/amd64-generic/latest/chromiumos_image.bin

set -e

QEMU_BIN="/opt/homebrew/bin/qemu-system-x86_64"
IMAGE="${1:-openos-image.bin}"
OVMF="/opt/homebrew/share/qemu/edk2-x86_64-code.fd"

# 硬件配置
CPU="host"
CORES=4
RAM="4G"
VGA="virtio"
DISPLAY="cocoa"
USB="on"
NET="user"

# 颜色
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

banner() {
    echo ""
    echo "  ╔══════════════════════════════════════╗"
    echo "  ║         Open-OS  QEMU  Launcher      ║"
    echo "  ╚══════════════════════════════════════╝"
    echo ""
}

banner

# 检查 QEMU
if [ ! -f "$QEMU_BIN" ]; then
    echo -e "${RED}✗ QEMU not found at $QEMU_BIN${NC}"
    echo "  Install: brew install qemu"
    exit 1
fi
echo -e "${GREEN}✓ QEMU${NC} $(qemu-system-x86_64 --version | head -1)"

# 检查镜像
if [ ! -f "$IMAGE" ]; then
    echo ""
    echo -e "${YELLOW}⚠  Image not found: $IMAGE${NC}"
    echo ""
    echo "  要运行 Open-OS，需要一个系统镜像。获取方式:"
    echo ""
    echo "  1. 从 Linux 构建 (推荐):"
    echo "     cd chromiumos/src/scripts"
    echo "     ./build_packages --board=openos-amd64"
    echo "     ./build_image --board=openos-amd64 dev"
    echo "     # 镜像输出在: ../build/images/openos-amd64/latest/"
    echo ""
    echo "  2. 使用 ChromiumOS 预构建镜像作为基础测试:"
    echo "     (下载地址: https://chromium.googlesource.com/chromiumos/docs/+/HEAD/cros_vm.md)"
    echo ""
    echo -e "${YELLOW}是否使用空白镜像创建演示环境? (y/n)${NC}"
    read -r answer
    if [ "$answer" = "y" ]; then
        echo ""
        echo "创建 8GB 演示镜像 + Alpine Linux ..."
        qemu-img create -f qcow2 "$IMAGE" 8G
        echo -e "${GREEN}✓ 镜像已创建: $IMAGE (8GB)${NC}"
        echo ""
        echo "你可以从 Alpine Linux ISO 启动并安装:"
        echo "  curl -LO https://dl-cdn.alpinelinux.org/alpine/latest-stable/releases/x86_64/alpine-standard-3.19.0-x86_64.iso"
    else
        exit 0
    fi
else
    echo -e "${GREEN}✓ Image${NC} $(du -h "$IMAGE" | cut -f1) $IMAGE"
fi

# OVMF UEFI 固件
if [ -f "$OVMF" ]; then
    FIRMWARE="-drive if=pflash,format=raw,readonly=on,file=$OVMF"
    echo -e "${GREEN}✓ UEFI${NC} $OVMF"
else
    FIRMWARE=""
    echo -e "${YELLOW}⚠  No UEFI firmware (legacy BIOS mode)${NC}"
fi

echo ""
echo "  Launching Open-OS with:"
echo "  CPU:    $CPU ($CORES cores)"
echo "  RAM:    $RAM"
echo "  VGA:    $VGA"
echo "  NET:    $NET"
echo ""

# 启动 VM
qemu-system-x86_64 \
    -name "Open-OS" \
    -machine q35,accel=hvf \
    -cpu "$CPU" \
    -smp "$CORES" \
    -m "$RAM" \
    $FIRMWARE \
    -drive file="$IMAGE",format=qcow2,if=virtio \
    -vga "$VGA" \
    -display "$DISPLAY" \
    -usb \
    -device usb-tablet \
    -nic user,model=virtio-net-pci,hostfwd=tcp::9222-:22 \
    -netdev user,id=net0 \
    "$@" &

QEMU_PID=$!
echo -e "${GREEN}✓ QEMU running (PID: $QEMU_PID)${NC}"
echo ""
echo "  SSH:    ssh root@localhost -p 9222"
echo "  Stop:   kill $QEMU_PID"
wait $QEMU_PID
