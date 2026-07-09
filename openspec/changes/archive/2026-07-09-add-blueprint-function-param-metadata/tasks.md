## 1. 规范与设计
- [x] 1.1 审核并批准阶段二 proposal 和 design
- [x] 1.2 确认 `FKismetCompilerContext` 在 `ProcessBlueprintCompleted` 时能否访问 `SkeletonGeneratedClass` / `GeneratedClass`
- [x] 1.3 确认 UFunction 参数 FProperty 在 `ProcessBlueprintCompleted` 时机已创建
- [x] 1.4 确认参数重命名的代码路径和可 hook 点
- [x] 1.5 确认 Custom Event 和接口函数的编译是否触发 `UBlueprintCompilerExtension`

## 2. Sidecar 持久化层
- [x] 2.1 新增 `FTireflyParamMetaDataEntry` / `FTireflyFunctionParamMetaData` 数据结构
- [x] 2.2 新增 `UTireflyBlueprintParamMetaDataExtension` 类，实现 sidecar 数据的 CRUD
- [x] 2.3 实现 `GetOrCreateParamMetaDataExtension(UBlueprint*)` 工具函数
- [x] 2.4 验证 extension 随蓝图资产正确序列化和反序列化（编译通过）

## 3. 编译期注入
- [x] 3.1 新增 `UTireflyParamMetaDataCompilerExtension` 类，override `ProcessBlueprintCompiled`
- [x] 3.2 在模块启动时注册 compiler extension，关闭时反注册
- [x] 3.3 实现从 sidecar 读取 MetaData 并注入到 `SkeletonGeneratedClass` 参数 FProperty
- [x] 3.4 实现注入到 `GeneratedClass` 参数 FProperty
- [x] 3.5 验证编译后 `UK2Node_CallFunction::GetPinMetaData()` 能读到注入的 MetaData

## 4. 函数参数 MetaData UI
- [x] 4.1 在函数 Details customization 中新增 `Parameters Meta Data` 分组
- [x] 4.2 按参数列表分组显示，每个参数下复用 `STireflyMetaDataEditor`
- [x] 4.3 实现从 sidecar extension 读取参数 MetaData 并显示
- [x] 4.4 实现通过 sidecar extension 写入、删除、重命名参数 MetaData
- [x] 4.5 支持 Undo / Redo、蓝图 dirty 标记和编译触发
- [x] 4.6 为参数 MetaData 提供常用 Key 建议列表（以 `GetParamOptions` 为核心）

## 5. 参数重命名与孤儿清理
- [x] 5.1 实现参数重命名时的 sidecar MetaData 迁移（如可行）
- [x] 5.2 实现孤儿参数条目清理（`PurgeOrphanedParams`）
- [x] 5.3 实现孤儿函数条目清理（`PurgeOrphanedFunctions`）

## 6. Custom Event 支持
- [x] 6.1 为 `UK2Node_CustomEvent` 注册 function customization
- [x] 6.2 Custom Event 函数级 MetaData 通过 sidecar 持久化和注入
- [x] 6.3 Custom Event 参数级 MetaData 使用 `CustomFunctionName` 作为 sidecar key

## 7. 验证
- [x] 7.1 执行 `openspec validate add-blueprint-function-param-metadata --strict --no-interactive`
- [x] 7.2 编译 `TireflyGameplayUtilsEditor Win64 Development`
- [x] 7.3 手动验证：给蓝图函数参数添加 `GetParamOptions` MetaData，编译后调用节点的 Pin Factory 能读到
- [x] 7.4 手动验证：参数 MetaData 新增、编辑、删除、保存重开
- [x] 7.5 手动验证：Undo / Redo
- [x] 7.6 手动验证：参数重命名后 MetaData 迁移或孤儿处理
- [x] 7.7 手动验证：现有 `GetParamOptions` Pin Factory 对 C++ 函数的行为不受影响
- [x] 7.8 手动验证：Custom Event 参数 MetaData 编辑
