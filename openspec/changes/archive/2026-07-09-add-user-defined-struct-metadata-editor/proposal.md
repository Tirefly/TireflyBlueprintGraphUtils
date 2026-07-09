# Proposal: add-user-defined-struct-metadata-editor

## Why

UE5 的用户定义结构体（`UUserDefinedStruct`）编辑器没有提供编辑任意 MetaData 的能力。结构体自身和结构体成员变量都可以通过 C++ 的 `UMETA` 设置任意 MetaData，但蓝图编辑器中没有等价的 UI。

当前 `TireflyBlueprintGraphUtils` 插件已实现蓝图变量、函数、函数参数的 MetaData 编辑能力。结构体是最后一类缺少此能力的常见蓝图数据类型。

结构体 MetaData 的实际需求场景包括：
- 给结构体成员变量设置 `GetParamOptions`、`DisplayName`、`DisplayPriority`、`ForceShow` 等自定义 MetaData。
- 给结构体自身设置 `HiddenByDefault`、`DisableSplitPin`、`DisplayName` 等 MetaData。
- 项目中可能依赖结构体成员 MetaData 来驱动其他插件行为（如 DataTable 行筛选、属性面板条件显示等）。

## What Changes

1. 监听 `UAssetEditorSubsystem::OnAssetOpenedInEditor()`，当用户打开 `UUserDefinedStruct` 资产时，为该结构体编辑器工具栏添加两个独立按钮：
   - **`Struct MetaData`**：弹出窗口编辑结构体自身的 MetaData（使用 `UObject::SetMetaData` / `RemoveMetaData` 读写）。
   - **`Member MetaData`**：弹出窗口按成员变量分组编辑，每个成员变量一个可折叠的 `STireflyMetaDataEditor`。
2. 结构体成员变量的 MetaData 通过 `FStructureEditorUtils::SetMetaData` 和 `FStructureEditorUtils::RemoveMetaData`（或直接操作 `FStructVariableDescription::MetaData`）读写。
3. 修改后调用 `FStructureEditorUtils::OnStructureChanged()` 触发结构体重编译和依赖刷新。
4. 所有修改支持 Undo / Redo（`FScopedTransaction`），修改后标记资产 dirty。
5. 修改 `blueprint-metadata-authoring` spec，解除结构体 MetaData 的限制。
