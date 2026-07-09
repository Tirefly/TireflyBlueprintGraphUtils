## MODIFIED Requirements

### Requirement: 第一阶段不得实现参数和结构体 MetaData 编辑
The Blueprint MetaData editor SHALL be limited to the shared Key/Value UI, Blueprint variable MetaData, and Blueprint function MetaData. Function parameter MetaData is now covered by the `blueprint-function-param-metadata` capability. UserDefinedStruct MetaData is now covered by the `user-defined-struct-metadata` capability.

#### Scenario: 用户期望编辑函数参数 MetaData
- **WHEN** 用户需要编辑 Blueprint function parameter MetaData such as `UPARAM`-style metadata
- **THEN** this capability MUST defer to the `blueprint-function-param-metadata` capability for that workflow

#### Scenario: 用户期望编辑 UserDefinedStruct MetaData
- **WHEN** 用户需要编辑 UserDefinedStruct or struct member MetaData
- **THEN** this capability MUST defer to the `user-defined-struct-metadata` capability for that workflow
- **AND** the `user-defined-struct-metadata` capability MUST be active before struct MetaData editing is available
