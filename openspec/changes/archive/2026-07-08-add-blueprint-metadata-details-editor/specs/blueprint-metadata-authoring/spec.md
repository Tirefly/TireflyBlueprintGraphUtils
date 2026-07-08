## ADDED Requirements

### Requirement: 通用 MetaData Key/Value 编辑控件
TireflyBlueprintGraphUtils SHALL provide a reusable editor UI for editing MetaData as key/value rows, where the key represents a MetaData name and the value represents the MetaData string value.

#### Scenario: 用户新增 MetaData 行
- **WHEN** 用户在 MetaData 编辑控件中点击新增
- **THEN** 系统 MUST 创建一个可编辑的 Key/Value 行
- **AND** 该行 MUST 在提交有效 Key 后写入目标 MetaData 存储

#### Scenario: 用户编辑 MetaData 值
- **WHEN** 用户修改某一行的 Value 并提交
- **THEN** 系统 MUST 将该 Value 写入对应 Key
- **AND** 空字符串 Value MUST 被视为有效值

#### Scenario: 用户输入重复 Key
- **WHEN** 用户把某一行 Key 改成目标中已经存在的 Key
- **THEN** 系统 MUST 拒绝提交该修改
- **AND** UI MUST 提供明确的错误提示或保持原值

#### Scenario: 用户删除 MetaData 行
- **WHEN** 用户删除某一行 MetaData
- **THEN** 系统 MUST 从目标 MetaData 存储移除该 Key

### Requirement: MetaData 编辑必须支持事务与脏标记
TireflyBlueprintGraphUtils SHALL wrap MetaData mutations in Unreal Editor transactions and mark the owning Blueprint as modified when editing Blueprint variable or function MetaData.

#### Scenario: 变量 MetaData 修改进入 Undo 栈
- **WHEN** 用户修改蓝图变量 MetaData
- **THEN** 修改 MUST 创建 editor transaction
- **AND** 用户 MUST be able to undo and redo the change through the editor

#### Scenario: 函数 MetaData 修改进入 Undo 栈
- **WHEN** 用户修改蓝图函数 MetaData
- **THEN** 修改 MUST 创建 editor transaction
- **AND** 用户 MUST be able to undo and redo the change through the editor

#### Scenario: 修改后蓝图标记为 dirty
- **WHEN** 用户成功新增、编辑、删除或重命名 MetaData
- **THEN** 所属 Blueprint MUST be marked as modified

### Requirement: 蓝图变量 Details 面板必须提供 MetaData 编辑分组
TireflyBlueprintGraphUtils SHALL add a MetaData editing section to Blueprint variable Details panels through Unreal's public Blueprint variable customization extension point.

#### Scenario: 用户选择蓝图成员变量
- **WHEN** 用户在蓝图编辑器中选中一个 Blueprint 成员变量
- **THEN** Details 面板 MUST include a `MetaData` section provided by TireflyBlueprintGraphUtils
- **AND** the section MUST display existing variable MetaData rows

#### Scenario: 用户编辑蓝图变量 MetaData
- **WHEN** 用户在变量 `MetaData` 分组中新增、编辑或删除一项 MetaData
- **THEN** 系统 MUST persist the change to that Blueprint variable's MetaData
- **AND** the change MUST survive Blueprint save, close, reopen, and compile

#### Scenario: 变量 MetaData 编辑使用公开 API
- **WHEN** 插件写入或删除蓝图变量 MetaData
- **THEN** it SHOULD use `FBlueprintEditorUtils::SetBlueprintVariableMetaData` and `FBlueprintEditorUtils::RemoveBlueprintVariableMetaData` or equivalent public APIs
- **AND** it MUST NOT depend on Kismet private detail customization classes

### Requirement: 蓝图函数 Details 面板必须提供 MetaData 编辑分组
TireflyBlueprintGraphUtils SHALL add a MetaData editing section to Blueprint function Details panels through Unreal's public Blueprint function customization extension point.

#### Scenario: 用户选择蓝图函数
- **WHEN** 用户在蓝图编辑器中选中一个用户声明函数
- **THEN** Details 面板 MUST include a `MetaData` section provided by TireflyBlueprintGraphUtils
- **AND** the section MUST display existing function MetaData rows

#### Scenario: 用户编辑蓝图函数 MetaData
- **WHEN** 用户在函数 `MetaData` 分组中新增、编辑或删除一项 MetaData
- **THEN** 系统 MUST persist the change to that function's user-declared function MetaData
- **AND** the change MUST survive Blueprint save, close, reopen, and compile

#### Scenario: 函数 MetaData 编辑使用公开 API
- **WHEN** 插件定位或修改蓝图函数 MetaData
- **THEN** it SHOULD use `FBlueprintEditorUtils::GetGraphFunctionMetaData`, `FBlueprintEditorUtils::ModifyFunctionMetaData`, `FKismetUserDeclaredFunctionMetadata`, or equivalent public APIs
- **AND** it MUST NOT depend on Kismet private detail customization classes

### Requirement: 第一阶段不得实现参数和结构体 MetaData 编辑
The stage-one Blueprint MetaData editor SHALL be limited to the shared Key/Value UI, Blueprint variable MetaData, and Blueprint function MetaData.

#### Scenario: 用户期望编辑函数参数 MetaData
- **WHEN** 用户需要编辑 Blueprint function parameter MetaData such as `UPARAM`-style metadata
- **THEN** this stage-one feature MUST NOT claim support for that workflow
- **AND** parameter MetaData support MUST remain a separate future proposal

#### Scenario: 用户期望编辑 UserDefinedStruct MetaData
- **WHEN** 用户需要编辑 UserDefinedStruct or struct member MetaData
- **THEN** this stage-one feature MUST NOT claim support for that workflow
- **AND** struct MetaData support MUST remain a separate future proposal

### Requirement: MetaData Key 建议不得限制手动输入
TireflyBlueprintGraphUtils SHALL provide optional common MetaData Key suggestions without requiring a complete dynamic enumeration of all UE engine MetaData keys.

#### Scenario: 用户输入建议列表外的 Key
- **WHEN** 用户输入一个不在建议列表中的有效 MetaData Key
- **THEN** 系统 MUST allow the key to be committed

#### Scenario: 用户选择常用 Key
- **WHEN** 用户打开常用 Key 建议入口
- **THEN** 系统 SHOULD provide common keys relevant to the current target type
- **AND** 系统 MUST NOT include keys that already exist on the current target in the suggestions

#### Scenario: 变量和函数使用不同建议列表
- **WHEN** 用户在变量 MetaData 编辑器中打开常用 Key 建议入口
- **THEN** 系统 SHOULD show variable-oriented suggestions such as `GetOptions`, `EditCondition`, and `AllowedClasses`
- **AND** 系统 SHOULD NOT show function-only suggestions such as `WorldContext`

#### Scenario: 蓝图已有显式 UI 的 Key 不进入建议列表
- **WHEN** 用户打开常用 Key 建议入口
- **THEN** 系统 MUST NOT suggest keys that already have dedicated Blueprint editor UI, such as `DisplayName` and `Category`
- **AND** 用户仍可手动输入这些 Key
