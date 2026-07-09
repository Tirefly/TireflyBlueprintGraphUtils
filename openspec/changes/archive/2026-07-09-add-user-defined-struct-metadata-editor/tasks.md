## 1. 规范与设计
- [x] 1.1 审核并批准 proposal 和 design
- [x] 1.2 确认 `FStructureEditorUtils::GetVarDesc` 返回引用还是拷贝
- [x] 1.3 确认 `FStructureEditorUtils::OnStructureChanged` 签名和用法
- [x] 1.4 确认 `UAssetEditorSubsystem::OnAssetOpenedInEditor` 回调签名
- [x] 1.5 确认 `FAssetEditorToolkit::AddToolbarExtender` 可用性

## 2. 工具栏扩展
- [x] 2.1 在模块启动时监听 `UAssetEditorSubsystem::OnAssetOpenedInEditor()`
- [x] 2.2 识别 `UUserDefinedStruct` 资产并获取 `FAssetEditorToolkit`
- [x] 2.3 创建 toolbar extender，添加 `Struct MetaData` 和 `Member MetaData` 两个独立按钮
- [x] 2.4 避免重复添加 extender
- [x] 2.5 模块关闭时解绑委托

## 3. MetaData 编辑窗口
- [x] 3.1 新增结构体自身 MetaData 编辑窗口（`SWindow` + `STireflyMetaDataEditor`）
- [x] 3.2 新增成员变量 MetaData 编辑窗口（`SWindow`，按成员分组，每个成员一个 `SExpandableArea` + `STireflyMetaDataEditor`）
- [x] 3.3 常用 Key 建议列表（区分结构体级和成员级）

## 4. 结构体自身 MetaData 读写
- [x] 4.1 通过 `FMetaData::GetMapForObject` 读取
- [x] 4.2 通过 `UObject::SetMetaData` 写入
- [x] 4.3 通过 `UObject::RemoveMetaData` 删除

## 5. 结构体成员变量 MetaData 读写
- [x] 5.1 通过 `FStructureEditorUtils::GetVarDescByGuid` / `FStructVariableDescription::MetaData` 读取
- [x] 5.2 通过 `FStructureEditorUtils::SetMetaData` 写入
- [x] 5.3 实现成员变量 MetaData 删除（直接操作 `FStructVariableDescription::MetaData`）

## 6. 事务与刷新
- [x] 6.1 所有修改包在 `FScopedTransaction` 中
- [x] 6.2 修改前调用 `Struct->Modify()`
- [x] 6.3 修改后触发 `FStructureEditorUtils::OnStructureChanged`
- [x] 6.4 Undo / Redo 支持

## 7. 验证
- [x] 7.1 执行 `openspec validate add-user-defined-struct-metadata-editor --strict --no-interactive`
- [x] 7.2 编译 `TireflyGameplayUtilsEditor Win64 Development`
- [x] 7.3 手动验证：结构体自身 MetaData 新增、编辑、删除、保存重开
- [x] 7.4 手动验证：结构体成员变量 MetaData 新增、编辑、删除、保存重开
- [x] 7.5 手动验证：Undo / Redo
- [x] 7.6 手动验证：修改后引用该结构体的 DataTable / 蓝图属性正确刷新
- [x] 7.7 手动验证：现有功能不受影响
