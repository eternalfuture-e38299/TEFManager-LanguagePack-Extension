# TEFManager-LanguagePack-Extension

## 📖 项目介绍

TEFManager 语言包扩展模块，一个基于 TEFKernel 框架的跨平台 Terraria 语言包加载系统。

### ✨ 核心特性

- **🌍 跨平台支持** - 基于 TEFKernel，支持 Android / Windows / Linux / macOS 全平台
- **📦 多语言包加载** - 支持同时加载多个语言包，按优先级自动合并
- **🧩 补丁包系统** - 支持语言补丁包，可对现有语言进行增量修改
- **⚡ 轻量高效** - 专为移动设备优化，加载速度快，内存占用低
- **🔄 智能回退** - 当翻译缺失时，自动使用原版对应语言的文本补全
- **🎨 可自定义显示名** - 支持在游戏中显示自定义语言名称

---

## 🏗️ 架构设计

### 核心模块

```text
TEFManager-LanguagePack-Extension/
├── core.hpp/cpp          # 核心模块，管理语言包索引和生命周期
├── LanguagePack.hpp/cpp  # 语言包类，负责解压和解析语言包
├── LanguageManager.hpp/cpp # 语言管理器，Hook 游戏语言加载流程
└── Log.hpp              # 跨平台日志系统
```

### 数据结构

```cpp
// 语言包信息
struct PackInfo {
    std::string name;           // 语言包名称
    std::string author;         // 作者
    std::string languagecode;   // 语言代码 (如 zh-Hans, en-US)
    PackType type;              // LanguagePack 或 PatchPack
    std::string fileextension;  // 文件扩展名 (默认 json)
    std::string displayname;    // 游戏中显示的名称
};

// 语言包类
class LanguagePack {
    PackEntry _entry;                              // 文件条目
    PackInfo _info;                                // 包信息
    std::unordered_map<std::string, std::vector<uint8_t>> _data;  // 文件数据缓存
};
```

---

## 📦 语言包制作指南

### 目录结构

```text
语言包名称.zip
├── pack_info.json           # TEFManager 包信息 (必需)
├── config.json              # 语言包配置 (必需)
├── icon.png                 # 图标文件 (可选)
└── localization/            # 语言文件目录
    ├── Base.json            # 基础翻译
    ├── Game.json            # 游戏核心内容
    ├── Items.json           # 物品翻译
    ├── NPCs.json            # NPC翻译
    ├── Projectiles.json     # 弹幕翻译
    ├── Town.json            # 城镇相关翻译
    ├── Legacy.json          # 世界相关翻译
    └── Mobile.json          # 移动端设置相关翻译
```

---

### pack_info.json (TEFManager 包信息)

**TEFManager 框架要求的包描述文件，安装时唯一读取。**

#### 语言包类型
```json
{
    "name": "我的语言包",
    "author": "eternalfuture",
    "version": "1.0.0",
    "description": "简体中文语言包",
    "type": "LanguagePack"
}
```

#### 语言补丁包类型
```json
{
    "name": "我的语言修补包",
    "author": "eternalfuture",
    "version": "1.0.0",
    "description": "自定义 UI 文本修补",
    "type": "LanguagePatchPack"
}
```

#### 字段说明

| 字段          | 必填 | 类型   | 说明                                  |
|---------------|------|--------|---------------------------------------|
| `name`        | ✅   | string | 包名称                                |
| `author`      | ✅   | string | 作者信息                              |
| `version`     | ✅   | string | 语义化版本 (如 1.0.0)                 |
| `description` | ✅   | string | 简短描述                              |
| `type`        | ✅   | string | `LanguagePack` 或 `LanguagePatchPack` |

---

### config.json (语言包配置)

**语言包运行时的内部配置，用于加载逻辑。`type` 值需与 `pack_info.json` 保持一致。**

#### 语言包类型
```json
{
    "name": "我的语言包",
    "author": "eternalfuture",
    "languagecode": "zh-Hans",
    "displayname": "简体中文 (官方推荐)",
    "type": "LanguagePack",
    "fileextension": "json"
}
```

#### 语言补丁包类型
```json
{
    "name": "我的语言修补包",
    "author": "eternalfuture",
    "languagecode": "zh-Hans",
    "displayname": "自定义 UI 补丁",
    "type": "LanguagePatchPack",
    "fileextension": "json"
}
```

#### 字段说明
| 字段            | 必填 | 类型   | 说明                                                              |
|-----------------|------|--------|-------------------------------------------------------------------|
| `name`          | ✅   | string | 包名称                                                            |
| `author`        | ✅   | string | 作者信息                                                          |
| `languagecode`  | ✅   | string | 语言代码 (见下方支持列表)                                         |
| `displayname`   | ❌   | string | 游戏中显示的名称，默认 `{name} by {author}` (来自 pack_info.json) |
| `type`          | ✅   | string | 必须与 `pack_info.json` 中的 `type` 值一致                        |
| `fileextension` | ❌   | string | 语言文件扩展名，默认 `json`                                       |
---

### 字段来源对照

| 信息            | 来源文件                         | 说明                 |
|-----------------|----------------------------------|----------------------|
| `name`          | `pack_info.json`                 | 包名称               |
| `author`        | `pack_info.json`                 | 作者                 |
| `version`       | `pack_info.json`                 | 版本号               |
| `description`   | `pack_info.json`                 | 描述                 |
| `type`          | `pack_info.json` / `config.json` | 包类型（两处需一致） |
| `languagecode`  | `config.json`                    | 语言代码             |
| `displayname`   | `config.json`                    | 游戏内显示名         |
| `fileextension` | `config.json`                    | 文件扩展名           |

---

### 支持的语言代码

