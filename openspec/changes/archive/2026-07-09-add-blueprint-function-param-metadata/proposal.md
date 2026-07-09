# Proposal: add-blueprint-function-param-metadata

## Why

现有 `FTireflyParamOptionPinFactory` 通过 `UK2Node_CallFunction::GetPinMetaData()` 读取函数参数 MetaData（如 `GetParamOptions`），但只有 C++ `UFUNCTION` 的参数能通过 `UPARAM(meta=(...))` 设置 MetaData。蓝图函数参数由 `FUserPinInfo` 描述，该结构没有 MetaData 字段，蓝图编辑器也没有提供参数 MetaData 的编辑 UI。

因此，蓝图开发者无法为蓝图函数参数设置 `GetParamOptions` 等参数级 MetaData，导致 Pin Factory 的选项列表功能无法用于蓝图函数参数。这是本插件 `FunctionParamOptions` 能力的关键缺口。

需要通过 sidecar 持久化 + 编译期注入的方式，让蓝图函数参数也能拥有 MetaData，并在函数 Details 面板提供编辑入口。

## What Changes

1. 新增 `UTireflyBlueprintParamMetaDataExtension`（`UBlueprintExtension` 子类），在蓝图资产上以子对象形式持久化函数参数 MetaData sidecar 数据，按函数图 Guid + 参数名映射。
2. 新增 `UTireflyParamMetaDataCompilerExtension`（`UBlueprintCompilerExtension` 子类），在蓝图编译期将 sidecar MetaData 注入到 `SkeletonGeneratedClass` 和 `GeneratedClass` 的 `UFunction` 参数 `FProperty` 上，使 `UK2Node_CallFunction::GetPinMetaData()` 能读到。
3. 扩展函数 Details customization，新增 `Parameters Meta Data` 分组，按参数分组编辑，复用现有 `STireflyMetaDataEditor` 控件。
4. 处理参数重命名时的 sidecar MetaData 迁移。
5. 修改 `blueprint-metadata-authoring` spec，解除参数 MetaData 的限制（结构体 MetaData 仍保留为 future proposal）。
