# TireflyBlueprintGraphUtils 蓝图 Meta 数据编辑扩展调研

## 背景

目标是在 `TireflyBlueprintGraphUtils` 插件中新增 UE5 蓝图编辑器扩展，使编辑器用户可以增删改以下对象的 Meta 数据：

- 蓝图编辑器中的 `UPROPERTY` 变量。
- 蓝图编辑器中的 `UFUNCTION` 函数。
- 蓝图编辑器中的 `UPARAM` 函数参数。
- 用户定义结构体编辑器中的结构体声明。
- 用户定义结构体编辑器中的结构体成员变量。

UI 方向采用类似 `TMap` 的 Key/Value 编辑方式：MetaData 名称在左侧，MetaData 值在右侧。实现上不要求必须使用真实 `TMap` 属性编辑器，只需要提供相同信息结构和编辑体验。

对于 `UUserDefinedStruct` 相关 MetaData，不应默认尝试追加到结构体编辑器现有 `Structure` 页。该页虽然底层使用 `IDetailsView`，但已经由 Kismet 私有的 instanced customization 完整接管。更合理的入口是在结构体编辑器顶部工具栏区域增加 `MetaData` 按钮，打开统一配置面板，同时编辑结构体自身 MetaData 和结构体成员变量 MetaData。

当前项目引擎版本以 `TireflyGameplayUtils.uproject` 的 `EngineAssociation` 为准，为 UE 5.7。

## 总体结论

整体可行，但不同目标的实现难度和稳定性差异较大。

| 目标 | 可行性 | 推荐程度 | 主要原因 |
| --- | --- | --- | --- |
| 蓝图 `UPROPERTY` 变量 Meta | 可行 | 高 | UE 已提供变量 Details 扩展入口和变量 Meta 读写 API。 |
| 蓝图 `UFUNCTION` 函数 Meta | 可行 | 高 | UE 已提供函数 Details 扩展入口，用户声明函数已有 `FKismetUserDeclaredFunctionMetadata`。 |
| 蓝图 `UPARAM` 参数 Meta | 部分可行 | 中 | 蓝图参数描述结构本身没有 Meta 字段，需要额外持久化并在编译期注入到 `UFunction` 参数 `FProperty`。 |
| 用户定义结构体成员变量 Meta | 可行 | 高 | `FStructVariableDescription` 已有持久化 `MetaData` map，`FStructureEditorUtils` 已公开 Set/Get API；UI 推荐放入结构体编辑器工具栏打开的统一 MetaData 面板。 |
| 用户定义结构体自身 Meta | 可行 | 中 | `UUserDefinedStruct` 本身可设置 UObject Meta；UI 推荐通过 `IUserDefinedStructureEditor` 的资产编辑器工具栏扩展进入。 |

建议分阶段实现：第一阶段先实现公共 MetaData 编辑组件、蓝图变量 Meta、蓝图函数 Meta；第二阶段实现 UserDefinedStruct 工具栏入口和结构体/成员变量统一 MetaData 面板；第三阶段再处理函数参数 Meta 的 sidecar 持久化和编译期注入。

## 当前插件状态

`TireflyBlueprintGraphUtils` 当前是 Editor 插件模块：

- 插件模块类型：`Editor`。
- 加载阶段：`PreDefault`。
- 已依赖 `BlueprintGraph`、`UnrealEd`、`GraphEditor`、`Slate`、`SlateCore` 等编辑器模块。
- 现有能力集中在 `FunctionParamOptions`，通过自定义 Pin Factory 读取函数参数 Pin 的 `GetParamOptions` Meta。

当前实现中，`FTireflyParamOptionPinFactory` 通过类似下面的路径读取 Meta：

```cpp
CallFunctionNode->GetPinMetaData(InPin->PinName, FName("GetParamOptions"));
```

这说明如果后续要让蓝图函数参数支持自定义 `GetParamOptions`，最终必须让 `UK2Node_CallFunction::GetPinMetaData()` 能读到该参数 Meta。

## UE 5.7 关键依据

