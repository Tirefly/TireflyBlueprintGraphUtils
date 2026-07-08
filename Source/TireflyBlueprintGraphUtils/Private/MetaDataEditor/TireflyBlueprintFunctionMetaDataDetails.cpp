// Copyright Tirefly. All Rights Reserved.

#include "MetaDataEditor/TireflyBlueprintFunctionMetaDataDetails.h"

#include "BlueprintEditor.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/Blueprint.h"
#include "K2Node_FunctionEntry.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "MetaDataEditor/STireflyMetaDataEditor.h"
#include "ScopedTransaction.h"


#define LOCTEXT_NAMESPACE "TireflyBlueprintFunctionMetaDataDetails"


TSharedPtr<IDetailCustomization> FTireflyBlueprintFunctionMetaDataDetails::MakeInstance(TSharedPtr<IBlueprintEditor> BlueprintEditor)
{
	return MakeShared<FTireflyBlueprintFunctionMetaDataDetails>(BlueprintEditor);
}

FTireflyBlueprintFunctionMetaDataDetails::FTireflyBlueprintFunctionMetaDataDetails(TWeakPtr<IBlueprintEditor> InBlueprintEditor)
	: BlueprintEditor(InBlueprintEditor)
{
}

void FTireflyBlueprintFunctionMetaDataDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	if (!TryInitializeTarget(DetailBuilder))
	{
		return;
	}

	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("MetaData", LOCTEXT("MetaDataCategory", "MetaData"));
	Category.AddCustomRow(LOCTEXT("MetaDataFilter", "MetaData"))
	.WholeRowContent()
	[
		SNew(STireflyMetaDataEditor)
		.OnGetMetaDataMap(FTireflyGetMetaDataMap::CreateSP(this, &FTireflyBlueprintFunctionMetaDataDetails::GetMetaDataMap))
		.OnSetMetaData(FTireflySetMetaData::CreateSP(this, &FTireflyBlueprintFunctionMetaDataDetails::SetMetaData))
		.OnRemoveMetaData(FTireflyRemoveMetaData::CreateSP(this, &FTireflyBlueprintFunctionMetaDataDetails::RemoveMetaData))
		.OnRenameMetaData(FTireflyRenameMetaData::CreateSP(this, &FTireflyBlueprintFunctionMetaDataDetails::RenameMetaData))
		.CommonKeys(TArray<FName>
		{
			TEXT("WorldContext"),
			TEXT("DefaultToSelf"),
			TEXT("HidePin"),
			TEXT("AdvancedDisplay"),
			TEXT("AutoCreateRefTerm"),
			TEXT("DeterminesOutputType"),
			TEXT("DynamicOutputParam"),
			TEXT("ExpandEnumAsExecs"),
			TEXT("DevelopmentOnly")
		})
		.KeyDescriptions(FTireflyMetaDataKeyDescriptions
		{
			{ TEXT("WorldContext"), LOCTEXT("WorldContextDesc", "Specifies which parameter provides the World context for this function.") },
			{ TEXT("DefaultToSelf"), LOCTEXT("DefaultToSelfDesc", "Specifies which parameter defaults to 'self' when nothing is connected.") },
			{ TEXT("HidePin"), LOCTEXT("HidePinDesc", "Hides the specified parameter pin on the call node.") },
			{ TEXT("AdvancedDisplay"), LOCTEXT("AdvancedDisplayDesc", "Marks parameters as advanced (collapsed by default) on the call node.") },
			{ TEXT("AutoCreateRefTerm"), LOCTEXT("AutoCreateRefTermDesc", "Auto-creates literal input pins for reference parameters that are left unconnected.") },
			{ TEXT("DeterminesOutputType"), LOCTEXT("DeterminesOutputTypeDesc", "Specifies which input parameter determines the type of the output pin.") },
			{ TEXT("DynamicOutputParam"), LOCTEXT("DynamicOutputParamDesc", "Marks an output parameter as dynamically typed based on another parameter.") },
			{ TEXT("ExpandEnumAsExecs"), LOCTEXT("ExpandEnumAsExecsDesc", "Expands an enum parameter into separate exec output pins.") },
			{ TEXT("DevelopmentOnly"), LOCTEXT("DevelopmentOnlyDesc", "Restricts this function to development-only builds.") }
		})
	];
}

