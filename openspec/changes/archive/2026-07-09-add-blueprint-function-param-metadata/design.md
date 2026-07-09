# Design: add-blueprint-function-param-metadata

## 技术背景

### 引擎现状（UE 5.7）

- `FUserPinInfo`（`K2Node_EditablePinBase.h:30`）只有 `PinName`、`PinType`、`DesiredPinDirection`、`PinDefaultValue` 四个字段，**没有 MetaData 字段，没有 Guid**。
- `UK2Node_CallFunction::GetPinMetaData()`（`K2Node_CallFunction.cpp:3668`）的读取路径：
  1. 先查父类 `UK2Node::GetPinMetaData`（处理 split struct pin）。
  2. 通过 `GetTargetFunction()` 获取 `UFunction`；当 `IsGeneratedClassLayoutReady()` 为 false 时优先查 `SkeletonGeneratedClass`，否则查 `GeneratedClass`。
  3. 调用 `UFunction::FindPropertyByName(PinName)` 找到参数 `FProperty`。
  4. 调用 `FProperty::GetMetaData(Key)` 读取元数据。
- 因此，只要在编译结果的 `UFunction` 参数 `FProperty` 上写入 MetaData，`GetPinMetaData()` 就能读到。

### 引擎扩展点

| 扩展点 | 模块 | 实例粒度 | 编译回调 | 适用职责 |
|---|---|---|---|---|
| `UBlueprintExtension` | Runtime/Engine | 每蓝图实例，序列化进资产 | 基类仅 3 个弱钩子 | **sidecar 持久化** |
| `UBlueprintCompilerExtension` | Editor/Kismet | 全局注册，按 `TSubclassOf<UBlueprint>` 匹配 | `ProcessBlueprintCompiled`，在函数生成后、代码生成前 | **编译期注入** |

`UBlueprintExtension` 基类的编译回调（`HandlePreloadObjectsForCompilation`、`HandleGenerateFunctionGraphs`、`HandleGatherSearchData`）时机不够明确，不适合做注入。`HandleBeginCompilation` / `HandleFinishCompilingClass` 等是 `UAnimBlueprintExtension` / `UWidgetBlueprintExtension` 子类自定义的，基类没有。

`UBlueprintCompilerExtension::ProcessBlueprintCompiled` 收到 `const FKismetCompilerContext&` 和 `const FBlueprintCompiledData&`，调用时机为"图裁剪和函数生成之后、代码生成之前"，此时 UFunction 参数 FProperty 应已创建。

## 数据模型

```cpp
/** 单个参数的 MetaData 条目 */
USTRUCT()
struct FTireflyParamMetaDataEntry
{
    GENERATED_BODY()

    /** 参数名，对应 FUserPinInfo::PinName */
    UPROPERTY()
    FName ParamName;

    /** 该参数的 MetaData Key/Value 集合 */
    UPROPERTY()
    TMap<FName, FString> MetaData;
};

/** 单个函数的参数 MetaData 集合 */
USTRUCT()
struct FTireflyFunctionParamMetaData
{
    GENERATED_BODY()

    /** 函数图的 Guid（UEdGraph::GraphGuid），稳定标识 */
    UPROPERTY()
    FGuid FunctionGraphGuid;

    /** 函数名，用于辅助匹配和显示 */
    UPROPERTY()
    FName FunctionName;

    /** 该函数各参数的 MetaData 列表 */
    UPROPERTY()
    TArray<FTireflyParamMetaDataEntry> ParamMetaData;
};
```

### 持久化容器

```cpp
/**
 * 蓝图函数参数 MetaData 的 sidecar 持久化扩展。
 * 作为 UBlueprint 的子对象自动序列化到资产中。
 */
UCLASS()
class UTireflyBlueprintParamMetaDataExtension : public UBlueprintExtension
{
    GENERATED_BODY()

public:
    /** 获取或创建指定函数图的参数 MetaData 条目 */
    FTireflyFunctionParamMetaData* FindOrAddFunction(const FGuid& FunctionGraphGuid, FName FunctionName);

    /** 查找指定函数图的参数 MetaData 条目（只读） */
    const FTireflyFunctionParamMetaData* FindFunction(const FGuid& FunctionGraphGuid) const;

    /** 删除指定函数图的参数 MetaData 条目 */
    void RemoveFunction(const FGuid& FunctionGraphGuid);

    /** 获取或创建指定参数的 MetaData 条目 */
    FTireflyParamMetaDataEntry* FindOrAddParam(FTireflyFunctionParamMetaData& FunctionEntry, FName ParamName);

    /** 删除指定参数的 MetaData 条目 */
    void RemoveParam(FTireflyFunctionParamMetaData& FunctionEntry, FName ParamName);

    /** 参数重命名时迁移 MetaData */
    void RenameParam(const FGuid& FunctionGraphGuid, FName OldParamName, FName NewParamName);

    /** 清理不存在于当前函数参数列表中的孤儿条目 */
    void PurgeOrphanedParams(const FGuid& FunctionGraphGuid, const TArray<FName>& ValidParamNames);

    /** 清理不存在于当前蓝图中的孤儿函数条目 */
    void PurgeOrphanedFunctions(const TArray<FGuid>& ValidFunctionGraphGuids);

private:
    UPROPERTY()
    TArray<FTireflyFunctionParamMetaData> FunctionParamMetaData;
};
```

