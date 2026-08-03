# OPENOS 项目记忆

## 项目概览
- **项目**: OPENOS - 基于 ChromiumOS 的去 Google 化操作系统
- **组织**: OCS (Open Code Studio)
- **许可证**: GPL-3.0
- **目标**: 移除 Google 专有服务，内置 Android 支持 (AnOpenDroid)，使用 OPT 作为包管理器

## 技术架构决策
- **OS 层**: 基于 ChromiumOS + Gentoo/Portage 构建系统，通过 repo + depot_tools 管理源码
- **去 Google 化**: 通过 Portage package.mask + USE flags + 文件删除三层策略
- **Android 支持**: 使用 ARCVM 虚拟机方案，AOSP 系统镜像 + F-Droid 替代 Google Play
- **包管理器**: OPT (Open Package Tool)，计划用 OpenScript 重写核心逻辑，GUI 使用 Qt C++，服务端 Go

## 直接修改 ChromiumOS 源码树（零脚本原则）
所有定制直接写入 `chromiumos/src/` 源码树，不维护外部脚本：

**chromiumos-overlay（degoogle 永久化）**：
- `profiles/targets/chromeos/package.mask` — 18个 Google 包屏蔽
- `profiles/targets/chromeos/make.defaults` — USE flags + 品牌配置

**openos-overlay（新建，`src/third_party/openos-overlay/`）**：
- `chromeos-base/openos-meta` — 元包（依赖 theme + Android + OPT）
- `chromeos-base/openos-theme` — NOTHING UI CSS（login/shell/tokens/webui）

**板级注册**：`overlay-amd64-generic` 的 layout.conf + parent 引用 openos

## 项目规则
- 所有脚本必须使用 `set -euo pipefail`
- 系统名展示为 `OPENOS`（代码标识符保留小写 `openos`）
- 构建目标板默认 `amd64-generic`
- 源码目录默认 `${OPENOS_ROOT}/chromiumos`
- 移除的 Google 服务共 14 类（见 degoogle.sh）

## 构建约束
- 构建时间 ≤ 6 小时
- 存储空间 ≤ 60GB（当前 58GB）
- 保留内核版本：v6.6（主力）, v6.10-enablement, v6.12

## 硬件兼容策略
- 保留 coreboot + fsp + amd-fsp：为专属硬件 BIOS 自控准备
- 保留 u-boot/zephyr/pigweed：ARM 等多平台支持
- 保留各版本 Mesa 驱动：多 GPU 架构
- 删除 Chromebook 专属固件（指纹/工厂/校准），将来自研通用硬件兼容层

## 版权 & 品牌（2026-07-18）
- 版权头: `OCS (Open Code Studio)` — 801+ 个文件
- 系统展示名: `OPENOS`（代码标识符/变量名/路径保留 `openos` 小写）
- 品牌名已全部从 ChromiumOS/Google/Chrome OS 替换
- 镜像名（chromiumos_base_image 等）: 已保护，未修改
- 关键文件: LICENSE(OCS)、AUTHORS(OCS)、constants.py(OPENOS)、openos_version.py(OPENOS_BUILD)
- hostname_util.py: Google 域名检测已清空
- 未修改: third_party/（第三方代码）、api/gen/（protobuf Go 标识符）

## OPT 包管理器（原 OPK，2026-07-18 改名）
- 目录: `opt/`（原 `opk/`）
- 计划用 OpenScript 重写核心（libopt、opt-cli、opk-cli）
- OpenScript 仓库: `/Users/cangcang/code/Open-Script-OS`
- 配置路径: `/etc/opt/opt-repos.conf`（原 `/etc/opk/opk-repos.conf`）
- 当前保留 Rust 原型代码用于参考
