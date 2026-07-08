# 项目上下文

## 目的

TireflyBlueprintGraphUtils 是 `TireflyGameplayUtils` 仓库中的蓝图图表与蓝图编辑器增强插件，面向 Unreal Engine 5.7 的编辑器工作流扩展。

这套 `openspec/` 工作区用于记录 TireflyBlueprintGraphUtils 插件内的功能提案、设计决策和已落地能力规范。除非特别说明，`openspec/`、`Source/`、`Documents/`、`Resources/` 等路径都以 `Plugins/TireflyBlueprintGraphUtils/` 为根目录。

## 技术栈

- 引擎：Unreal Engine 5.7
- 主语言：C++17，遵循 UE5 C++ 规范与仓库级 `ue5-cpp-style` 约束
- UI：Slate / PropertyEditor / Kismet editor extension APIs
- 构建系统：UnrealBuildTool
- 插件类型：Editor plugin

## 当前插件能力

- `FunctionParamOptions`：为函数参数 Pin 提供基于 MetaData 的选项列表 UI。
- 当前实现依赖 `UK2Node_CallFunction::GetPinMetaData()` 读取参数 Pin Meta，例如 `GetParamOptions`。

## 设计原则

- 优先使用 UE 公开编辑器扩展点，例如 `FBlueprintEditorModule` 的变量 / 函数 customization API。
- 避免依赖 Kismet 私有 UI 类型或通过 Slate 层级遍历侵入编辑器内部布局。
- 公共 UI 组件应与具体 MetaData 存储目标解耦，通过适配器读写 Key/Value。
- 第一版不尝试动态枚举 UE5 引擎中所有可能的 MetaData Key；提供手动输入与常用 Key 建议即可。
- 所有编辑器修改必须支持 Undo / Redo，并正确标记资产或蓝图 dirty。

## 非目标

- 不修改 Unreal Engine 源码。
- 不手改生成文件。
- 不在 Editor 插件能力中引入运行时模块依赖。
- 未经单独提案，不实现 UserDefinedStruct 工具栏面板或蓝图函数参数 Meta sidecar 编译注入。

## 验证策略

- OpenSpec 提案使用 `openspec validate <change-id> --strict --no-interactive` 校验。
- C++ 实现完成后使用当前项目 UE 5.7 `TireflyGameplayUtilsEditor Win64 Development` 编译验证。
- 编辑器 UI 能力需要补充手动验证记录，包括打开蓝图、编辑变量 / 函数 Meta、Undo / Redo、保存重开、编译后保留。