### 蓝图变量 Meta

蓝图变量由 `FBPVariableDescription` 描述，其中包含：

```cpp
TArray<FBPVariableMetaDataEntry> MetaDataArray;
```

并提供：

- `SetMetaData(FName Key, FString Value)`
- `GetMetaData(FName Key) const`
- `RemoveMetaData(FName Key)`
- `HasMetaData(FName Key) const`

编辑器层也已有工具函数：

- `FBlueprintEditorUtils::SetBlueprintVariableMetaData()`
- `FBlueprintEditorUtils::GetBlueprintVariableMetaData()`
- `FBlueprintEditorUtils::RemoveBlueprintVariableMetaData()`

蓝图编辑器提供外部变量 Details customization 扩展点：

- `FBlueprintEditorModule::RegisterVariableCustomization()`
- `FBlueprintEditorModule::CustomizeVariable()`

内置变量详情页末尾会调用外部 customization，因此插件可以在现有变量细节面板中追加 `Meta Data` 分组。

### 蓝图函数 Meta

用户声明函数的 Meta 由 `FKismetUserDeclaredFunctionMetadata` 持久化，其中包含私有 `MetaDataMap`，并提供：

- `SetMetaData(FName Key, const FStringView Value)`
- `RemoveMetaData(FName Key)`
- `HasMetaData(FName Key) const`
- `GetMetaDataMap() const`

蓝图编辑器提供外部函数 Details customization 扩展点：

- `FBlueprintEditorModule::RegisterFunctionCustomization()`
- `FBlueprintEditorModule::CustomizeFunction()`

内置函数详情页末尾会调用外部 customization，因此插件可以给函数详情面板追加任意 Meta 编辑 UI。

### 蓝图函数参数 Meta

蓝图函数参数由 `UK2Node_EditablePinBase::UserDefinedPins` 持久化，元素类型为 `FUserPinInfo`。该结构只有：

```cpp
FName PinName;
FEdGraphPinType PinType;
TEnumAsByte<EEdGraphPinDirection> DesiredPinDirection;
FString PinDefaultValue;
```

没有 Meta 数据字段。

`UK2Node_CallFunction::GetPinMetaData()` 的参数 Meta 查询逻辑大致为：

1. 先查节点或拆分结构体 Pin 相关 Meta。
2. 再通过目标 `UFunction` 找到同名参数 `FProperty`，读取该 `FProperty` 的 Meta。
3. 最后检查少量函数级特殊 Meta。

因此，如果希望蓝图函数参数表现得像 C++ 的 `UPARAM(meta = (...))`，最终必须在蓝图编译结果的 `UFunction` 参数 `FProperty` 上写入对应 Meta。

这不是单纯 Details UI 能解决的问题，需要额外持久化和编译期同步。

### 用户定义结构体成员变量 Meta

用户定义结构体成员变量由 `FStructVariableDescription` 描述，其中已有：

```cpp
TMap<FName, FString> MetaData;
```

并且 `FStructureEditorUtils` 公开了：

- `SetMetaData(UUserDefinedStruct* Struct, FGuid VarGuid, FName Key, const FString& Value)`
- `GetMetaData(const UUserDefinedStruct* Struct, FGuid VarGuid, FName Key)`

UE 内置结构体编辑器已经使用这些 API 编辑固定 Meta，例如 `ClampMin`、`ClampMax`、`UIMin`、`UIMax`。扩展为任意 Key/Value 编辑是自然延伸。

需要注意的是，UE 内置结构体成员列表由 `FUserDefinedStructureDetails`、`FUserDefinedStructureLayout` 和 `FUserDefinedStructureFieldLayout` 等 Kismet 私有类型构建。外部插件不应依赖这些私有类来把任意 MetaData 表格嵌入每个成员行中。更稳妥的 UI 是在结构体编辑器工具栏中提供 `MetaData` 入口，打开独立面板后按成员变量分组编辑。

### 用户定义结构体自身 Meta

