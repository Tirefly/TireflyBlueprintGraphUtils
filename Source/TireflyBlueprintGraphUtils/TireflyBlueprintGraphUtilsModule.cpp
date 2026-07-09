// Copyright Tirefly. All Rights Reserved.

#include "TireflyBlueprintGraphUtilsModule.h"

#include "BlueprintCompilationManager.h"
#include "BlueprintEditorModule.h"
#include "EdGraphUtilities.h"
#include "Editor.h"
#include "FunctionParamOptions/TireflyParamOptionPinFactory.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_FunctionEntry.h"
#include "MetaDataEditor/TireflyBlueprintFunctionMetaDataDetails.h"
#include "MetaDataEditor/TireflyBlueprintVariableMetaDataDetails.h"
#include "Misc/CoreDelegates.h"
#include "Modules/ModuleManager.h"
#include "ParamMetaData/TireflyParamMetaDataCompilerExtension.h"
#include "StructMetaData/TireflyStructMetaDataEditorManager.h"
#include "UObject/UnrealType.h"


#define LOCTEXT_NAMESPACE "FTireflyBlueprintGraphUtilsModule"

void FTireflyBlueprintGraphUtilsModule::StartupModule()
{
	TireflyNameOptionPinFactory = MakeShareable(new FTireflyParamOptionPinFactory());
	FEdGraphUtilities::RegisterVisualPinFactory(TireflyNameOptionPinFactory);

	RegisterParamMetaDataCompilerExtension();

	if (GEditor)
	{
		RegisterBlueprintDetailsCustomizations();
	}
	else
	{
		PostEngineInitHandle = FCoreDelegates::OnPostEngineInit.AddRaw(this, &FTireflyBlueprintGraphUtilsModule::RegisterBlueprintDetailsCustomizations);
	}
}

void FTireflyBlueprintGraphUtilsModule::ShutdownModule()
{
	if (PostEngineInitHandle.IsValid())
	{
		FCoreDelegates::OnPostEngineInit.Remove(PostEngineInitHandle);
		PostEngineInitHandle.Reset();
	}

	UnregisterBlueprintDetailsCustomizations();
	UnregisterParamMetaDataCompilerExtension();

	if (TireflyNameOptionPinFactory.IsValid())
	{
		FEdGraphUtilities::UnregisterVisualPinFactory(TireflyNameOptionPinFactory);
		TireflyNameOptionPinFactory.Reset();
	}
}

void FTireflyBlueprintGraphUtilsModule::RegisterBlueprintDetailsCustomizations()
{
	FBlueprintEditorModule& BlueprintEditorModule = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>("Kismet");
	BlueprintVariableCustomizationHandle = BlueprintEditorModule.RegisterVariableCustomization(
		FProperty::StaticClass(),
		FOnGetVariableCustomizationInstance::CreateStatic(&FTireflyBlueprintVariableMetaDataDetails::MakeInstance));

	BlueprintFunctionCustomizationHandle = BlueprintEditorModule.RegisterFunctionCustomization(
		UK2Node_FunctionEntry::StaticClass(),
		FOnGetFunctionCustomizationInstance::CreateStatic(&FTireflyBlueprintFunctionMetaDataDetails::MakeInstance));

	CustomEventCustomizationHandle = BlueprintEditorModule.RegisterFunctionCustomization(
		UK2Node_CustomEvent::StaticClass(),
		FOnGetFunctionCustomizationInstance::CreateStatic(&FTireflyBlueprintFunctionMetaDataDetails::MakeInstance));

	// 注册结构体 MetaData 编辑器工具栏扩展
	StructMetaDataEditorManager = MakeUnique<FTireflyStructMetaDataEditorManager>();
	StructMetaDataEditorManager->Register();
}

void FTireflyBlueprintGraphUtilsModule::UnregisterBlueprintDetailsCustomizations()
{
	if (!FModuleManager::Get().IsModuleLoaded("Kismet"))
	{
		return;
	}

	FBlueprintEditorModule& BlueprintEditorModule = FModuleManager::GetModuleChecked<FBlueprintEditorModule>("Kismet");
	if (BlueprintVariableCustomizationHandle.IsValid())
	{
		BlueprintEditorModule.UnregisterVariableCustomization(FProperty::StaticClass(), BlueprintVariableCustomizationHandle);
		BlueprintVariableCustomizationHandle.Reset();
	}

	if (BlueprintFunctionCustomizationHandle.IsValid())
	{
		BlueprintEditorModule.UnregisterFunctionCustomization(UK2Node_FunctionEntry::StaticClass(), BlueprintFunctionCustomizationHandle);
		BlueprintFunctionCustomizationHandle.Reset();
	}

	if (CustomEventCustomizationHandle.IsValid())
	{
		BlueprintEditorModule.UnregisterFunctionCustomization(UK2Node_CustomEvent::StaticClass(), CustomEventCustomizationHandle);
		CustomEventCustomizationHandle.Reset();
	}

	if (StructMetaDataEditorManager.IsValid())
	{
		StructMetaDataEditorManager->Unregister();
		StructMetaDataEditorManager.Reset();
	}
}

void FTireflyBlueprintGraphUtilsModule::RegisterParamMetaDataCompilerExtension()
{
	ParamMetaDataCompilerExtension = TStrongObjectPtr<UTireflyParamMetaDataCompilerExtension>(
		NewObject<UTireflyParamMetaDataCompilerExtension>());
	FBlueprintCompilationManager::RegisterCompilerExtension(
		UBlueprint::StaticClass(), ParamMetaDataCompilerExtension.Get());
}

void FTireflyBlueprintGraphUtilsModule::UnregisterParamMetaDataCompilerExtension()
{
	// FBlueprintCompilationManager 没有公开反注册接口，
	// 但编辑器插件模块在编辑器生命周期内不会被卸载，extension 实例会随模块存活。
	// 释放 TStrongObjectPtr 让 UObject 进入 GC 流程。
	ParamMetaDataCompilerExtension.Reset();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTireflyBlueprintGraphUtilsModule, TireflyBlueprintGraphUtils)
