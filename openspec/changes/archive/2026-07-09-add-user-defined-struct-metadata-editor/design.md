# Design: add-user-defined-struct-metadata-editor

## 技术背景

### 引擎现状（UE 5.7）

#### 结构体成员变量 MetaData

- `FStructVariableDescription` 有 `TMap<FName, FString> MetaData` 字段。
- `FStructureEditorUtils` 公开了：
  - `SetMetaData(UUserDefinedStruct* Struct, FGuid VarGuid, FName Key, const FString& Value)`
  - `GetMetaData(const UUserDefinedStruct* Struct, FGuid VarGuid, FName Key)`
  - **没有公开的 `RemoveMetaData`**，需要直接操作 `FStructVariableDescription::MetaData.Remove(Key)`。
- 成员变量通过 `FGuid VarGuid` 稳定标识（不是名字），重命名不影响 Guid。

#### 结构体自身 MetaData

- `UUserDefinedStruct` 继承自 `UScriptStruct` → `UObject`，有完整的 UObject MetaData 系统。
- 通过 `UObject::SetMetaData(FName Key, const TCHAR* Value)` 和 `UObject::RemoveMetaData(FName Key)` 读写。
- `UObject::GetMetaDataMap()` 可获取只读 `TMap<FName, FString>*`。

#### 结构体编辑器 UI 扩展

- 用户定义结构体编辑器 `IUserDefinedStructureEditor` 继承自 `FAssetEditorToolkit`。
- 内部通过 `RegisterInstancedCustomPropertyLayout(UUserDefinedStruct::StaticClass(), ...)` 注册私有 `FUserDefinedStructureDetails`，由它完整接管 `Structure` 页。
- 全局 `RegisterCustomClassLayout(UUserDefinedStruct)` **不会**生效，因为 instanced layout 优先。
- 正确的扩展点是 **工具栏**：通过 `UAssetEditorSubsystem::OnAssetOpenedInEditor()` 识别打开的结构体编辑器，调用 `FAssetEditorToolkit::AddToolbarExtender()`。

## UI 方案

### 工具栏按钮

监听 `UAssetEditorSubsystem::OnAssetOpenedInEditor()`，当打开的资产是 `UUserDefinedStruct` 时：
1. 将编辑器 toolkit cast 为 `FAssetEditorToolkit` 或 `IAssetEditorInstance`。
2. 调用 `AddToolbarExtender()` 添加工具栏扩展。
3. 在 `Asset` section 后追加两个独立按钮：
   - **`Struct MetaData`**：弹出结构体自身 MetaData 编辑窗口。
   - **`Member MetaData`**：弹出成员变量 MetaData 编辑窗口。
4. 调用 `RegenerateMenusAndToolbars()` 刷新工具栏。

### Struct MetaData 窗口

点击 `Struct MetaData` 按钮后弹出一个 `SWindow`，只包含结构体自身的 MetaData 编辑器：

```
[结构体 MetaData 编辑窗口]
└── STireflyMetaDataEditor (结构体自身 Key/Value)
```

### Member MetaData 窗口

点击 `Member MetaData` 按钮后弹出一个 `SWindow`，按成员变量分组：

```
[成员变量 MetaData 编辑窗口]
├── [成员变量 1: VarName_1 (Type)]
│    └── SExpandableArea → STireflyMetaDataEditor
├── [成员变量 2: VarName_2 (Type)]
│    └── SExpandableArea → STireflyMetaDataEditor
└── ...
```

### 结构体成员变量获取

通过 `FStructureEditorUtils::GetVarDescriptions(UUserDefinedStruct*)` 获取所有成员变量的 `FStructVariableDescription`，其中包含：
- `VarGuid`：稳定标识，用于读写 MetaData。
- `VarName`：显示名。
- `MetaData`：`TMap<FName, FString>`，当前 MetaData。

## 数据读写

### 结构体自身 MetaData