`UUserDefinedStruct` 继承自 `UScriptStruct` / `UObject`，技术上可以通过 UObject Meta API 读写结构体自身 Meta。

主要问题是 UI 接入：UE5 的用户定义结构体编辑器不是常规“对象 + Details 面板”的可扩展布局。源码中 `FUserDefinedStructureEditor::SpawnStructureTab()` 会创建 `IDetailsView`，随后通过 `RegisterInstancedCustomPropertyLayout(UUserDefinedStruct::StaticClass(), ...)` 注册私有 `FUserDefinedStructureDetails`，由它完整生成 `Structure` 页内容。

PropertyEditor 查询 detail customization 时会先查当前 `IDetailsView` 的 instanced layout map，只有没有 instanced customization 时才查全局 `RegisterCustomClassLayout()`。因此插件全局注册 `UUserDefinedStruct` class layout 不能自然追加到该结构体编辑器中，反而存在覆盖或失效风险。

更合适的公开接入点是资产编辑器工具栏。`IUserDefinedStructureEditor` 继承自 `FAssetEditorToolkit`，可以通过 `UAssetEditorSubsystem::OnAssetOpenedInEditor()` 识别打开的 `UUserDefinedStruct` 编辑器，转换为 `FAssetEditorToolkit` 后调用 `AddToolbarExtender()`，再 `RegenerateMenusAndToolbars()`。

## 推荐实现方案

### 公共 UI：类 TMap 的 Meta 数据编辑组件

建议先实现一个可复用 Slate 组件，用于编辑 `TMap<FName, FString>` 风格的 Meta 数据。这里的“类 TMap”指 UI 和交互模型，不强制底层数据必须是 `TMap`。

基础布局：

- 左侧列：MetaData 名称，保存为 `FName` 或可转换为 `FName` 的字符串。
- 右侧列：MetaData 值，保存为 `FString`。
- 行尾操作：删除按钮。
- 表格顶部或底部：新增按钮。
- 可选搜索框：按 Key 或 Value 过滤。

建议组件职责：

- 展示 Key / Value 列表。
- 新增 Meta 项。
- 删除 Meta 项。
- 编辑 Key。
- 编辑 Value。
- 检查空 Key、重复 Key、非法 Key。
- 支持空字符串 Value，因为部分 UE Meta 使用空字符串表示存在性标记。
- 支持事务：每次提交修改前创建 `FScopedTransaction`。
- 修改后刷新 Details，并标记资产或蓝图 dirty。

不建议第一版动态扫描并列出 UE5 引擎中所有可能的 MetaData Key。原因是 UE 的 MetaData Key 分散在多个模块、常量和字符串字面量中，动态完整枚举成本高，也很难保证准确。更稳妥的方案是：

- 允许用户手动输入任意 Key。
- 内置一个常用 Key 建议列表，例如 `DisplayName`、`ToolTip`、`ClampMin`、`ClampMax`、`UIMin`、`UIMax`、`GetOptions`、`GetParamOptions` 等。
- 通过 ComboButton 或自动补全提供建议，但不限制用户输入。
- 后续如有需要，再维护插件自己的 MetaData Key 注册表，而不是试图动态读取整个引擎的所有 Key。

建议命名方向：

- `STireflyMetaDataEditor`
- `FTireflyMetaDataEntry`
- `ITireflyMetaDataEditableTarget`

其中 `ITireflyMetaDataEditableTarget` 可抽象读写目标，避免变量、函数、结构体成员各写一套表格 UI。

### 蓝图变量 Meta 实现

实现步骤：

1. 在模块启动时加载 `Kismet` 模块。
2. 调用 `FBlueprintEditorModule::RegisterVariableCustomization()`。
3. 对常见 `FProperty` 类型注册 customization，或按 `FProperty::StaticClass()` / 字段类继承查询机制覆盖所有变量类型。
4. 在 customization 中通过当前选中变量名、蓝图对象和本地变量作用域定位变量。
5. 读取时调用 `FBlueprintEditorUtils::GetBlueprintVariableMetaData()` 或直接读取 `FBPVariableDescription`。
6. 写入时调用 `SetBlueprintVariableMetaData()`。
7. 删除时调用 `RemoveBlueprintVariableMetaData()`。
8. 修改后调用 `FBlueprintEditorUtils::MarkBlueprintAsModified()`。

