// Copyright Tirefly. All Rights Reserved.

#include "ParamMetaData/TireflyParamMetaDataCompilerExtension.h"

#include "Engine/Blueprint.h"
#include "ParamMetaData/TireflyBlueprintParamMetaDataExtension.h"
#include "UObject/UnrealType.h"


void UTireflyParamMetaDataCompilerExtension::ProcessBlueprintCompiled(
	const FKismetCompilerContext& CompilationContext,
	const FBlueprintCompiledData& Data)
{
	UBlueprint* Blueprint = CompilationContext.Blueprint;
	if (!Blueprint)
	{
		return;
	}

	// 查找 sidecar extension
	UTireflyBlueprintParamMetaDataExtension* ParamMetaExtension = nullptr;
	for (const TObjectPtr<UBlueprintExtension>& Extension : Blueprint->GetExtensions())
	{
		ParamMetaExtension = Cast<UTireflyBlueprintParamMetaDataExtension>(Extension);
		if (ParamMetaExtension)
		{
			break;
		}
	}

	if (!ParamMetaExtension)
	{
		return;
	}

	const TArray<FTireflyFunctionParamMetaData>& AllFuncMeta = ParamMetaExtension->GetAllFunctionParamMetaData();
	if (AllFuncMeta.Num() == 0)
	{
		return;
	}

	// 获取需要注入的两个 class
	UClass* GeneratedClass = CompilationContext.NewClass;
	UClass* SkeletonClass = Blueprint->SkeletonGeneratedClass.Get();

	// 遍历 sidecar 中每个函数的参数 MetaData，注入到 UFunction 参数 FProperty
	for (const FTireflyFunctionParamMetaData& FuncMeta : AllFuncMeta)
	{
		// 注入到 GeneratedClass
		if (GeneratedClass)
		{
			UFunction* Function = GeneratedClass->FindFunctionByName(FuncMeta.FunctionName);
			InjectFunctionMetaData(Function, FuncMeta);
			InjectParamMetaData(Function, FuncMeta);
		}

		// 注入到 SkeletonGeneratedClass（编辑器中 GetPinMetaData 在编译期间可能查此 class）
		if (SkeletonClass && SkeletonClass != GeneratedClass)
		{
			UFunction* Function = SkeletonClass->FindFunctionByName(FuncMeta.FunctionName);
			InjectFunctionMetaData(Function, FuncMeta);
			InjectParamMetaData(Function, FuncMeta);
		}
	}
}

void UTireflyParamMetaDataCompilerExtension::InjectFunctionMetaData(
	UFunction* Function, const FTireflyFunctionParamMetaData& FuncMeta)
{
	if (!Function || FuncMeta.FunctionMetaData.Num() == 0)
	{
		return;
	}

	for (const auto& MetaPair : FuncMeta.FunctionMetaData)
	{
		Function->SetMetaData(MetaPair.Key, *MetaPair.Value);
	}
}

void UTireflyParamMetaDataCompilerExtension::InjectParamMetaData(
	UFunction* Function, const FTireflyFunctionParamMetaData& FuncMeta)
{
	if (!Function)
	{
		return;
	}

	// 遍历 UFunction 的参数 FProperty
	for (TFieldIterator<FProperty> It(Function); It && (It->PropertyFlags & CPF_Parm); ++It)
	{
		FProperty* Property = *It;

		// 查找该参数的 sidecar MetaData
		for (const FTireflyParamMetaDataEntry& ParamEntry : FuncMeta.ParamMetaData)
		{
			if (Property->GetFName() == ParamEntry.ParamName)
			{
				// 写入所有 Key/Value
				for (const auto& MetaPair : ParamEntry.MetaData)
				{
					Property->SetMetaData(MetaPair.Key, *MetaPair.Value);
				}
				break;
			}
		}
	}
}