```cpp
// 读取
const TMap<FName, FString>* MetaMap = Struct->GetMetaDataMap();

// 写入
Struct->SetMetaData(Key, *Value);
Struct->Modify();

// 删除
Struct->RemoveMetaData(Key);
```

### 结构体成员变量 MetaData

```cpp
// 读取
FString Value = FStructureEditorUtils::GetMetaData(Struct, VarGuid, Key);

// 写入
FStructureEditorUtils::SetMetaData(Struct, VarGuid, Key, Value);

// 删除（FStructureEditorUtils 没有公开 RemoveMetaData）
TArray<FStructVariableDescription>& VarDescs = ...;
for (FStructVariableDescription& VarDesc : VarDescs)
{
    if (VarDesc.VarGuid == VarGuid)
    {
        VarDesc.MetaData.Remove(Key);
        break;
    }
}
```

### 修改后刷新

结构体 MetaData 修改后需要调用 `FStructureEditorUtils::OnStructureChanged(Struct, EStructureEditorChangeInfo::Other)` 触发重编译和依赖刷新，否则引用该结构体的 DataTable、蓝图等不会立即生效。

## 事务与脏标记

- 所有修改包在 `FScopedTransaction` 中。
- 修改前调用 `Struct->Modify()`。
- 修改后调用 `Struct->MarkPackageDirty()`。
- 修改后触发 `FStructureEditorUtils::OnStructureChanged()`。

## 工具栏扩展的生命周期管理

### 注册

在模块 `StartupModule` 时：
1. 获取 `UAssetEditorSubsystem`。
2. 绑定 `OnAssetOpenedInEditor` 委托。

### 处理资产打开

在回调中：
1. 检查打开的资产是否为 `UUserDefinedStruct`。
2. 将 toolkit 参数 cast 为 `IAssetEditorInstance` / `FAssetEditorToolkit`。
3. 检查是否已添加过 extender（避免重复添加）。
4. 创建 toolbar extender 并添加。

### 反注册

在模块 `ShutdownModule` 时：
1. 解绑 `OnAssetOpenedInEditor` 委托。
2. 无法主动移除已添加到已打开编辑器的 extender（引擎没有 `RemoveToolbarExtender`），但编辑器关闭时 extender 会随 toolkit 一起销毁。

## 模块依赖

`TireflyBlueprintGraphUtils.Build.cs` 需新增：
- `StructUtils`：`FStructureEditorUtils` 所在模块（或确认在 `UnrealEd` 中）。
- `Subsystems`：`UAssetEditorSubsystem`。
- `ToolMenus`：工具栏扩展（如果使用 ToolMenus API 而非旧的 toolbar extender）。

## 常用 Key 建议

### 结构体自身
- `HiddenByDefault`
- `DisableSplitPin`
- `DisplayName`

### 结构体成员变量
- `GetParamOptions`
- `DisplayPriority`
- `ForceShow`
- `NoResetToDefault`

## 风险与边界

### `FStructureEditorUtils::RemoveMetaData` 缺失

`FStructureEditorUtils` 没有公开 `RemoveMetaData`，需要直接操作 `FStructVariableDescription::MetaData`。这需要通过 `FStructureEditorUtils::GetVarDescriptions()` 获取可变引用。实现时需确认该函数返回的是可变引用还是拷贝。

### 结构体重编译开销

`OnStructureChanged` 会触发结构体重编译，可能影响引用该结构体的所有资产。不应在每次 Key/Value 提交时都调用，而应在面板关闭时统一触发一次。

### 工具栏 extender 重复添加

需要在添加前检查是否已添加，维护一个 `TSet<TWeakPtr<IAssetEditorInstance>>` 记录已处理的编辑器。

### UE 版本升级

`FStructureEditorUtils` 是公开 API（在 `UnrealEd` 模块中），稳定性较好。工具栏扩展通过 `FAssetEditorToolkit` 公开接口，风险低。`FUserDefinedStructureDetails` 等私有类型不依赖。
