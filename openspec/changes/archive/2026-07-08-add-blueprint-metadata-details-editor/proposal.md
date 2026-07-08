# 变更：新增蓝图变量与函数 MetaData 细节面板编辑能力

## 背景

TireflyBlueprintGraphUtils 当前已经通过 `FunctionParamOptions` 读取函数参数 Pin MetaData，为蓝图图表提供更便捷的参数选项体验。但 UE 蓝图编辑器默认只暴露少量固定 MetaData 编辑项，开发者无法在蓝图变量和蓝图函数的细节面板中以通用 Key/Value 方式增删改 MetaData。

本提案是蓝图 MetaData 编辑扩展的第一阶段，仅覆盖：公共 MetaData 表格、蓝图 `UPROPERTY` 变量 MetaData、蓝图 `UFUNCTION` 函数 MetaData。

## 变更内容

- 新增一个可复用的类 `TMap` MetaData 编辑 UI，用 Key / Value 双列编辑 `FName -> FString` 风格数据。
- 为蓝图变量 Details 增加 `MetaData` 分组，允许新增、编辑、删除变量 MetaData。
- 为蓝图函数 Details 增加 `MetaData` 分组，允许新增、编辑、删除用户声明函数 MetaData。
- 通过常用 Key 建议列表辅助输入，但不要求动态扫描 UE5 引擎所有 MetaData Key。
- 所有修改必须走事务，支持 Undo / Redo，并标记蓝图 dirty。

## 不包含内容

- 不实现蓝图函数参数 `UPARAM` MetaData 编辑。
- 不实现 UserDefinedStruct 自身或成员变量 MetaData 编辑。
- 不实现 sidecar 持久化或编译期注入。
- 不修改 Unreal Engine 源码。

## 影响范围

- 受影响规范：新增 `blueprint-metadata-authoring`
- 受影响代码：
  - `Source/TireflyBlueprintGraphUtils/TireflyBlueprintGraphUtils.Build.cs`
  - `Source/TireflyBlueprintGraphUtils/TireflyBlueprintGraphUtilsModule.*`
  - `Source/TireflyBlueprintGraphUtils/Public/MetaDataEditor/*`
  - `Source/TireflyBlueprintGraphUtils/Private/MetaDataEditor/*`

## 预期收益

- 蓝图作者可以直接在变量和函数细节面板中配置自定义 MetaData。
- `GetOptions`、`GetParamOptions` 等插件或引擎扩展常用 Key 可通过统一 UI 配置。
- 为后续函数参数 MetaData 和 UserDefinedStruct MetaData 编辑能力复用同一套 UI 基础设施。

## 风险与代价

- 蓝图变量和函数已有部分内置 MetaData 专用 UI，通用编辑器可能造成重复入口或误删内置 MetaData。
- 蓝图函数 MetaData 的强类型字段与 `MetaDataMap` 需要边界清晰，不能重复维护同一语义。
- 外部 Details customization 只能依赖公开 API 与当前选择对象定位目标，不能复用 Kismet 私有 `FBlueprintVarActionDetails` / `FBlueprintGraphActionDetails` 内部状态。

## 待审核重点

- 常用 MetaData Key 建议列表第一版包含哪些 Key。
- 是否默认展示 Engine-managed MetaData，还是默认隐藏并提供开关。
- 蓝图函数范围是否只覆盖普通函数，还是同时覆盖 Custom Event 的用户声明 MetaData。