注意事项：

- 需要区分普通成员变量和局部变量。
- 需要避免覆盖 UE 已有专用 UI 管理的 Meta 时造成困惑，例如 `Tooltip`、`Category`、`ClampMin`、`ClampMax`、`UIMin`、`UIMax`、`ExposeOnSpawn`、`Private`、`FieldNotify` 等。
- 可提供“显示内置 Meta”开关，默认显示所有 Meta，但对内置 Meta 标注为 Engine Managed。

### 蓝图函数 Meta 实现

实现步骤：

1. 在模块启动时调用 `FBlueprintEditorModule::RegisterFunctionCustomization()`。
2. 覆盖 `UK2Node_FunctionEntry`，必要时覆盖 `UK2Node_CustomEvent`、`UK2Node_Tunnel`。
3. 在 customization 中定位当前 `UK2Node_EditablePinBase` 或当前函数图。
4. 获取 `FKismetUserDeclaredFunctionMetadata`。
5. 使用 `GetMetaDataMap()` 读取已有 Meta。
6. 使用 `SetMetaData()`、`RemoveMetaData()` 修改。
7. 修改前创建事务，修改后标记图和蓝图 modified。

注意事项：

- `FKismetUserDeclaredFunctionMetadata` 已有强类型字段，如 `ToolTip`、`Category`、`Keywords`、`CompactNodeTitle`、`DeprecationMessage` 等。这些字段会编译成常见函数 Meta。
- 任意 Meta 应写入 `MetaDataMap`，不要把已有强类型字段重复维护两份。
- 对 Custom Event 的处理路径和普通 Function Entry 不完全相同，需要单独验证。

### 蓝图函数参数 Meta 实现

推荐方案是插件 sidecar 持久化加编译期注入。

#### 持久化方案

不要直接修改 `FUserPinInfo`，因为这需要改引擎或 fork `BlueprintGraph`，版本升级成本高。

建议把参数 Meta 存在蓝图或节点对象的插件专用持久化数据里，候选方式：

- 在函数 Entry 节点上使用 UObject Meta 或自定义 UObject 子对象存储。
- 在蓝图资产上存储插件专用扩展对象，按 Function Graph Guid + Pin Guid/Name 映射。
- 如果没有稳定 Pin Guid，则以 Function Guid / Graph Guid + 参数名为主键，同时处理重命名迁移。

推荐数据模型：

```cpp
struct FTireflyFunctionParamMetaData
{
	FGuid FunctionGraphGuid;
	FName FunctionName;
	FName ParamName;
	TMap<FName, FString> MetaData;
};
```

如果能获得更稳定的参数标识，应优先使用稳定 Guid；仅使用参数名会在重命名时需要迁移。

#### UI 方案

函数详情面板中，在每个输入/输出参数行的展开内容里追加 `Meta Data` 表格，或新增一个 `Parameters Meta Data` 分类，按参数分组显示。

优先建议第二种，因为不需要侵入 UE 内置 `FBlueprintArgumentLayout` 的私有实现。

#### 编译注入方案

注册 `UBlueprintCompilerExtension`，在蓝图编译完成或合适的编译阶段：

1. 找到生成类 / Skeleton 类中的目标 `UFunction`。
2. 遍历参数 `FProperty`。
3. 根据函数名和参数名查 sidecar Meta。
4. 调用 `FProperty::SetMetaData()` 写入参数 Meta。
5. 确保 SkeletonGeneratedClass 和 GeneratedClass 都同步，避免编辑器节点和运行时查询结果不一致。

注意事项：

