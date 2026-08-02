# OPENOS

一个基于 ChromiumOS 的操作系统，已移除所有 Google 专有服务，内置 Android 应用支持，并使用 OpenScript 编写的 OPT 作为包管理器。

```
  ___  ____  _____ _   _    ___  ____
 / _ \|  _ \| ____| \ | |  / _ \/ ___|
| | | | |_) |  _| |  \| | | | | \___ \
| |_| |  __/| |___| |\  | | |_| |___) |
 \___/|_|   |_____|_| \_|  \___/|____/
```

## 项目结构

```
OPENOS/
├── open-os/                  # 核心 OS 构建系统
│   ├── src/                  # ChromiumOS 源码树
│   │   ├── third_party/
│   │   │   ├── chromiumos-overlay/  # 上游 Portage overlay (ebuild 仓库)
│   │   │   ├── openos-overlay/      # ★ OPENOS 自定义 overlay (Portage+Bazel)
│   │   │   │   ├── MODULE.bazel + BUILD.bazel    # Bazel 构建文件
│   │   │   │   ├── .bazelrc + .bazelversion      # Bazel 配置
│   │   │   │   ├── chromeos-base/openos-meta/    # 元包 (ebuild+BUILD)
│   │   │   │   ├── chromeos-base/openos-theme/   # NOTHING UI 主题 (ebuild+BUILD)
│   │   │   │   ├── profiles/base/                # USE flags + platform constraints
│   │   │   │   └── x11-themes/                   # 光标/图标/壁纸
│   │   │   └── portage-stable/         # Portage 稳定 ebuild
│   │   ├── overlays/            # 板级 overlay (overlay-amd64-generic 等)
│   │   ├── platform2/           # ChromiumOS 系统服务
│   │   └── aosp/                # AOSP 组件 (ARCVM Android 运行时)
│   ├── chromite/             # ChromiumOS 构建工具 (Python)
│   ├── manifest/             # repo 清单文件
│   │   ├── default.xml       # 入口 (引用 full.xml + openos.xml)
│   │   ├── full.xml          # ChromiumOS 上游完整项目清单 (R151)
│   │   ├── openos.xml        # ★ OPENOS 扩展项目清单
│   │   └── _remotes.xml      # Git 远程源定义
│   ├── infra/                # CI/CD 基础设施 (Python/Go/Proto)
│   ├── bazel_deps/           # Bazel 依赖配置
│   └── qemu/                 # QEMU 演示镜像脚本
│
├── opt/                      # Open Package Tool 包管理器
│   ├── libopt/               # 核心库 (Rust)
│   ├── opt-cli/              # OPT CLI 命令行
│   ├── opk-cli/              # OPK CLI 命令行
│   ├── opk-server/           # 包仓库服务器
│   ├── opk-ui/               # 包管理 Web UI
│   ├── opt-gui/              # 桌面 GUI (Qt C++)
│   ├── docs/                 # 文档
│   └── repo/                 # 仓库池
│
├── depot_tools/              # ChromiumOS depot_tools 工具集
├── chromiumos/               # repo 工作目录 (.repo)
├── LICENSE                   # GPL-3.0
├── LICENSE.txt               # GPL-3.0 文本
└── README.md
```

## 许可证

本项目,使用GPL-3.0许可证,请按照[LICENSE](./LICENSE.txt)中的说明使用该项目。
