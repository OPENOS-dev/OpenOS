# OPT - Open Package Tool

OPENOS 的官方包管理器，使用 OpenScript 编写。支持 .opt 包格式。

```
  ___  ____  _____
 / _ \|  _ \|_   _|
| | | | |_) | | |
| |_| |  __/  | |
 \___/|_|     |_|
```

## 架构

```
                    ┌──────────┐
                    │ opt-gui  │  Qt C++ 桌面 GUI
                    └────┬─────┘
                         │ C FFI
                    ┌────▼─────┐
                    │  libopt  │  OpenScript 核心库
                    └─┬─────┬──┘
               ┌──────┘     └────────┐
          ┌────▼────┐          ┌─────▼─────┐
          │ opt-cli │          │ opk-server │
          │ OpenScript│        │ Go         │
          └─────────┘          └───────────┘
                                        │
          ┌─────────┐          ┌────────▼────────┐
          │ opk-cli │          │ OPT 仓库        │
          │ OpenScript│        │ (HTTP Packages) │
          └─────────┘          └─────────────────┘
```

## 组件

| 组件 | 语言 | 说明 |
|------|------|------|
| `libopt/` | OpenScript | 核心库 — 包格式、构建、安装逻辑 + C FFI |
| `opt-cli/` | OpenScript | 桌面版 CLI (.opt)，薄壳调用 libopt |
| `opt-gui/` | C++ Qt | 桌面版 GUI，通过 C FFI 调用 libopt |
| `opk-cli/` | OpenScript | 命令行工具，兼容 apt 语法 |
| `opk-ui/` | Python | 终端 UI (Textual)，类 aptitude 界面 |
| `opk-server/` | Go | 包仓库 HTTP 服务器 |
| `docs/` | - | 包格式规范、API 文档 |

## OpenScript 迁移

当前代码为 Rust 原型实现，计划用 OpenScript 重写核心逻辑。
OpenScript 源码仓库: `/Users/cangcang/code/Open-Script-OS`

迁移计划:
- [ ] `libopt/` — 核心库迁移到 OpenScript
- [ ] `opt-cli/` — CLI 迁移到 OpenScript
- [ ] `opk-cli/` — 命令行工具迁移到 OpenScript

## 快速开始

### 构建 Desktop 工具链

```bash
# 1. 构建 libopt 核心库
cd libopt && cargo build --release

# 2. 构建 opt CLI
cd ../opt-cli && cargo build --release

# 3. 构建 opt GUI (需要 Qt6)
cd ../opt-gui
mkdir build && cd build
cmake .. && make
```

### 运行 TUI

```bash
cd opk-ui
pip install -r requirements.txt
python -m opk_ui.app
```

### 启动仓库服务器

```bash
cd opk-server
go run . --repo=/var/opt/repo --addr=:8080
```

## 命令参考

### opt (桌面版)

| 命令 | 别名 | 说明 |
|------|------|------|
| `opt install <pkg.opt>` | `i` | 安装 .opt 包 |
| `opt remove <pkg>` | `rm` | 移除包 |
| `opt update` | `u` | 更新包列表 |
| `opt upgrade` | `up` | 升级所有包 |
| `opt search <q>` | `s` | 搜索包 |
| `opt info <pkg>` | `show` | 包详情 |
| `opt list` | `ls` | 列出包 |
| `opt build <dir>` | `b` | 从目录构建 .opt 包 |
| `opt check <file>` | `check` | 验证 .opt 包 |
| `opt extract <file>` | `x` | 提取 .opt 包 |
| `opt repo add <url>` | `r` | 添加仓库 |

### opk (移动版)

| 命令 | 别名 | 说明 |
|------|------|------|
| `opk install <pkg>` | `i` | 安装包 |
| `opk remove <pkg>` | `rm` | 移除包 |
| `opk update` | `u` | 更新包列表 |
| `opk upgrade` | `up` | 升级所有包 |
| `opk search <q>` | `s` | 搜索包 |
| `opk info <pkg>` | `show` | 包详情 |
| `opk list` | `ls` | 列出包 |
| `opk clean` | `cc` | 清理缓存 |
| `opk repo add <url>` | `r` | 添加仓库 |
| `opk version` | - | 版本信息 |

## 包格式

### 桌面版 (.opt)

```
mypackage_1.0.0_amd64.opt
├── opt.json              # 包元数据 (JSON，含桌面特有字段)
├── data.zip              # 应用文件 (内嵌 ZIP)
├── control.tar.gz        # 安装脚本 (preinst/postinst/prerm/postrm)
└── checksums.sha256      # 完整性校验
```

> `.opt` 专为 OPENOS 桌面应用设计，包含 `.desktop` 文件、图标、AppStream 元数据。
> 格式规范详见 [`docs/opt-format.md`](docs/opt-format.md)。

## 许可证

GPL-3.0-or-later
