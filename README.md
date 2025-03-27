# SmartNPC Game 🎮🧠

基于 GPT 技术的虚幻引擎智能 NPC 项目。玩家可与 NPC 实现实时对话，每个 NPC 拥有独立人物设定与记忆系统，支持蓝图与 C++ 扩展，适用于 AI 游戏研发、学术研究等场景。

---

## 🚀 项目特点

- 🤖 支持 GPT-3.5 / GPT-4 消息处理（C++ 接入 OpenAI API）
- 🧠 每个 NPC 拥有独立 Prompt 与对话记忆
- 🧩 基于 UE4 蓝图 + C++ 混合开发架构
- 🗣️ 支持玩家按 F 键进入聊天输入模式
- 📦 模块化组件：`ChatGPTHandler` 可复用至任意 Actor/NPC

---

## 🏗️ 技术栈

- Unreal Engine 4.27
- C++ + 蓝图混合架构
- OpenAI Chat Completions API
- JSON 序列化（FJsonObject）
- GitHub 版本管理（含 `.gitignore`）

---

## 🕹️ 使用说明

1. 启动项目（UE4 编辑器）
2. 将 `BP_NPCBase` 拖入场景
3. 在 NPC 的 `ChatGPTHandler` 中选择角色类型（对应 Prompt）
4. 玩家靠近 NPC 显示 “按 F 说话”
5. F 开启对话框，输入消息并回车，GPT 回复即展示

---

## 📁 文件结构（简略）

```
📦 SmartNPCGame/
├─ Source/
│  ├─ ChatGPTHandler.{h,cpp}      # GPT 请求处理组件（ActorComponent）
│  ├─ NPCPromptConfig.{h,cpp}     # 可选：配置存储所有角色 Prompt
│  └─ ...
├─ Content/
│  └─ UI/                         # WBP_ChatUI、WBP_Message 等
├─ Config/
├─ SmartNPCGame.uproject
└─ .gitignore
```

---

## 🧠 后续计划

- [ ] ✅ 支持多轮上下文记忆压缩
- [ ] ✅ 引入情绪状态 / 角色标签系统
- [ ] ✅ 自动生成 NPC 对话数据集（可训练）
- [ ] ✅ 支持 GPT 回答语音朗读（TTS）
- [ ] ✅ 升级至 UE5

---

## 🤝 贡献说明

欢迎参与开发、PR、提建议！你可以：

- Fork 本项目，自定义角色 Prompt 配置
- 提交 Bug 或新特性建议
- 基于本项目开发 Demo/教学案例

---

## 📜 License

本项目以 MIT 协议开源，使用时请注明原作者出处。
