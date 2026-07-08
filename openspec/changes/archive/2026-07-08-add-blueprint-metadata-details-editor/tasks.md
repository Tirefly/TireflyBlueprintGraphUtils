## 1. 规范与设计
- [x] 1.1 审核并批准阶段一 proposal
- [x] 1.2 确认第一版常用 MetaData Key 建议列表
- [x] 1.3 确认 Custom Event 是否纳入第一版函数 MetaData 编辑范围

## 2. 公共 MetaData 编辑 UI
- [x] 2.1 新增类 `TMap` 风格 Key / Value 编辑控件
- [x] 2.2 支持新增、编辑 Key、编辑 Value、删除行
- [x] 2.3 支持空字符串 Value
- [x] 2.4 实现空 Key、重复 Key 校验
- [x] 2.5 提供按变量 / 函数区分的常用 Key 建议入口，并过滤已存在 Key
- [x] 2.6 标记 Engine-managed Key，并在删除时给出提示
- [x] 2.7 为常用 Key 下拉项添加鼠标悬停 tooltip，描述各 Key 的作用

## 3. 蓝图变量 Details 扩展
- [x] 3.1 在模块启动时注册变量 Details customization
- [x] 3.2 在模块关闭时反注册变量 Details customization
- [x] 3.3 定位当前蓝图变量与所属蓝图
- [x] 3.4 读取变量 MetaData 并显示在 `MetaData` 分组
- [x] 3.5 写入、删除、重命名变量 MetaData
- [x] 3.6 支持 Undo / Redo、蓝图 dirty 标记和 Details 刷新

## 4. 蓝图函数 Details 扩展
- [x] 4.1 在模块启动时注册函数 Details customization
- [x] 4.2 在模块关闭时反注册函数 Details customization
- [x] 4.3 定位当前用户声明函数与 `FKismetUserDeclaredFunctionMetadata`
- [x] 4.4 读取函数 MetaData 并显示在 `MetaData` 分组
- [x] 4.5 写入、删除、重命名函数 MetaData
- [x] 4.6 支持 Undo / Redo、蓝图 dirty 标记和 Details 刷新

## 5. 验证
- [x] 5.1 执行 `openspec validate add-blueprint-metadata-details-editor --strict --no-interactive`
- [x] 5.2 编译 `TireflyGameplayUtilsEditor Win64 Development`
- [x] 5.3 手动验证蓝图变量 MetaData 新增、编辑、删除、保存重开
- [x] 5.4 手动验证蓝图函数 MetaData 新增、编辑、删除、保存重开
- [x] 5.5 手动验证 Undo / Redo
- [x] 5.6 手动验证现有 `GetParamOptions` 行为不受影响
- [x] 5.7 手动验证常用 Key 下拉 tooltip 显示
