## 背景

蓝图变量和蓝图函数在 UE 5.7 中已有公开 Details customization 扩展点：

- `FBlueprintEditorModule::RegisterVariableCustomization()`
- `FBlueprintEditorModule::RegisterFunctionCustomization()`

内置变量和函数详情页会在构建完原生 UI 后调用外部 customization，因此插件可以追加自己的 `MetaData` 分组，而不需要覆盖 UE 内置详情布局。

## 目标

- 提供可复用的 Key / Value MetaData 编辑控件。
- 在蓝图变量细节面板中编辑变量 MetaData。
- 在蓝图函数细节面板中编辑函数 MetaData。
- 支持事务、Undo / Redo、蓝图 dirty 标记和 Details 刷新。

## 非目标

- 不支持函数参数 MetaData。
- 不支持 UserDefinedStruct MetaData。
- 不动态枚举 UE5 引擎所有 MetaData Key。
- 不依赖 Kismet 私有 detail customization 类型。

## 核心设计

### 1. MetaData 编辑控件

新增一个 Slate 控件，暂名 `STireflyMetaDataEditor`。

控件展示一个类 `TMap` 表格：

- 左列：MetaData Key。
- 右列：MetaData Value。
- 行尾：删除按钮。
- 顶部或底部：新增按钮。
- 可选：常用 Key 建议 ComboButton。

控件不直接持有蓝图变量或函数逻辑，而是通过目标适配器读写：

```cpp
class ITireflyMetaDataEditableTarget
{
public:
	virtual TMap<FName, FString> GetMetaDataMap() const = 0;
	virtual bool CanEditMetaData(FName Key) const = 0;
	virtual void SetMetaData(FName Key, const FString& Value) = 0;
	virtual void RemoveMetaData(FName Key) = 0;
	virtual void RenameMetaData(FName OldKey, FName NewKey) = 0;
};
```

该接口为设计草案，具体实现时可根据 Slate 生命周期改为委托集合，避免不必要的继承结构。

### 2. Key 规则

- Key 不得为空。
- Key 不得重复。
- Key 存储为 `FName`。
- Value 存储为 `FString`，允许空字符串。
- 第一版允许手动输入任意 Key，但对明显无效输入给出错误提示。

第一版常用 Key 建议列表按目标类型区分，并且下拉列表必须过滤当前目标已经存在的 Key。蓝图已有显式 UI 的 `DisplayName`、`Category` 等 Key 不进入建议列表。

变量建议 Key：

- `GetOptions`
- `EditCondition`
- `EditConditionHides`
- `AllowedClasses`
- `ExactClass`
- `MustImplement`
- `MetaClass`

函数建议 Key：

- `WorldContext`
- `DefaultToSelf`
- `HidePin`
- `AdvancedDisplay`
- `AutoCreateRefTerm`
- `DeterminesOutputType`
- `DynamicOutputParam`
- `ExpandEnumAsExecs`
- `DevelopmentOnly`

### 3. 蓝图变量适配器

蓝图变量 MetaData 持久化在 `FBPVariableDescription::MetaDataArray`，写入优先使用公开工具函数：

- `FBlueprintEditorUtils::SetBlueprintVariableMetaData()`
- `FBlueprintEditorUtils::GetBlueprintVariableMetaData()`
- `FBlueprintEditorUtils::RemoveBlueprintVariableMetaData()`

变量 Details customization 通过 `FBlueprintEditorModule::RegisterVariableCustomization()` 注册。customization 通过 `IDetailLayoutBuilder::GetSelectedObjects()` 与当前 `IBlueprintEditor` 定位蓝图和变量名。

修改流程：

1. 创建 `FScopedTransaction`。
2. 调用 `Blueprint->Modify()`。
3. 写入或删除 MetaData。
4. 调用 `FBlueprintEditorUtils::MarkBlueprintAsModified()`。
5. 请求 Details 刷新。

### 4. 蓝图函数适配器

用户声明函数 MetaData 使用 `FKismetUserDeclaredFunctionMetadata`。函数 Details customization 通过 `FBlueprintEditorModule::RegisterFunctionCustomization()` 注册。

定位策略：

- 优先从当前选中 `UEdGraph` 调用 `FBlueprintEditorUtils::GetGraphFunctionMetaData()`。
- 对 `UK2Node_FunctionEntry` 选择场景，回溯其所属图后读取 graph function metadata。
- 对 Custom Event 是否纳入第一版，在实现前需确认选中对象和公开 API 是否足够稳定；如果不稳定，第一版只覆盖普通用户函数。

修改流程：

1. 创建 `FScopedTransaction`。
2. 调用 `FBlueprintEditorUtils::ModifyFunctionMetaData(Graph)`。
3. 对 `FKismetUserDeclaredFunctionMetadata` 调用 `SetMetaData()` 或 `RemoveMetaData()`。
4. 标记蓝图 modified。
5. 请求 Details 刷新。

### 5. 内置 MetaData 冲突处理

第一版不禁止编辑内置 MetaData，但 UI 必须标记常见 Engine-managed Key，例如：

- `Category`
- `ToolTip`
- `DisplayName`
- `ClampMin`
- `ClampMax`
- `UIMin`
- `UIMax`

删除 Engine-managed Key 时应提供确认或至少明确提示。

## 风险 / 取舍

- 全量枚举 UE MetaData Key 不现实，先做建议列表更稳。
- 变量和函数现有详情页已有专用字段，通用编辑器可能造成双入口，需要 UI 标识降低误操作风险。
- 如果函数 Custom Event 的公开定位路径不稳定，应延期覆盖，不应为了第一版引入私有依赖。
- 常用 Key 建议列表不是完整 MetaData 字典；用户仍可手动输入建议列表之外的有效 Key。

## 验证计划

- 打开蓝图，选中成员变量，新增 / 修改 / 删除 MetaData。
- 保存并重开蓝图，确认变量 MetaData 保留。
- 对变量 MetaData 执行 Undo / Redo。
- 打开蓝图，选中用户函数，新增 / 修改 / 删除 MetaData。
- 保存并重开蓝图，确认函数 MetaData 保留。
- 编译蓝图后确认 MetaData 保留。
- 确认现有 `FunctionParamOptions` Pin Factory 行为不受影响。
