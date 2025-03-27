# 🚀 SmartNPC Game - 项目启动指南

这是一个使用 Unreal Engine 构建的 GPT 智能 NPC 对话游戏项目。

## 📦 项目环境要求

- **Unreal Engine**：建议使用 **4.27**（需和项目版本一致）
- **Visual Studio**：推荐 2019 或 2022（需安装 C++ 和游戏开发组件）
- **Git**：用于克隆项目
- **Epic Games 账号**：下载和使用 UE 引擎

---

## 🛠 安装步骤

### 1️⃣ 安装 Epic Games Launcher 并配置 Unreal Engine

- 下载地址：https://www.unrealengine.com/download
- 安装完成后打开 Launcher → 登录账号
- 进入“库（Library）”页面 → 安装 Unreal Engine 4.27（或与项目一致的版本）

### 2️⃣ 克隆项目到本地

```bash
git clone https://github.com/BaconTaro/SmartNPCGame.git
cd SmartNPCGame
```

> ✅ 你也可以通过 GitHub Desktop 克隆项目。

### 3️⃣ 生成 VS 工程文件（第一次打开）

方法一（推荐）：
- 右键 `.uproject` 文件 → 选择 `Generate Visual Studio project files`

方法二：
- 双击 `.uproject` 文件 → 如果提示生成项目文件，点击确认

### 4️⃣ 打开项目

- 使用 Visual Studio 打开 `.sln` 文件，或者
- 双击 `.uproject` 文件直接进入编辑器

### 5️⃣ 编译项目（C++ 项目）

- 第一次打开会自动编译，也可以在 Visual Studio 中编译：
  - `Ctrl + Shift + B` 构建整个解决方案

---

## 📁 项目结构简要说明

| 目录         | 说明                            |
|--------------|---------------------------------|
| `Config/`    | UE 项目的基础配置               |
| `Content/`   | 蓝图资源、UI、材质等内容        |
| `Source/`    | 所有 C++ 源码                   |
| `.uproject`  | 项目主入口                      |
| `.gitignore` | 忽略文件配置                    |

---

## 🧠 常见问题

### Q: 项目无法打开 / 转换版本？
A: 请确认安装的 UE 版本与你项目一致（比如 4.27），否则可能提示转换。

### Q: 蓝图资源缺失？
A: 请确保你完整拉取了 `Content/` 文件夹，如果缺失请联系管理员重新上传或检查 `.gitignore`。

### Q: 打不开 `.uproject`？
A: 请先右键 `.uproject` → **生成 VS 项目文件**

---

## 📌 项目协作建议

- 使用 `main` 分支为主干，功能开发建议在其他分支进行（如 `feature/dialog-system`）
- 每次提交前运行本地测试，确保不破坏现有逻辑
- 如有蓝图冲突，请提前沟通并协调合并流程（蓝图非文本，冲突较难解决）