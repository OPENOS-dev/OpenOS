# OPT 包格式规范

> **OPENOS 桌面版应用安装包格式** — 基于 ZIP，面向 ChromiumOS 环境。

**后缀**: `.opt`  
**格式**: ZIP 压缩包  
**目标**: ChromiumOS 桌面应用分发（原生 Linux / Crostini / ARCVM）

---

## 设计原则

1. **自包含** — 一个 `.opt` 包含应用的所有文件、元数据、图标和安装逻辑
2. **零依赖安装** — 解析 ZIP 即可读取元数据，无需额外工具
3. **可验证** — SHA-256 完整性校验 + 可选的 GPG 签名
4. **桌面原生** — 直接包含 `.desktop` 文件、图标和 AppStream 元数据
5. **与 OPK 互补** — OPK 管移动端，OPT 管桌面端，元数据格式对齐

---

## 包结构

```
firefox_120.0_amd64.opt
├── opt.json                # 包元数据（必要）
├── data.zip                # 应用文件（必要）
│   ├── opt/<app>/          #   → /opt/<app>/     （主安装目录）
│   ├── usr/bin/            #   → /usr/bin/       （可执行文件）
│   ├── usr/share/          #   → /usr/share/     （共享资源）
│   └── ...                 #   相对 / 的任意路径
├── control.tar.gz          # 安装脚本（可选）
│   ├── preinst             #   安装前执行
│   ├── postinst            #   安装后执行
│   ├── prerm               #   移除前执行
│   └── postrm              #   移除后执行
└── checksums.sha256        # SHA-256 校验（必要）
```

### 层级说明

| 条目 | 必要性 | 说明 |
|------|--------|------|
| `opt.json` | ✅ 必要 | 包元数据，必须为包内第一个文件（方便流式读取） |
| `data.zip` | ✅ 必要 | 内嵌 ZIP，包含实际安装文件，路径相对于 `/` |
| `control.tar.gz` | ❌ 可选 | gzip 压缩的 tar 包，内含安装脚本 |
| `checksums.sha256` | ✅ 必要 | 包内所有文件的 SHA-256 哈希（格式同 `sha256sum`） |

> **为什么 data 用 zip 而非 tar.gz？**  
> 因为 OPT 包本身已经是 ZIP，内层 `data.zip` 可以被直接提取或挂载（`mount -o loop`），无需二次解压。外层的 control 信息保留为 tar.gz 以保持与 OPK 一致的脚本处理流程。

---

## 元数据 (opt.json)

```json
{
  "$schema": "https://openos.org/schemas/opt/v1.json",

  "name": "firefox",
  "version": "120.0",
  "description": "Mozilla Firefox web browser",
  "section": "net",
  "maintainer": "OPENOS Team <packages@openos.org>",
  "homepage": "https://firefox.com",
  "license": "MPL-2.0",

  "architecture": "amd64",
  "package_size": 52428800,
  "installed_size": 157286400,
  "filename": "firefox_120.0_amd64.opt",
  "sha256": "abcdef1234567890...",

  "depends": ["libgtk-3-0", "libdbus-1-3"],
  "recommends": ["openssh-client"],
  "suggests": ["firefox-langpack-zh-cn"],
  "provides": ["web-browser"],
  "conflicts": ["chromium"],
  "replaces": ["firefox-esr"],

  "priority": "optional",
  "tags": ["browser", "web", "gecko"],
  "origin": "openos-community",

  "desktop_file": "firefox.desktop",
  "appstream_id": "org.mozilla.firefox",
  "categories": ["Network", "WebBrowser"],
  "screenshots": [
    "https://screenshots.openos.org/firefox/1.png"
  ],

  "app_type": "gui",
  "runtime": "native",
  "min_kernel": "5.10",
  "chromium_features": ["wayland", "crostini"],
  "permissions": ["network", "notifications"],
  "xterm": false
}
```

### 通用字段（与 OPK 对齐）