### Extension 获取方式

```cpp
/**
 * 获取或创建蓝图的参数 MetaData sidecar extension。
 * 如果蓝图上还没有该 extension，则创建并添加。
 */
UTireflyBlueprintParamMetaDataExtension* GetOrCreateParamMetaDataExtension(UBlueprint* Blueprint);
```

使用 `UBlueprint::GetExtensions()` 查找已有 extension；如果不存在，`NewObject<UTireflyBlueprintParamMetaDataExtension>(Blueprint)` 创建后 `Blueprint->AddExtension(Extension)` 添加。

## 编译期注入

### 方案

注册 `UTireflyParamMetaDataCompilerExtension : public UBlueprintCompilerExtension`，在模块启动时通过 `FBlueprintCompilationManager::RegisterCompilerExtension(UBlueprint::StaticClass(), Extension)` 全局注册。

在 `ProcessBlueprintCompiled` 回调中：

1. 从 `FKismetCompilerContext` 获取正在编译的 `UBlueprint`。
2. 从蓝图的 Extensions 中找到 `UTireflyBlueprintParamMetaDataExtension`。
3. 如果存在 sidecar 数据，遍历蓝图的函数图：
   a. 通过函数图的 `GraphGuid` 匹配 sidecar 条目。
   b. 找到对应的 `UFunction`（在 `SkeletonGeneratedClass` 和 `GeneratedClass` 上分别查找）。
   c. 遍历参数 `FProperty`，按参数名匹配 sidecar 中的 MetaData。
   d. 调用 `FProperty::SetMetaData(Key, *Value)` 写入。

### 注入目标

- `SkeletonGeneratedClass`：编辑器中 `GetPinMetaData` 在 `IsGeneratedClassLayoutReady()` 为 false 时优先查此 class。通过 `CompilationContext.Blueprint->SkeletonGeneratedClass` 访问。
- `GeneratedClass`：运行时和编辑器最终状态都使用此 class。通过 `CompilationContext.NewClass`（即 `CompilationContext.Blueprint->GeneratedClass`）访问。
- 两者都必须注入，避免编辑器查询结果不一致。

### UFunction 查找方式

`ProcessBlueprintCompleted` 回调中，通过以下方式定位目标 UFunction：
1. 遍历 `FBlueprintCompiledData.IntermediateGraphs`，用 `UEdGraph::GraphGuid` 匹配 sidecar 中的 `FunctionGraphGuid`。
2. 从匹配的图中获取函数名（图名即为函数名），或直接使用 sidecar 中存储的 `FunctionName`。
3. 在 `SkeletonGeneratedClass` 和 `GeneratedClass` 上分别调用 `FindFunctionByName(FunctionName)` 获取 `UFunction*`。
4. 遍历 `UFunction` 的参数 `FProperty`（通过 `TFieldIterator<FProperty>(UFunction)`），按参数名匹配 sidecar 中的 `ParamName`。
5. 调用 `FProperty->SetMetaData(Key, *Value)` 写入 MetaData。

### 技术验证结论（已完成）