bool FTireflyBlueprintFunctionMetaDataDetails::TryInitializeTarget(IDetailLayoutBuilder& DetailBuilder)
{
	const TSharedPtr<IBlueprintEditor> BlueprintEditorPtr = BlueprintEditor.Pin();
	if (!BlueprintEditorPtr.IsValid())
	{
		return false;
	}

	const TSharedPtr<FBlueprintEditor> ConcreteBlueprintEditor = StaticCastSharedPtr<FBlueprintEditor>(BlueprintEditorPtr);
	Blueprint = ConcreteBlueprintEditor.IsValid() ? ConcreteBlueprintEditor->GetBlueprintObj() : nullptr;
	if (!Blueprint.IsValid())
	{
		return false;
	}

	const TArray<TWeakObjectPtr<UObject>>& SelectedObjects = DetailBuilder.GetSelectedObjects();
	for (const TWeakObjectPtr<UObject>& SelectedObject : SelectedObjects)
	{
		if (UEdGraph* Graph = Cast<UEdGraph>(SelectedObject.Get()))
		{
			if (FBlueprintEditorUtils::GetGraphFunctionMetaData(Graph))
			{
				FunctionGraph = Graph;
				return true;
			}
		}

		if (const UK2Node_FunctionEntry* FunctionEntry = Cast<UK2Node_FunctionEntry>(SelectedObject.Get()))
		{
			UEdGraph* Graph = FunctionEntry->GetGraph();
			if (Graph && FBlueprintEditorUtils::GetGraphFunctionMetaData(Graph))
			{
				FunctionGraph = Graph;
				return true;
			}
		}
	}

	return false;
}

TMap<FName, FString> FTireflyBlueprintFunctionMetaDataDetails::GetMetaDataMap() const
{
	TMap<FName, FString> Result;
	if (const FKismetUserDeclaredFunctionMetadata* MetaData = GetFunctionMetaData())
	{
		Result = MetaData->GetMetaDataMap();
	}
	return Result;
}

void FTireflyBlueprintFunctionMetaDataDetails::SetMetaData(FName Key, const FString& Value)
{
	if (!Blueprint.IsValid() || !FunctionGraph.IsValid() || Key.IsNone())
	{
		return;
	}

	FKismetUserDeclaredFunctionMetadata* MetaData = GetFunctionMetaData();
	if (!MetaData)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("SetFunctionMetaData", "Set Blueprint Function MetaData"));
	Blueprint->Modify();
	FBlueprintEditorUtils::ModifyFunctionMetaData(FunctionGraph.Get());
	MetaData->SetMetaData(Key, FString(Value));
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint.Get());
}

void FTireflyBlueprintFunctionMetaDataDetails::RemoveMetaData(FName Key)
{
	if (!Blueprint.IsValid() || !FunctionGraph.IsValid() || Key.IsNone())
	{
		return;
	}

	FKismetUserDeclaredFunctionMetadata* MetaData = GetFunctionMetaData();
	if (!MetaData)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("RemoveFunctionMetaData", "Remove Blueprint Function MetaData"));
	Blueprint->Modify();
	FBlueprintEditorUtils::ModifyFunctionMetaData(FunctionGraph.Get());
	MetaData->RemoveMetaData(Key);
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint.Get());
}

void FTireflyBlueprintFunctionMetaDataDetails::RenameMetaData(FName OldKey, FName NewKey)
{
	if (!Blueprint.IsValid() || !FunctionGraph.IsValid() || OldKey.IsNone() || NewKey.IsNone() || OldKey == NewKey)
	{
		return;
	}

	FKismetUserDeclaredFunctionMetadata* MetaData = GetFunctionMetaData();
	if (!MetaData || !MetaData->HasMetaData(OldKey))
	{
		return;
	}

	const FString ExistingValue = MetaData->GetMetaData(OldKey);
	const FScopedTransaction Transaction(LOCTEXT("RenameFunctionMetaData", "Rename Blueprint Function MetaData"));
	Blueprint->Modify();
	FBlueprintEditorUtils::ModifyFunctionMetaData(FunctionGraph.Get());
	MetaData->RemoveMetaData(OldKey);
	MetaData->SetMetaData(NewKey, FString(ExistingValue));
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint.Get());
}

FKismetUserDeclaredFunctionMetadata* FTireflyBlueprintFunctionMetaDataDetails::GetFunctionMetaData() const
{
	return FunctionGraph.IsValid()
		? FBlueprintEditorUtils::GetGraphFunctionMetaData(FunctionGraph.Get())
		: nullptr;
}

#undef LOCTEXT_NAMESPACE