| 字段 | 类型 | 必需 | 说明 |
|------|------|------|------|
| `name` | string | ✅ | 包名，全小写字母数字和连字符 |
| `version` | string | ✅ | 语义化版本（SemVer） |
| `description` | string | ✅ | 一行简短描述（≤80 字符） |
| `section` | string | ✅ | 分类: `base`, `devel`, `graphics`, `net`, `utils`, `games`, `office`, `science`, `system`, `multimedia` |
| `maintainer` | string | ✅ | 维护者邮箱或名称 |
| `homepage` | string | ❌ | 项目主页 URL |
| `license` | string | ✅ | SPDX 许可证标识符 |
| `architecture` | string | ✅ | `amd64`, `arm64`, `armhf`, `i386`, `all` |
| `package_size` | integer | ✅ | 包文件大小（字节） |
| `installed_size` | integer | ✅ | 安装后预估大小（字节） |
| `filename` | string | ✅ | 标准包文件名 |
| `sha256` | string | ✅ | 包文件的 SHA-256 摘要 |

### 依赖字段

| 字段 | 类型 | 必需 | 说明 |
|------|------|------|------|
| `depends` | string[] | ❌ | 硬依赖 |
| `recommends` | string[] | ❌ | 推荐安装（不安装不影响核心功能） |
| `suggests` | string[] | ❌ | 建议安装（增强体验） |
| `provides` | string[] | ❌ | 提供的虚拟包（如 `web-browser`） |
| `conflicts` | string[] | ❌ | 冲突包 |
| `replaces` | string[] | ❌ | 替代的旧包 |

### 桌面特有字段（新增于 OPT）

| 字段 | 类型 | 必需 | 说明 |
|------|------|------|------|
| `desktop_file` | string | ✅ | `.desktop` 文件名（位于 `data.zip` 的 `usr/share/applications/` 下） |
| `appstream_id` | string | ❌ | AppStream 反向域名 ID，如 `org.mozilla.firefox` |
| `categories` | string[] | ❌ | 应用分类标签，同 Freedesktop 菜单规范 |
| `screenshots` | string[] | ❌ | 截图 URL 列表（用于软件中心展示） |
| `app_type` | string | ✅ | 应用类型: `gui`, `cli`, `daemon`, `library`, `service` |
| `runtime` | string | ✅ | 运行时: `native`, `crostini`, `arcvm`, `flatpak`, `snap`, `container` |
| `min_kernel` | string | ❌ | 最低内核版本要求，如 `5.10` |
| `chromium_features` | string[] | ❌ | 需要的 ChromiumOS 特性: `wayland`, `crostini`, `arcvm`, `vmc`, `selinux`, `cgroup2` |
| `permissions` | string[] | ❌ | 应用权限声明: `network`, `filesystem`, `camera`, `microphone`, `bluetooth`, `notifications`, `clipboard`, `usb`, `serial` |
| `xterm` | boolean | ❌ | 是否需要在终端中运行（默认 `false`） |

---

## 标准包文件名

```
<name>_<version>_<architecture>.opt
```

示例:

| 包 | 文件名 |
|----|--------|
| Firefox 120.0 (amd64) | `firefox_120.0_amd64.opt` |
| VS Code 1.92.0 (arm64) | `code_1.92.0_arm64.opt` |
| 平台无关的 Python 库 | `python3-requests_2.32.0_all.opt` |

---

## 安装路径约定

`data.zip` 内的路径相对于系统根目录 `/`。约定如下：

| data.zip 内路径 | 安装到系统 | 说明 |
|----------------|-----------|------|
| `opt/<app>/` | `/opt/<app>/` | 应用主目录（推荐） |
| `usr/bin/<app>` | `/usr/bin/<app>` | 可执行文件（或符号链接到 `/opt/<app>/bin/`） |
| `usr/share/applications/<app>.desktop` | `/usr/share/applications/<app>.desktop` | .desktop 入口文件 |
| `usr/share/icons/hicolor/*/apps/<app>.png` | `/usr/share/icons/...` | 应用图标 |
| `usr/share/metainfo/<app>.appdata.xml` | `/usr/share/metainfo/<app>.appdata.xml` | AppStream 元数据 |
| `usr/share/doc/<app>/` | `/usr/share/doc/<app>/` | 文档 |
| `etc/<app>/` | `/etc/<app>/` | 配置文件 |

> **安装策略**: 优先安装到 `/opt/<app>/` 而非直接散落 `/usr/bin/`，便于版本管理和卸载清理。`/usr/bin/` 下只放符号链接或包装脚本。