1. **`FKismetCompilerContext` 访问接口**：`FKismetCompilerContext` 直接暴露 public 成员 `Blueprint`（`UBlueprint*`）和 `NewClass`（`UBlueprintGeneratedClass*`，即 GeneratedClass）。通过 `Blueprint->SkeletonGeneratedClass` 可访问 SkeletonGeneratedClass。`FunctionList` 是 protected，普通 Extension 不可直接访问，但可通过 `Blueprint->GeneratedClass` 用 `FindFunctionByName` 或 `TFieldIterator<UFunction>` 查找 UFunction。
2. **UFunction 参数 FProperty 创建时机**：`ProcessExtensions` 在 STAGE XII（COMPILE CLASS LAYOUT）末尾调用，位于 `CompileClassLayout`（含 `PrecompileFunction` → `CreateParametersForFunction` → `StaticLink`）完成之后、`CompileFunctions`（STAGE XIII，字节码生成）之前。**在 `ProcessBlueprintCompleted` 被调用时，UFunction 的参数链表（`ChildProperties`）已完全填充并链接完毕**，仅局部变量和字节码尚未生成。
3. **`FProperty::SetMetaData` 可用性**：`FField` 在 `WITH_METADATA` 构建中有 `SetMetaData` 方法，Editor 构建中可用。
4. **编译时序风险已消除**：不需要备选方案，`ProcessBlueprintCompleted` 时机完全满足注入需求。
5. **Custom Event 与接口函数**：Custom Event 编译时通过 `CreateFunctionStubForEvent` 转换为 `UK2Node_FunctionEntry`，走标准 `PrecompileFunction` 路径创建 UFunction + 参数 FProperty。接口蓝图以 `Normal` JobType 编译，`ShouldCompileClassLayout()` 返回 true，`ProcessExtensions` 会被调用。两者都生成 UFunction + 参数 FProperty，都能被注入。
6. **Extension 注册类型**：必须注册在 `UBlueprint::StaticClass()` 上（不能注册在 `UBlueprintCore` 上），因为 `ProcessExtensions` 循环条件是 `Iter != UBlueprint::StaticClass()->GetSuperClass()`。注册在 `UBlueprint` 上可匹配所有 `UBlueprint` 子类（普通/接口/动画/Widget 蓝图）。
7. **`FBlueprintCompiledData` 不含 UFunction 列表**：回调中要找 UFunction，需通过 `CompilationContext.Blueprint->GeneratedClass` 查找。`FBlueprintCompiledData.IntermediateGraphs` 只含中间图列表，可用于通过 `GraphGuid` 匹配 sidecar 条目。

## UI 扩展

### 入口

扩展现有的 `TireflyBlueprintFunctionMetaDataDetails`（函数 Details customization），在函数 MetaData 分组下方新增 `Parameters Meta Data` 分组。

### 布局

```
[函数 Details 面板]
  ├── MetaData                    (已有，函数级 MetaData)
  └── Parameters Meta Data        (新增)
       ├── [参数名 1]
       │    └── STireflyMetaDataEditor (该参数的 Key/Value 表)
       ├── [参数名 2]
       │    └── STireflyMetaDataEditor
       └── ... (按参数列表顺序)
```

### 读写流程

1. 从当前选中的 `UK2Node_FunctionEntry`（或 `UK2Node_CustomEvent`）获取函数图。
2. 获取函数图的 `GraphGuid`。
3. 获取蓝图对象的 `UTireflyBlueprintParamMetaDataExtension`。
4. 读取：从 extension 中按 `GraphGuid` + `ParamName` 查找 MetaData。
5. 写入：通过 extension 的方法修改 MetaData，创建 `FScopedTransaction`，调用 `Blueprint->Modify()`。
6. 写入后标记蓝图 modified，并触发编译（使注入生效）。

### 参数列表获取

从 `UK2Node_EditablePinBase::UserDefinedPins` 获取当前函数的参数列表。只显示输入参数（`DesiredPinDirection == EGPD_Input`），输出参数（返回值）也可支持但优先级低。

### 常用 Key 建议

参数级 MetaData 的常用 Key 与变量级类似（`GetParamOptions`、`DisplayName`、`EditCondition` 等），但会额外包含参数专用 Key（如 `NativeConst` 等暂不纳入建议列表，只允许手动输入）。第一阶段建议列表以 `GetParamOptions` 为核心，后续按需扩展。

## 参数重命名与删除处理

### 重命名

`FUserPinInfo` 没有 Guid，参数重命名时 sidecar MetaData 会失联。处理方案：

