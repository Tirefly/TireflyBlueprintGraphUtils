// Copyright Tirefly. All Rights Reserved.

#include "TireflyBlueprintGraphUtilsModule.h"

#include "BlueprintEditorModule.h"
#include "EdGraphUtilities.h"
#include "Editor.h"
#include "FunctionParamOptions/TireflyParamOptionPinFactory.h"
#include "K2Node_FunctionEntry.h"
#include "MetaDataEditor/TireflyBlueprintFunctionMetaDataDetails.h"
#include "MetaDataEditor/TireflyBlueprintVariableMetaDataDetails.h"
#include "Misc/CoreDelegates.h"
#include "Modules/ModuleManager.h"
#include "UObject/UnrealType.h"


#define LOCTEXT_NAMESPACE "FTireflyBlueprintGraphUtilsModule"

void FTireflyBlueprintGraphUtilsModule::StartupModule()
{
	TireflyNameOptionPinFactory = MakeShareable(new FTireflyParamOptionPinFactory());
	FEdGraphUtilities::RegisterVisualPinFactory(TireflyNameOptionPinFactory);

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
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTireflyBlueprintGraphUtilsModule, TireflyBlueprintGraphUtils)
