// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"


class FTireflyParamOptionPinFactory;


class FTireflyBlueprintGraphUtilsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	// 注册蓝图编辑器 Details 扩展。
	void RegisterBlueprintDetailsCustomizations();

	// 注销蓝图编辑器 Details 扩展。
	void UnregisterBlueprintDetailsCustomizations();

private:
	// 函数参数选项 Pin 工厂。
	TSharedPtr<FTireflyParamOptionPinFactory> TireflyNameOptionPinFactory;

	// 蓝图变量 Details 扩展句柄。
	FDelegateHandle BlueprintVariableCustomizationHandle;

	// 蓝图函数 Details 扩展句柄。
	FDelegateHandle BlueprintFunctionCustomizationHandle;

	// 引擎初始化完成后注册蓝图 Details 扩展的回调句柄。
	FDelegateHandle PostEngineInitHandle;
};