| 语言代码  | 语言名称              |
|-----------|-----------------------|
| `en-US`   | English               |
| `de-DE`   | German                |
| `it-IT`   | Italian               |
| `fr-FR`   | French                |
| `es-ES`   | Spanish               |
| `ru-RU`   | Russian               |
| `zh-Hans` | Chinese (Simplified)  |
| `zh-Hant` | Chinese (Traditional) |
| `pt-BR`   | Portuguese (Brazil)   |
| `pl-PL`   | Polish                |
| `ja-JP`   | Japanese              |
| `ko-KR`   | Korean                |

---

## 🎯 语言文件规范

### JSON 文件格式

所有语言文件使用 **UTF-8** 编码，结构如下：

```json
{
    "UI.Back": "返回",
    "UI.Close": "关闭",
    "UI.Confirm": "确认",
    "Game.WorldLoad": "正在加载世界..."
}
```
* 或者

```json
{
   "UI":
   {
      "Back": "返回",
      "Close": "关闭",
      "Confirm": "确认"
   },
   "Game":
   {
      "WorldLoad": "正在加载世界..."
   }
}
```

### 原版游戏文件命名

| 文件名             | 用途               |
|--------------------|--------------------|
| `Base.json`        | 基础 UI 和通用文本 |
| `Game.json`        | 游戏核心内容       |
| `Items.json`       | 所有物品名称和描述 |
| `NPCs.json`        | NPC 名称和对话     |
| `Projectiles.json` | 弹幕和射弹名称     |
| `Town.json`        | 城镇 NPC 相关文本  |
| `Legacy.json`      | 世界生成相关文本   |
| `Mobile.json`      | 移动端专属设置     |

---

## 🔧 安装与配置

### 1. 放置语言包

将语言包 `.zip` 文件放入以下目录：

```
TEFManager/
└── private/
    └── language_packs/
        └── your_language_pack.zip
```

### 2. 主语言包配置文件 (`config.json`)
```json
[
    {
        "file": "zh-Hans.zip",
        "priority": 0,
        "enable": true
    },
    {
        "file": "zh-Hant.zip",
        "priority": 1,
        "enable": true
    }
]
```

### 3. 补丁包配置文件 (`language_patch_packs.json`)

```json
[
    {
        "file": "custom_ui_patch.zip",
        "priority": 10,
        "enable": true
    }
]
```

### 配置字段说明

| 字段       | 类型   | 说明                         |
|------------|--------|------------------------------|
| `file`     | string | ZIP 文件名                   |
| `priority` | int    | 优先级（数值越小越优先加载） |
| `enable`   | bool   | 是否启用                     |

---

## 🚀 工作流程

### 语言包加载流程

```
1. 模块初始化
   ├── 读取 config.json (主语言包)
   ├── 读取 language_patch_packs.json (补丁包)
   ├── 按优先级排序
   └── 建立索引 (languagecode → LanguagePack)

2. Hook 游戏语言加载
   ├── SetLanguageHook
   │   ├── 拦截 SetLanguage 调用
   │   ├── 查找匹配的语言包
   │   └── 加载语言列表 UI
   └── LoadLanguageHook
       ├── 拦截 LoadLanguage 调用
       ├── 加载主语言包的所有 JSON 文件
       ├── 加载补丁包的所有 JSON 文件
       └── 刷新 UI 文本
```

### 优先级规则

1. **补丁包叠加**：先加载主语言包，再叠加补丁包
2. **配置优先级**：`priority` 数值越小，加载顺序越靠前
3. **回退机制**：缺失的文本自动使用 `languagecode` 指定的语言

---

## 🛠️ 开发指南

### 环境要求

- TEFKernel 框架 (v1.0.0+)
- CMake 3.10+
- C++17 兼容编译器

### 构建

```bash
mkdir build && cd build
cmake .. -DTEFKERNEL_PATH=/path/to/TEFKernel
make
```

### 依赖库

| 库              | 用途                 |
|-----------------|----------------------|
| `miniz`         | ZIP 文件解压         |
| `nlohmann/json` | JSON 解析            |
| `TEFKernel`     | 核心框架和 Hook 系统 |

---

## 📊 平台支持

| 平台    | 支持状态 | 备注     |
|---------|----------|----------|
| Android | ✅       | 完整支持 |
| Windows | ✅       | 完整支持 |
| Linux   | ✅       | 完整支持 |
| macOS   | ✅       | 完整支持 |

---

## 🤝 贡献指南

1. Fork 本仓库
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

---

## 📝 常见问题
**Q: 如何解决翻译冲突？**
A: 系统会按优先级排序，补丁包会覆盖对应语言的同键值翻译。

**Q: 能否只翻译部分内容？**
A: 可以，系统会自动使用 `languagecode` 指定的语言补全缺失翻译。

**Q: 如何更新语言包？**
A: 修改后更新 `version` 字段，用户可无缝升级。

**Q: 移动端和 PC 端语言包通用吗？**
A: 完全通用，使用相同的 ZIP 格式和 JSON 结构。

**Q: `pack_info.json` 和 `config.json` 的 `type` 必须一致吗？**
A: 是的，两者必须保持一致，否则语言包可能无法正确加载。
---

## 🙏 致谢
- [TEFKernel](https://github.com/eternalfuture-e38299/TEFKernel) - 核心框架
- [miniz](https://github.com/richgel999/miniz) - ZIP 解压库
- [nlohmann/json](https://github.com/nlohmann/json) - JSON 解析库
---

## 🔗 相关链接
- [问题反馈](https://github.com/eternalfuture-e38299/TEFManager-LanguagePack-Extension/issues)
- [TEFKernel 文档](https://github.com/eternalfuture-e38299/TEFKernel)
---