- 需要验证 `UK2Node_CallFunction::GetPinMetaData()` 在编辑器中拿到的是最新 Skeleton 函数还是 Generated 函数。
- 需要处理函数参数重命名、删除、复制、粘贴、Undo/Redo。
- 需要处理接口蓝图、宏、事件、自定义事件的差异。
- 需要测试现有 `GetParamOptions` Pin Factory 是否能读到注入后的参数 Meta。

### 用户定义结构体 Meta 工具栏面板实现

实现步骤：

1. 模块启动时监听 `UAssetEditorSubsystem::OnAssetOpenedInEditor()`。
2. 当打开资产是 `UUserDefinedStruct` 且编辑器实例是 `IUserDefinedStructureEditor` / `FAssetEditorToolkit` 时，为该 toolkit 添加 toolbar extender。
3. toolbar extender 在 `Asset` section 后追加 `MetaData` 按钮。
4. 点击按钮后打开下拉面板、模态窗口或独立 Slate 面板。
5. 面板上方编辑 `UUserDefinedStruct` 自身 MetaData。
6. 面板下方列出结构体成员变量，选中成员后编辑对应 `FStructVariableDescription::MetaData`。
7. 修改结构体自身 Meta 时调用 `Struct->Modify()`、`SetMetaData()` / `RemoveMetaData()`，并标记 package dirty。
8. 修改成员变量 Meta 时优先调用 `FStructureEditorUtils::SetMetaData()`；删除时修改 `FStructVariableDescription::MetaData.Remove(Key)`。
9. 修改后调用 `FStructureEditorUtils::OnStructureChanged()` 或等价流程，确保重编译结构体并刷新依赖。

注意事项：

- 不建议第一版通过全局 `RegisterCustomClassLayout(UUserDefinedStruct)` 尝试追加 UI，因为结构体编辑器使用 instanced customization，会优先于全局 customization。
- 不建议第一版用 Slate widget 遍历方式找到内部 `IDetailsView` 或成员行再插入控件，这会依赖私有 UI 结构，UE 升级风险高。
- `FStructureEditorUtils` 没有公开 `RemoveMetaData()`，需要谨慎封装删除逻辑。
- 对 `ClampMin`、`ClampMax`、`UIMin`、`UIMax` 这类已有内置 UI 的 Meta，建议标注为 Engine Managed 或提供过滤。
- 结构体字段的变更会影响 DataTable、默认值实例和引用该结构体的蓝图，必须走 UE 的结构体变更通知流程。

备选方案：

- 如果 toolbar extender 的排序无法满足体验要求，可改用 `AddToolbarWidget()` 放到工具栏右侧，但这会离编译/保存等主操作更远。
- 如果面板内容过多，可从按钮打开独立 Tab 或浮动窗口。
- 不推荐覆盖或复制 `FUserDefinedStructureDetails` 私有实现。

## 模块依赖建议

`TireflyBlueprintGraphUtils.Build.cs` 后续可能需要补充以下依赖：

- `Kismet`
- `KismetCompiler`
- `PropertyEditor`
- `EditorStyle` 或 `AppFramework`，视 UI 使用情况而定。
- `ToolMenus`，如果使用 tool menu 扩展或菜单入口。
- `StructUtils`，如果直接包含用户定义结构体相关类型。

已有依赖 `BlueprintGraph`、`UnrealEd`、`GraphEditor`、`Slate`、`SlateCore` 可以继续保留。

## 风险与边界

### Meta Key 合法性

Meta Key 应使用 `FName`，建议禁止空 Key，禁止重复 Key。是否允许包含空格和特殊字符需要明确规则。

建议默认只允许：

```text
[A-Za-z_][A-Za-z0-9_]*
```

如果要兼容 UE 已有 Meta 名称，可放宽但要给出警告。

### 内置 Meta 冲突

很多 Meta 已由 UE 内置 UI 管理。任意编辑能力会带来误操作风险。

建议提供以下策略：

- 默认显示所有 Meta。
- 对已知内置 Meta 显示标记。
- 删除内置 Meta 时弹确认。
- 可配置隐藏内置 Meta。

### 编译与刷新

Meta 修改后是否需要立即编译取决于目标：

