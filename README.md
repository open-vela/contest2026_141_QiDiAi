# QiDiAi 建木 — 基于 openvela 的可穿戴设备语义索引

> 2026 首届 openvela AI 硬件开发者大赛 · 队伍 QiDiAi（编号 141）
> 赛道：手表应用创新 + AI 硬件产品创新

## 一、作品简介

**建木**是一个面向 openvela 智能手表 / 手环的**轻量级离线 AI 语义检索应用**。
它将自研 **V10 向量嵌入模型**（1.93M 参数、1024 维、7.3MB 权重、C 实现、仅依赖 libc+libm、推理 0.33-1.55ms）部署于 openvela，
为可穿戴设备提供**毫秒级、完全离线**的语义搜索能力，解决云端大模型在可穿戴场景下的四大盲区：
持续在线功耗、隐私红线、毫秒级响应、离线连续性。

本仓库当前包含一个**可编译、可运行的垂直切片 Demo**（`app/jianmu/`）：
端到端跑通 `文本嵌入 → 本地向量索引 → 离线语义检索`，并已在 openvela 构建树中接好。
**V10 已接入真实引擎**（1024 维输出、7.3MB 权重、纯 libc+libm 零依赖），非占位实现。

### 评测结果

| 指标 | 结果 |
|------|------|
| 语义区分（10组三元组） | 10/10 (100%) |
| 类别准确率（200条语料） | 93% |
| recall@3（200条语料） | 100% |
| 确定性 | cosine = 1.000000 |
| 推理速度 | 0.33-1.55 ms/embed |
| 搜索延迟（200条） | 26.3ms |

## 二、选题方向

- **手表应用创新**：在 openvela 手表 / 手环上提供腕上语义搜索与索引管理 UI。
- **AI 硬件产品创新**：将 V9v3 端侧推理作为核心 AI 能力落地，含 BLE 多设备语义同步、OTA。

## 三、目录结构

```
app/jianmu/            — 核心应用（V10 嵌入 + 本地语义索引 + 离线检索 Demo）
  ├─ jianmu_main.c     — 入口：加载权重 → embed → 建索引 → 检索 全流程演示
  ├─ v10.c             — V10 推理引擎（Pure Source Pool，1.93M 参数）
  ├─ v10.h             — V10 API 头文件（init / embed / dim / free）
  ├─ v10_weights.baize — 7.3MB 模型权重（建议 gitignore / Git LFS）
  └─ semantic_index.*  — 本地向量索引与余弦相似度检索（向量堆分配）
board/contest_board/   — 板级适配骨架（SF32LB52 LCD，Phase 2/3 启用）
quickapp/hello_quickapp/ — 快应用骨架（腕上 UI，Phase 2 启用）
docs/建木-方案.md        — 完整参赛方案（产品定义、选板、功能、技术方案、计划）
docs/v10-evaluation-report.md — V10 引擎评测报告
docs/env-setup.md       — 开发环境搭建（WSL2 + repo 拉取 + 编译烧录）
tools/wsl2-bootstrap.sh — WSL2 一键环境引导脚本
logs/                   — AI Coding 日志（由组委会日志归集工具导出后提交，详见 logs/README.md）
```

## 四、运行方式

> 完整步骤见 [`docs/env-setup.md`](docs/env-setup.md)。要点：

1. **拉取完整工程**（在 WSL2 Ubuntu 中，工作区根目录）：
   ```bash
   repo init -u https://github.com/open-vela/contest2026_141_QiDiAi \
     -b dev-ai-contest-2026 -m contest2026_141_QiDiAi.xml
   repo sync -c -j8
   ```
2. **启用并编译**（在 openvela 工作区根目录，即本仓的上一级）：
   ```bash
   ./build.sh <board-config-path> menuconfig   # 开启 EXAMPLES_JIANMU
   ./build.sh <board-config-path> -j8
   ```
3. **运行**：烧录到开发板，或在 QEMU 模拟器中运行 `jianmu` 命令，观察离线语义检索输出。

## 五、AI Coding 使用说明

本作品全程借助 AI 辅助开发（需求拆解、方案设计、代码骨架、调试、文档）。
- 方案定义与功能拆分见 `docs/建木-方案.md`；
- V10 C 引擎移植、语义索引实现、board config 修复均由 AI 辅助完成；
- V10 评测脚本与报告由 AI 辅助生成；
- 完整 AI 对话日志见 `logs/` 目录（按组委会规范导出）。

## 六、提交须知（评委评估口径）

评委依据「**作品本身 + 本 README 说明 + `logs/` 里的 AI Coding 日志**」评估。
- 所有改动经 **Pull Request** 合入（分支保护，可自行合入自己的 PR）。
- 首次贡献需在官网签署 **CLA**，PR 评论 `/check-cla` 复检。
- **作品提交截止：2026-09-20**，截止后收回 push 权限。
- 官方流程以 [《参赛代码提交指南》](https://github.com/open-vela/docs/blob/dev-ai-contest-2026/zh-cn/contest_2026/code_submission_guide.md) 为准。

### 提交步骤

1. **导出 AI Coding 日志**：使用组委会日志归集工具，导出到 `logs/qidiai/`
2. **签署 CLA**：首次 PR 时在官网签署
3. **创建 PR**：从 `dev-ai-contest-2026` 分支创建 PR
4. **合并 PR**：分支保护，可自行合入