---

## 安装脚本 (control.tar.gz)

脚本规范同 Debian 包，包内结构：

```
control.tar.gz
├── preinst     # 安装前 — 准备环境、创建用户等
├── postinst    # 安装后 — 注册 .desktop、更新图标缓存、启动服务等
├── prerm       # 移除前 — 停止服务等
└── postrm      # 移除后 — 清理配置文件等
```

所有脚本可选，以 **退出码 0** 表示成功，非零表示失败。

### 预定义变量（由安装器注入）

| 变量 | 说明 |
|------|------|
| `$OPT_PKG_NAME` | 包名 |
| `$OPT_PKG_VERSION` | 包版本 |
| `$OPT_PKG_DIR` | 应用安装目录（通常为 `/opt/<name>/`） |
| `$OPT_DATA_DIR` | data.zip 提取的临时路径 |

---

## 安装流程

```
1. 校验 checksums.sha256
2. 解析 opt.json，验证必需字段
3. 检查依赖是否满足（depends）
4. 检查冲突（conflicts）
5. 检查内核版本和 ChromiumOS 特性要求
6. 执行 preinst（如有）
7. 提取 data.zip 到系统根目录
8. 执行 postinst（如有）
   └─ 默认行为：更新 .desktop 数据库、图标缓存
9. 标记包为已安装（写入 /var/lib/opt/packages/）
```

---

## 卸载流程

```
1. 检查 /var/lib/opt/packages/ 中的记录
2. 执行 prerm（如有）
3. 删除 data.zip 中记录的所有文件
4. 执行 postrm（如有）
5. 从 /var/lib/opt/packages/ 移除记录
```

---

## 校验文件 (checksums.sha256)

格式同 `sha256sum`：

```
d2d2d2d2d2d2d2d2d2d2d2d2d2d2d2d2d2d2d2d2d2d2d2d2d2d2d2d2d2d2d2d2  opt.json
e3e3e3e3e3e3e3e3e3e3e3e3e3e3e3e3e3e3e3e3e3e3e3e3e3e3e3e3e3e3e3e3  data.zip
f4f4f4f4f4f4f4f4f4f4f4f4f4f4f4f4f4f4f4f4f4f4f4f4f4f4f4f4f4f4f4f4  control.tar.gz
```

---

## 包仓库索引

OPT 仓库使用 JSON 索引（对应 `Packages.json`），与 OPK 仓库格式对齐：

```json
{
  "origin": "openos-community",
  "label": "OPENOS Community Repository",
  "description": "Community-maintained packages for OPENOS Desktop",
  "components": ["main", "contrib", "community"],
  "architectures": ["amd64", "arm64"],
  "packages": [ /* <opt.json 内容> */ ],
  "index_sha256": "...",
  "generated_at": "2026-07-03T10:00:00Z"
}
```

---

## 与 OPK 的差异对比

| 维度 | OPK (移动版) | OPT (桌面版) |
|------|-------------|-------------|
| 包后缀 | `.opk` | `.opt` |
| 容器格式 | tar.gz | zip |
| 数据格式 | tar.gz | 内嵌 zip |
| 目标平台 | AOSP Android | ChromiumOS Linux |
| 桌面集成 | ❌ 无 | ✅ .desktop, AppStream, 图标 |
| 运行时类型 | 仅 `native` | `native`, `crostini`, `arcvm` 等 |
| 权限模型 | Android 权限 | ChromiumOS 特性声明 |
| 依赖系统 | apt 兼容 | apt 兼容（共享仓库） |
| 最小元数据 | 通用字段 | 通用 + 桌面字段 |
| 安装路径 | `/data/app/` | `/opt/` + `/usr/` |

---

## 未来扩展

- **GPG 签名**: `opt.json.asc` 附在包内，用于验证包来源
- **增量更新**: 基于 bsdiff 的差分包（`.opt.diff`）
- **Delta 清单**: 用于 OTA 场景
- **AppStream 集成**: 解析 `<app>.appdata.xml` 生成软件中心信息
- **沙箱声明**: 声明应用所需的 Flatpak/Bubblewrap 沙箱规则
