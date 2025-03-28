# 🔍 SmartNPC Game - 精细任务拆分与实现方式说明

## 🕹️ 1. 游戏入口 / 开场（成员C）

- 创建 `WBP_StartMenu` 蓝图 → 放置标题 `Text` + 按钮 `StartGame`
- `StartGame` 按钮绑定事件 → 调用 `OpenLevel` 或切换到 UI 说明界面
- `WBP_IntroText` 蓝图用于过场说明 → 加入文字数组 + `Delay` 分步展示
- 使用 `Level Sequence` 或 `UMG Animation` 实现淡入淡出动画

## 🧱 2. 地图场景搭建（成员A）

- 使用 `BSP` 或自定义 `StaticMesh` 搭建房间结构
- 从 `Megascans` 或 `StarterContent` 拿素材放置家具
- 制作 NPC 躲藏点：使用 `TriggerBox` + 添加 `Tag` 标记
- 配置基础光照 + `PostProcessVolume` 设置氛围
- 在编辑器中标记摄像头位置（用 `Billboard` 或空物体）

## 🎥 3. 摄像头系统开发（成员B）

- 创建 `BP_CameraActor`，继承 `CameraActor` 或使用 `SceneCaptureComponent2D`
- 创建摄像头管理器，在 `GameMode` 或 `CameraManager` 中注册所有摄像头
- 切换视角：调用 `SetViewTargetWithBlend` 实现自然过渡
- 鼠标右键控制视角：`SpringArm` + 控制 `Yaw/Pitch` 实现旋转
- 摄像头编号用 `TextRender` 或绑定小的 `Widget` 显示

## 🧭 4. 任务目标与提示系统（成员C）

- 创建 `WBP_TaskObjective` 蓝图 → 使用 `TextBlock` 显示目标内容
- 使用 `GameInstance` 或 `GameState` 管理任务进度变量
- 每次切换任务内容播放 `UMG Animation` 作为提示过渡
- 与摄像头系统联动：广播当前摄像头编号 → UI 动态更新提示

## 🖱 5. 摄像头提示UI + 切换逻辑（成员B）

- 每个摄像头生成一个按钮 → `WBP_CameraButton` 包含图标+文本
- 鼠标点击按钮时：通过 `SetViewTargetWithBlend` 切换视角
- 支持右上角回主控位按钮：回到最初 ViewTarget
- 鼠标 Hover 时：使用 `Tooltip` 或自制悬浮框显示房间名

## 🧍 6. NPC交互系统（成员D）

- NPC 使用 `AIController` + `NavMesh` 实现路径寻路
- 蓝牙耳机区域放置 `TriggerBox` → 检测进入后切换为“连接状态”
- NPC 感知摄像头通过 `PawnSensing` 或 `LineTrace` 实现
- 接收指令后用 `AI MoveTo` 节点移动到目标点
- 角色面向目标：使用 `FindLookAtRotation` + `RInterpTo`

## 🔑 7. 可交互物品系统（成员D）

- 创建 `BP_InteractableBase` 类 → 所有交互物品继承
- 每个物体配置 `Tag`（如 `KeyCard`, `Door`）与 `OnInteract()` 方法
- 玩家通过 UI 选择目标物品 → NPC 路径导航到其位置
- 与物品交互后发送通知 → 使用 `Blueprint Interface` 回调反馈

## 🎧 8. 与NPC建立对话链接（成员D + E）

- 玩家点击 `WBP_ConnectButton` → 切换 `GameMode` 状态为 “ChatMode”
- NPC 执行动作（如点头）表示连接建立
- 当前摄像头编号 + 房间内容 作为 GPT Prompt 上下文 → 传递给 Handler
- 激活 `WBP_ChatUI` → 开始输入并展示回复

## 🤖 9. GPT对话内容与提示词（成员E）

- 每个 NPC 携带一个 `UChatGPTHandler`，含 `PersonaPrompt`
- 游戏内物品 + 当前视角拼接成上下文 Prompt
- 使用 UE HTTP 请求调用 GPT 接口（或 OpenRouter / 自部署 API）
- 结果通过事件传回 UI，或调用 `BlueprintImplementableEvent` 实现交互更新

## 💬 10. 对话框系统（成员C）

- 创建 `WBP_ChatUI`：含 `EditableTextBox`（玩家输入）+ `ScrollBox`（消息历史）
- Enter 键发送内容 → 调用 `ChatGPTHandler.SendMessage()`
- GPT 回复使用 `CreateWidget` 创建消息组件 → 加入 `ScrollBox`
- ESC 键或关闭按钮调用 `RemoveFromParent()` + 切回游戏输入模式