- 变量和函数 Meta：通常标记蓝图修改即可，某些 Meta 需要刷新节点或重新编译才能影响 CallFunction 节点。
- 参数 Meta：必须至少同步到 Skeleton/Generated 函数参数，否则 `GetPinMetaData()` 不一定可见。
- 结构体成员 Meta：需要走结构体变更通知，否则引用方可能不刷新。

### Undo/Redo

所有修改必须包在 `FScopedTransaction` 中，并调用目标对象 `Modify()`。

需要重点验证：

- 新增 Meta 后 Undo。
- 删除 Meta 后 Undo。
- 改 Key 后 Undo。
- 改 Value 后 Undo。
- 参数重命名后 Undo。
- 结构体成员重命名后 Undo。

### UE 版本升级

蓝图变量和函数扩展点是公开 API，稳定性较好。

用户定义结构体编辑器 `Structure` 页的大量逻辑在 Kismet 私有 `.cpp` 中，UE 升级风险较高。应避免复制私有实现、覆盖内置 instanced customization、或通过 Slate 层级遍历侵入内部成员行。

## 推荐阶段拆分

### 阶段 1：蓝图 Details 能力

目标：实现稳定、可立即使用的蓝图变量和函数 Meta 编辑能力。

范围：

- 蓝图变量 Meta 编辑。
- 蓝图函数 Meta 编辑。
- 公共 Meta 数据表格组件。

验收点：

- 可以新增、编辑、删除任意 Key/Value。
- 修改会进入 Undo/Redo。
- 关闭并重新打开资产后 Meta 保留。
- 编译蓝图后 Meta 保留。
- 现有 `GetParamOptions` 功能不受影响。

### 阶段 2：结构体工具栏面板

目标：通过结构体编辑器工具栏入口统一编辑结构体自身和结构体成员变量 Meta。

范围：

- 监听 `UAssetEditorSubsystem::OnAssetOpenedInEditor()`。
- 为 `IUserDefinedStructureEditor` 添加 toolbar extender。
- 增加 `MetaData` 按钮。
- 面板中编辑 `UUserDefinedStruct` 自身 Meta。
- 面板中按成员变量编辑 `FStructVariableDescription::MetaData`。

验收点：

- 结构体自身 Meta 可新增、编辑、删除。
- 结构体成员变量 Meta 可新增、编辑、删除。
- 保存、重开、编译结构体后 Meta 保留。
- 不破坏原结构体编辑器的成员变量编辑、默认值编辑、编译、保存和拖拽排序。

### 阶段 3：函数参数 Meta

目标：支持蓝图函数参数上的 `UPARAM` 风格 Meta，服务 `GetParamOptions` 等参数 Pin 扩展。

范围：

- 参数 Meta sidecar 存储。
- 函数详情面板中的参数 Meta UI。
- `UBlueprintCompilerExtension` 编译期注入。
- Skeleton/Generated 函数同步验证。

验收点：

- 给蓝图函数参数添加 `GetParamOptions` 后，调用该函数的节点 Pin Factory 能读到 Meta。
- 参数重命名后 Meta 可以迁移或给出明确处理。
- 参数删除后 Meta 被清理或标记为孤儿项。
- 编译、重开编辑器后仍可用。

## 最终建议

不要一次性把五类目标都作为同一批实现完成。蓝图变量和蓝图函数有公开 Details 扩展点，应该先做。结构体自身和结构体成员变量的数据层都可行，但 UI 应走结构体编辑器工具栏统一入口，作为第二阶段单独验证。函数参数 Meta 涉及 sidecar 持久化和编译期注入，应作为第三阶段处理。

如果业务目标主要是让现有 `GetParamOptions` 支持蓝图函数参数，那么可以把第三阶段的函数参数 Meta 提前到第二阶段执行，并把结构体工具栏面板后移。因为现有 Pin Factory 依赖 `UK2Node_CallFunction::GetPinMetaData()`，只有参数 Meta 最终进入 `UFunction` 参数 `FProperty`，调用节点才能稳定读取。
