## ADDED Requirements

### Requirement: 函数参数 MetaData sidecar 持久化
TireflyBlueprintGraphUtils SHALL persist function parameter MetaData on the Blueprint asset as a sidecar data structure, without modifying the engine's `FUserPinInfo` type.

#### Scenario: 用户保存并重新打开蓝图
- **WHEN** 用户为函数参数添加 MetaData 后保存蓝图并重新打开
- **THEN** 系统 MUST 保留该参数的 sidecar MetaData
- **AND** 该 MetaData MUST 不依赖 `FUserPinInfo` 的任何字段变更

#### Scenario: 用户删除函数参数
- **WHEN** 用户删除一个已有 sidecar MetaData 的函数参数
- **THEN** 系统 SHOULD 清理对应的孤儿 sidecar 条目
- **AND** 系统 MUST NOT 因孤儿数据导致崩溃或序列化错误

#### Scenario: 旧版本蓝图兼容
- **WHEN** 不含 sidecar extension 的旧蓝图被打开
- **THEN** 系统 MUST 正常加载且不报错
- **AND** 系统 MUST NOT 自动创建空的 extension 除非用户实际编辑了参数 MetaData

### Requirement: 函数参数 MetaData 编译期注入
TireflyBlueprintGraphUtils SHALL inject sidecar parameter MetaData into the generated `UFunction` parameter `FProperty` during Blueprint compilation, so that `UK2Node_CallFunction::GetPinMetaData()` can read it.

#### Scenario: 编译后 Pin Factory 能读到参数 MetaData
- **WHEN** 用户给蓝图函数参数添加 `GetParamOptions` MetaData 并编译蓝图
- **THEN** 调用该函数的节点的 Pin Factory MUST 能通过 `GetPinMetaData()` 读到该 MetaData
- **AND** 注入 MUST 同时覆盖 `SkeletonGeneratedClass` 和 `GeneratedClass`

#### Scenario: 每次编译后重新注入
- **WHEN** 蓝图被重新编译
- **THEN** 系统 MUST 重新注入所有 sidecar 参数 MetaData
- **AND** 注入结果 MUST 不受上一次编译产物的影响

#### Scenario: 编译注入使用公开扩展点
- **WHEN** 插件注册编译期注入
- **THEN** it SHOULD use `UBlueprintCompilerExtension` and `FBlueprintCompilationManager::RegisterCompilerExtension` or equivalent public APIs
- **AND** it MUST NOT depend on private Kismet compiler internals

### Requirement: 函数参数 MetaData 编辑 UI
TireflyBlueprintGraphUtils SHALL provide a `Parameters Meta Data` section in the Blueprint function Details panel, grouped by parameter name.

#### Scenario: 用户选中函数
- **WHEN** 用户在蓝图编辑器中选中一个用户声明函数
- **THEN** Details 面板 MUST include a `Parameters Meta Data` section
- **AND** the section MUST display one MetaData editor per function parameter, grouped by parameter name

#### Scenario: 用户编辑函数参数 MetaData
- **WHEN** 用户在 `Parameters Meta Data` 分组中为某个参数新增、编辑或删除 MetaData
- **THEN** 系统 MUST persist the change to the sidecar extension on the Blueprint asset
- **AND** the change MUST trigger a Blueprint compile so that injection takes effect

#### Scenario: 复用公共 MetaData 编辑控件
- **WHEN** 插件渲染参数 MetaData 编辑 UI
- **THEN** it SHOULD reuse `STireflyMetaDataEditor` or an equivalent shared component
- **AND** it MUST provide the same Key/Value editing, validation, and common Key suggestions as variable and function MetaData editors

### Requirement: 函数参数 MetaData 事务与脏标记
TireflyBlueprintGraphUtils SHALL wrap function parameter MetaData mutations in editor transactions and mark the owning Blueprint as modified.

#### Scenario: 参数 MetaData 修改进入 Undo 栈
- **WHEN** 用户修改函数参数 MetaData
- **THEN** 修改 MUST 创建 editor transaction
- **AND** 用户 MUST be able to undo and redo the change through the editor

#### Scenario: 修改后蓝图标记为 dirty
- **WHEN** 用户成功新增、编辑、删除或重命名参数 MetaData
- **THEN** 所属 Blueprint MUST be marked as modified

### Requirement: 函数参数重命名处理
TireflyBlueprintGraphUtils SHALL migrate sidecar MetaData when a function parameter is renamed, or otherwise provide a clear orphan-handling mechanism.

#### Scenario: 用户重命名参数
- **WHEN** 用户重命名一个已有 sidecar MetaData 的函数参数
- **THEN** 系统 SHOULD migrate the sidecar MetaData to the new parameter name
- **AND** if migration is not possible, the system MUST mark the old entry as orphaned and provide a recovery or cleanup path

#### Scenario: 孤儿数据不导致崩溃
- **WHEN** sidecar 中存在与当前参数列表不匹配的条目
- **THEN** 系统 MUST NOT crash or block editing
- **AND** 系统 SHOULD provide a way to view and clean up orphaned entries

### Requirement: 参数 MetaData 常用 Key 建议以参数级 MetaData 为核心
TireflyBlueprintGraphUtils SHALL provide common MetaData Key suggestions relevant to function parameters, with `GetParamOptions` as the primary suggestion.

#### Scenario: 用户打开常用 Key 建议入口
- **WHEN** 用户在参数 MetaData 编辑器中打开常用 Key 建议入口
- **THEN** 系统 SHOULD show parameter-oriented suggestions such as `GetParamOptions`
- **AND** 系统 MUST NOT include keys that already exist on the current parameter in the suggestions
