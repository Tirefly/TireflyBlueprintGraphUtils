# user-defined-struct-metadata Specification

## Purpose
TBD - created by archiving change add-user-defined-struct-metadata-editor. Update Purpose after archive.
## Requirements
### Requirement: 用户定义结构体编辑器工具栏必须提供 MetaData 编辑入口
TireflyBlueprintGraphUtils SHALL add two separate toolbar buttons — `Struct MetaData` and `Member MetaData` — to the UserDefinedStruct editor toolbar via `UAssetEditorSubsystem::OnAssetOpenedInEditor()` and `FAssetEditorToolkit::AddToolbarExtender()`.

#### Scenario: 用户打开用户定义结构体资产
- **WHEN** 用户在内容浏览器中双击打开一个 `UUserDefinedStruct` 资产
- **THEN** 结构体编辑器工具栏 MUST 出现 `Struct MetaData` 和 `Member MetaData` 两个独立按钮
- **AND** 该按钮 MUST 不破坏结构体编辑器原有的成员变量编辑、默认值编辑、编译、保存等功能

#### Scenario: 重复打开同一结构体
- **WHEN** 用户关闭并重新打开同一结构体资产
- **THEN** 系统 MUST NOT 重复添加按钮
- **AND** 工具栏 MUST 只有一个 `Struct MetaData` 按钮和一个 `Member MetaData` 按钮

#### Scenario: 两个按钮弹出独立窗口
- **WHEN** 用户点击 `Struct MetaData` 按钮
- **THEN** 系统 MUST 弹出只包含结构体自身 MetaData 的编辑窗口
- **WHEN** 用户点击 `Member MetaData` 按钮
- **THEN** 系统 MUST 弹出只包含成员变量 MetaData 的编辑窗口

### Requirement: 结构体自身 MetaData 编辑
TireflyBlueprintGraphUtils SHALL provide MetaData editing for the `UUserDefinedStruct` itself using UObject MetaData APIs.

#### Scenario: 用户编辑结构体自身 MetaData
- **WHEN** 用户在 MetaData 编辑窗口的上方面板中新增、编辑或删除一项 MetaData
- **THEN** 系统 MUST persist the change to the `UUserDefinedStruct`'s UObject MetaData
- **AND** the change MUST survive asset save, close, and reopen

#### Scenario: 结构体自身 MetaData 使用公开 API
- **WHEN** 插件读取或修改结构体自身 MetaData
- **THEN** it SHOULD use `UObject::SetMetaData`, `UObject::RemoveMetaData`, and `UObject::GetMetaDataMap` or equivalent public APIs
- **AND** it MUST NOT depend on private struct editor internals

### Requirement: 结构体成员变量 MetaData 编辑
TireflyBlueprintGraphUtils SHALL provide MetaData editing for each member variable of a `UUserDefinedStruct`, grouped by member name, using `FStructureEditorUtils` public APIs.

#### Scenario: 用户编辑结构体成员变量 MetaData
- **WHEN** 用户在 MetaData 编辑窗口的下方面板中为某个成员变量新增、编辑或删除一项 MetaData
- **THEN** 系统 MUST persist the change to that member variable's `FStructVariableDescription::MetaData`
- **AND** the change MUST survive asset save, close, reopen, and struct recompile

#### Scenario: 结构体成员变量 MetaData 使用公开 API
- **WHEN** 插件读取或修改结构体成员变量 MetaData
- **THEN** it SHOULD use `FStructureEditorUtils::SetMetaData` and `FStructureEditorUtils::GetMetaData` or equivalent public APIs
- **AND** it MUST NOT depend on `FUserDefinedStructureDetails` or other private Kismet types

#### Scenario: 成员变量按 Guid 标识
- **WHEN** 插件定位结构体成员变量以读写 MetaData
- **THEN** it MUST use the member's `VarGuid` as the stable identifier
- **AND** member rename MUST NOT cause MetaData loss

### Requirement: 结构体 MetaData 编辑必须支持事务与变更通知
TireflyBlueprintGraphUtils SHALL wrap all struct MetaData mutations in editor transactions and trigger struct change notification after editing.

#### Scenario: 结构体 MetaData 修改进入 Undo 栈
- **WHEN** 用户修改结构体自身或成员变量 MetaData
- **THEN** 修改 MUST 创建 editor transaction
- **AND** 用户 MUST be able to undo and redo the change through the editor

#### Scenario: 修改后资产标记为 dirty
- **WHEN** 用户成功新增、编辑或删除结构体 MetaData
- **THEN** 所属 `UUserDefinedStruct` 的 package MUST be marked as dirty

#### Scenario: 修改后触发结构体变更通知
- **WHEN** 用户完成 MetaData 编辑并关闭编辑窗口
- **THEN** 系统 MUST call `FStructureEditorUtils::OnStructureChanged` or equivalent to trigger struct recompile and dependent asset refresh
- **AND** 引用该结构体的 DataTable 和蓝图属性 MUST reflect the updated MetaData

### Requirement: 结构体 MetaData 常用 Key 建议
TireflyBlueprintGraphUtils SHALL provide common MetaData Key suggestions relevant to struct-level and struct-member-level MetaData respectively.

#### Scenario: 用户在结构体自身 MetaData 编辑器中打开常用 Key 建议
- **WHEN** 用户在结构体级 MetaData 编辑器中打开常用 Key 建议
- **THEN** 系统 SHOULD show struct-oriented suggestions
- **AND** 系统 MUST NOT include keys that already exist on the struct in the suggestions

#### Scenario: 用户在成员变量 MetaData 编辑器中打开常用 Key 建议
- **WHEN** 用户在成员变量级 MetaData 编辑器中打开常用 Key 建议
- **THEN** 系统 SHOULD show member-oriented suggestions such as `GetParamOptions`
- **AND** 系统 MUST NOT include keys that already exist on that member in the suggestions