1. **主动迁移（优先）**：UE 5.7 提供了 `UK2Node::OnUserDefinedPinRenamed()` 委托（`FOnUserDefinedPinRenamed`，参数为 `UK2Node* Node, FName OldName, FName NewName`），在 `UK2Node::RenameUserDefinedPin` 成功后广播。
   - **关键限制**：这是 **per-node** 委托（通过 `FUObjectAnnotationSparse` 存储），不是全局的。必须针对每个函数入口节点（`UK2Node_FunctionEntry` / `UK2Node_CustomEvent`）实例注册。
   - **注册时机**：在函数 Details customization 构建时，为当前选中的函数入口节点注册委托。节点切换时移除旧委托、注册新委托。
   - **委托回调**：在回调中调用 `Extension->RenameParam(FunctionGraphGuid, OldName, NewName)` 迁移 sidecar 数据。
   - **引擎参考示例**：`UMovieSceneEventSectionBase::OnUserDefinedPinRenamed`、`UMovieScene::OnDynamicBindingUserDefinedPinRenamed`、`AnimGraphNode_CallFunction`。
   - **委托生命周期**：`FUObjectAnnotationSparse` 是 transient 的，节点销毁后委托自动清理。但在编辑器运行期间，节点切换需要手动管理委托注册/反注册。
2. **被动清理（兜底）**：在 UI 构建时，将 sidecar 中存在但当前参数列表中不存在的 `ParamName` 标记为孤儿，提供"恢复到新参数"或"删除"选项。如果 per-node 委托注册时机错漏，兜底机制仍能保证数据不丢失。

### 删除

参数删除后，sidecar 中对应的 MetaData 条目成为孤儿。处理方式：
- 在 UI 构建时清理不匹配的条目（`PurgeOrphanedParams`）。
- 或保留孤儿数据，在 UI 中以灰色显示并允许用户手动删除或迁移。

### 函数删除

函数删除后，对应函数图的 `GraphGuid` 不再有效。在 `PurgeOrphanedFunctions` 中清理。

## 事务与脏标记

- 所有 sidecar MetaData 修改（新增、编辑、删除、重命名）必须包在 `FScopedTransaction` 中。
- 修改前调用 `Extension->Modify()` 和 `Blueprint->Modify()`。
- 修改后调用 `FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint)`。
- 修改后触发蓝图编译（`FBlueprintEditorUtils::CompileBlueprint` 或等价流程），使注入生效。
- Undo/Redo 通过 `FScopedTransaction` 自动进入编辑器 Undo 栈。

## 模块依赖

`TireflyBlueprintGraphUtils.Build.cs` 需新增：
- `KismetCompiler`：`UBlueprintCompilerExtension`、`FKismetCompilerContext`、`FBlueprintCompilationManager`。
- `Runtime/Engine` 中 `UBlueprintExtension` 已通过 `Engine` 模块可用（现有依赖 `UnrealEd` 已包含）。

## 风险与边界

### 编译时序

技术验证已确认 `ProcessBlueprintCompiled` 回调时 UFunction 参数 FProperty 已完全创建并链接，注入无时序风险。`ProcessExtensions` 在 STAGE XII 末尾调用，位于 `CompileClassLayout`（含 `PrecompileFunction` → `CreateParametersForFunction` → `StaticLink`）完成之后、`CompileFunctions`（字节码生成）之前。

### Meta 被覆盖

每次蓝图编译都会重新生成 UFunction 和 FProperty，sidecar MetaData 需要每次编译后重新注入。`UBlueprintCompilerExtension` 的回调正好满足这个需求。

### 多版本兼容

`UTireflyBlueprintParamMetaDataExtension` 作为 `UPROPERTY` 子对象序列化到蓝图资产中。旧版本蓝图（没有 extension）打开时不会出错，只是没有参数 MetaData。新版本保存后，旧版本插件打开时 extension 子对象会被忽略（UE 序列化容错）。

### Custom Event 与接口函数

- Custom Event（`UK2Node_CustomEvent`）编译时通过 `CreateFunctionStubForEvent` 转换为 `UK2Node_FunctionEntry`，走标准 `PrecompileFunction` 路径，UFunction + 参数 FProperty 正常生成，可注入。
- 接口蓝图以 `Normal` JobType 编译，`ShouldCompileClassLayout()` 返回 true，`ProcessExtensions` 会被调用。接口函数标记为 InterfaceStub 但 UFunction + 参数 FProperty 仍会创建，可注入。接口蓝图不生成 bytecode，但注入不依赖 bytecode。
- 宏（`UK2Node_Tunnel`）暂不纳入第一版。

### 性能

- sidecar 数据量通常很小（每个函数几个参数，每个参数几个 MetaData）。
- 编译注入开销可忽略（遍历函数列表 + `SetMetaData` 调用）。
- 不需要额外的定时器或轮询。
