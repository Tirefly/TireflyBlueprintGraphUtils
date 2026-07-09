// Copyright Tirefly. All Rights Reserved.

#include "ParamMetaData/TireflyBlueprintParamMetaDataExtension.h"

#include "Engine/Blueprint.h"


FTireflyFunctionParamMetaData* UTireflyBlueprintParamMetaDataExtension::FindOrAddFunction(FName FunctionName)
{
	for (FTireflyFunctionParamMetaData& Entry : FunctionParamMetaData)
	{
		if (Entry.FunctionName == FunctionName)
		{
			return &Entry;
		}
	}

	FTireflyFunctionParamMetaData& NewEntry = FunctionParamMetaData.AddDefaulted_GetRef();
	NewEntry.FunctionName = FunctionName;
	return &NewEntry;
}

const FTireflyFunctionParamMetaData* UTireflyBlueprintParamMetaDataExtension::FindFunction(FName FunctionName) const
{
	for (const FTireflyFunctionParamMetaData& Entry : FunctionParamMetaData)
	{
		if (Entry.FunctionName == FunctionName)
		{
			return &Entry;
		}
	}

	return nullptr;
}

void UTireflyBlueprintParamMetaDataExtension::RemoveFunction(FName FunctionName)
{
	for (int32 i = 0; i < FunctionParamMetaData.Num(); ++i)
	{
		if (FunctionParamMetaData[i].FunctionName == FunctionName)
		{
			FunctionParamMetaData.RemoveAt(i);
			return;
		}
	}
}

void UTireflyBlueprintParamMetaDataExtension::PurgeOrphanedFunctions(const TArray<FName>& ValidFunctionNames)
{
	for (int32 i = FunctionParamMetaData.Num() - 1; i >= 0; --i)
	{
		if (!ValidFunctionNames.Contains(FunctionParamMetaData[i].FunctionName))
		{
			FunctionParamMetaData.RemoveAt(i);
		}
	}
}

FTireflyParamMetaDataEntry* UTireflyBlueprintParamMetaDataExtension::FindOrAddParam(
	FTireflyFunctionParamMetaData& FunctionEntry, FName ParamName)
{
	for (FTireflyParamMetaDataEntry& Entry : FunctionEntry.ParamMetaData)
	{
		if (Entry.ParamName == ParamName)
		{
			return &Entry;
		}
	}

	FTireflyParamMetaDataEntry& NewEntry = FunctionEntry.ParamMetaData.AddDefaulted_GetRef();
	NewEntry.ParamName = ParamName;
	return &NewEntry;
}

void UTireflyBlueprintParamMetaDataExtension::RemoveParam(
	FTireflyFunctionParamMetaData& FunctionEntry, FName ParamName)
{
	for (int32 i = 0; i < FunctionEntry.ParamMetaData.Num(); ++i)
	{
		if (FunctionEntry.ParamMetaData[i].ParamName == ParamName)
		{
			FunctionEntry.ParamMetaData.RemoveAt(i);
			return;
		}
	}
}

void UTireflyBlueprintParamMetaDataExtension::RenameParam(
	FName FunctionName, FName OldParamName, FName NewParamName)
{
	FTireflyFunctionParamMetaData* FunctionEntry = FindOrAddFunction(FunctionName);
	if (!FunctionEntry)
	{
		return;
	}

	for (FTireflyParamMetaDataEntry& Entry : FunctionEntry->ParamMetaData)
	{
		if (Entry.ParamName == OldParamName)
		{
			Entry.ParamName = NewParamName;
			break;
		}
	}
}

void UTireflyBlueprintParamMetaDataExtension::PurgeOrphanedParams(
	FName FunctionName, const TArray<FName>& ValidParamNames)
{
	FTireflyFunctionParamMetaData* FunctionEntry = const_cast<FTireflyFunctionParamMetaData*>(
		FindFunction(FunctionName));
	if (!FunctionEntry)
	{
		return;
	}

	for (int32 i = FunctionEntry->ParamMetaData.Num() - 1; i >= 0; --i)
	{
		if (!ValidParamNames.Contains(FunctionEntry->ParamMetaData[i].ParamName))
		{
			FunctionEntry->ParamMetaData.RemoveAt(i);
		}
	}
}

UTireflyBlueprintParamMetaDataExtension* UTireflyBlueprintParamMetaDataExtension::GetOrCreate(
	UBlueprint* Blueprint)
{
	if (!Blueprint)
	{
		return nullptr;
	}

	for (TObjectPtr<UBlueprintExtension> Extension : Blueprint->GetExtensions())
	{
		if (UTireflyBlueprintParamMetaDataExtension* ParamMetaExt =
			Cast<UTireflyBlueprintParamMetaDataExtension>(Extension))
		{
			return ParamMetaExt;
		}
	}

	UTireflyBlueprintParamMetaDataExtension* NewExtension =
		NewObject<UTireflyBlueprintParamMetaDataExtension>(Blueprint);
	Blueprint->AddExtension(NewExtension);
	return NewExtension;
}

const UTireflyBlueprintParamMetaDataExtension* UTireflyBlueprintParamMetaDataExtension::Find(
	const UBlueprint* Blueprint)
{
	if (!Blueprint)
	{
		return nullptr;
	}

	for (const TObjectPtr<UBlueprintExtension>& Extension : Blueprint->GetExtensions())
	{
		if (const UTireflyBlueprintParamMetaDataExtension* ParamMetaExt =
			Cast<UTireflyBlueprintParamMetaDataExtension>(Extension))
		{
			return ParamMetaExt;
		}
	}

	return nullptr;
}
