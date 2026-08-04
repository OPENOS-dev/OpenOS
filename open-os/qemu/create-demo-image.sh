#!/bin/bash
# Open-OS Demo Image Creator
# 创建最小演示镜像，展示 Open-OS 启动过程

set -e
IMAGE="openos-demo.img"
SIZE="2G"

echo "=== Open-OS Demo Image Creator ==="
echo ""

# Create raw image
qemu-img create -f raw "$IMAGE" "$SIZE"
echo "✓ Created $IMAGE ($SIZE)"

# Mount and setup
DEV=$(hdiutil attach -imagekey diskimage-class=CRawDiskImage -nomount "$IMAGE" 2>&1 | head -1 | awk '{print $1}')
if [ -z "$DEV" ]; then
    echo "✗ Cannot attach image"
    exit 1
fi
echo "✓ Attached: $DEV"

# Partition using macOS tools
diskutil partitionDisk "$DEV" 1 GPT "Free Space" "100%" 2>/dev/null || true

# Format as ext2 (macOS can't do ext2 natively, use newfs_msdos as demo)
# For real ChromiumOS, build on Linux
echo ""
echo "⚠  macOS cannot create ChromiumOS rootfs (needs ext4 + verified boot)."
echo "   此镜像为占位符。实际系统镜像需在 Linux 环境下编译。"
echo ""
echo "   Linux 构建命令:"
echo "   cd chromiumos/src/scripts"
echo "   ./build_packages --board=openos-amd64"
echo "   ./build_image --board=openos-amd64 dev"
echo ""

hdiutil detach "$DEV" 2>/dev/null || true
echo "✓ Done"
