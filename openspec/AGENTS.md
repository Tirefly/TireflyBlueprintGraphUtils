# OpenSpec 使用说明

面向在 TireflyBlueprintGraphUtils 插件中使用 OpenSpec 进行规格驱动开发的 AI 编码助手说明。

## 作用范围与路径

- 这套 OpenSpec 工作区位于 `Plugins/TireflyBlueprintGraphUtils/openspec`。
- 所有 OpenSpec CLI 命令都应在 `Plugins/TireflyBlueprintGraphUtils` 目录下执行。
- 除非另有说明，`openspec/`、`Source/`、`Documents/`、`Resources/` 等路径都相对于插件根目录。
- 插件外的仓库级路径仍保持仓库相对路径写法。

## 快速检查清单

- 先进入插件根目录：`cd Plugins/TireflyBlueprintGraphUtils`
- 搜索已有工作：`openspec list`、`openspec list --specs`，全文检索优先用 `rg`
- 判断范围：是新增 capability，还是修改已有 capability
- 选择唯一的 `change-id`：使用 kebab-case，并以动词开头，例如 `add-`、`update-`、`remove-`、`refactor-`
- 搭建骨架：`proposal.md`、`tasks.md`、必要时创建 `design.md`，以及按 capability 划分的 delta spec
- 编写 delta：使用 `## ADDED|MODIFIED|REMOVED|RENAMED Requirements`，每个 requirement 至少包含一个 `#### Scenario:`
- 校验：执行 `openspec validate <change-id> --strict --no-interactive` 并修复问题
- 等待批准：提案批准前不要开始实现

## 工作流

### 阶段 1：创建变更提案

当请求涉及新增功能、架构调整、行为变化、编辑器扩展或新的 authoring 流程时，应先创建 proposal。

标准流程：

1. 阅读 `openspec/project.md`。
2. 检查 `openspec/changes/` 中是否已有相关变更。
3. 选择唯一且以动词开头的 `change-id`。
4. 在 `openspec/changes/<change-id>/` 下创建 `proposal.md`、`tasks.md`、必要时创建 `design.md`。
5. 在 `openspec/changes/<change-id>/specs/<capability>/spec.md` 中编写 delta。
6. 执行 `openspec validate <change-id> --strict --no-interactive`。

### 阶段 2：实现变更

1. 阅读 `proposal.md`。
2. 阅读 `design.md`（如果存在）。
3. 阅读 `tasks.md`。
4. 按任务顺序实现。
5. 完成后更新 `tasks.md` 勾选状态。
6. 编译验证相关 Editor 目标。

### 阶段 3：归档变更

完成并验证后，将 change 归档到 `openspec/changes/archive/YYYY-MM-DD-<change-id>/`，并在需要时同步更新 `openspec/specs/`。

## Spec 文件格式

每个 requirement 必须至少包含一个 scenario：

```markdown
## ADDED Requirements

### Requirement: 示例能力
系统 SHALL 提供某个能力。

#### Scenario: 示例场景
- **WHEN** 用户执行某个动作
- **THEN** 系统产生预期结果
```

## 注意事项

- Specs 是已落地事实契约，Changes 是待批准提案。
- 活动 change 未批准前，不应直接实现功能代码。
- 结构体编辑器相关方案应优先避免依赖 Kismet 私有 UI 实现。
- 本插件是 Editor 插件，新增能力默认只作用于编辑器，不应引入运行时依赖。
