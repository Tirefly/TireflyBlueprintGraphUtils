// Copyright Tirefly. All Rights Reserved.

#include "MetaDataEditor/TireflyBlueprintVariableMetaDataDetails.h"

#include "BlueprintEditor.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/Blueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "MetaDataEditor/STireflyMetaDataEditor.h"
#include "ScopedTransaction.h"
#include "UObject/PropertyWrapper.h"


#define LOCTEXT_NAMESPACE "TireflyBlueprintVariableMetaDataDetails"


TSharedPtr<IDetailCustomization> FTireflyBlueprintVariableMetaDataDetails::MakeInstance(TSharedPtr<IBlueprintEditor> BlueprintEditor)
{
	return MakeShared<FTireflyBlueprintVariableMetaDataDetails>(BlueprintEditor);
}

FTireflyBlueprintVariableMetaDataDetails::FTireflyBlueprintVariableMetaDataDetails(TWeakPtr<IBlueprintEditor> InBlueprintEditor)
	: BlueprintEditor(InBlueprintEditor)
{
}

void FTireflyBlueprintVariableMetaDataDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
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
		.OnGetMetaDataMap(FTireflyGetMetaDataMap::CreateSP(this, &FTireflyBlueprintVariableMetaDataDetails::GetMetaDataMap))
		.OnSetMetaData(FTireflySetMetaData::CreateSP(this, &FTireflyBlueprintVariableMetaDataDetails::SetMetaData))
		.OnRemoveMetaData(FTireflyRemoveMetaData::CreateSP(this, &FTireflyBlueprintVariableMetaDataDetails::RemoveMetaData))
		.OnRenameMetaData(FTireflyRenameMetaData::CreateSP(this, &FTireflyBlueprintVariableMetaDataDetails::RenameMetaData))
		.CommonKeys(TArray<FName>
		{
			TEXT("GetOptions"),
			TEXT("EditCondition"),
			TEXT("EditConditionHides"),
			TEXT("AllowedClasses"),
			TEXT("ExactClass"),
			TEXT("MustImplement"),
			TEXT("MetaClass")
		})
		.KeyDescriptions(FTireflyMetaDataKeyDescriptions
		{
			{ TEXT("GetOptions"), LOCTEXT("GetOptionsDesc", "Specifies a function name that returns an array of options for this variable's dropdown pin.") },
			{ TEXT("EditCondition"), LOCTEXT("EditConditionDesc", "A boolean expression that controls whether this property is editable.") },
			{ TEXT("EditConditionHides"), LOCTEXT("EditConditionHidesDesc", "Hides this property when the EditCondition is false.") },
			{ TEXT("AllowedClasses"), LOCTEXT("AllowedClassesDesc", "Restricts asset picker to only show assets of the specified class(es).") },
			{ TEXT("ExactClass"), LOCTEXT("ExactClassDesc", "When used with AllowedClasses, requires an exact class match instead of allowing subclasses.") },
			{ TEXT("MustImplement"), LOCTEXT("MustImplementDesc", "Restricts class picker to classes that implement the specified interface.") },
			{ TEXT("MetaClass"), LOCTEXT("MetaClassDesc", "Restricts class picker to the specified class and its subclasses.") }
		})
	];
}

bool FTireflyBlueprintVariableMetaDataDetails::TryInitializeTarget(IDetailLayoutBuilder& DetailBuilder)
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
		if (const UPropertyWrapper* PropertyWrapper = Cast<UPropertyWrapper>(SelectedObject.Get()))
		{
			if (const FProperty* Property = PropertyWrapper->GetProperty())
			{
				const FName CandidateVariableName = Property->GetFName();
				if (FBlueprintEditorUtils::FindNewVariableIndex(Blueprint.Get(), CandidateVariableName) != INDEX_NONE)
				{
					VariableName = CandidateVariableName;
					return true;
				}
			}
		}
	}

	return false;
}

TMap<FName, FString> FTireflyBlueprintVariableMetaDataDetails::GetMetaDataMap() const
{
	TMap<FName, FString> Result;
	if (!Blueprint.IsValid() || VariableName.IsNone())
	{
		return Result;
	}

	const int32 VariableIndex = FBlueprintEditorUtils::FindNewVariableIndex(Blueprint.Get(), VariableName);
	if (VariableIndex == INDEX_NONE)
	{
		return Result;
	}

	const FBPVariableDescription& VariableDescription = Blueprint->NewVariables[VariableIndex];
	for (const FBPVariableMetaDataEntry& Entry : VariableDescription.MetaDataArray)
	{
		if (!Entry.DataKey.IsNone())
		{
			Result.Add(Entry.DataKey, Entry.DataValue);
		}
	}

	return Result;
}

void FTireflyBlueprintVariableMetaDataDetails::SetMetaData(FName Key, const FString& Value)
{
	if (!Blueprint.IsValid() || VariableName.IsNone() || Key.IsNone())
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("SetVariableMetaData", "Set Blueprint Variable MetaData"));
	Blueprint->Modify();
	FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint.Get(), VariableName, nullptr, Key, Value);
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint.Get());
}

void FTireflyBlueprintVariableMetaDataDetails::RemoveMetaData(FName Key)
{
	if (!Blueprint.IsValid() || VariableName.IsNone() || Key.IsNone())
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("RemoveVariableMetaData", "Remove Blueprint Variable MetaData"));
	Blueprint->Modify();
	FBlueprintEditorUtils::RemoveBlueprintVariableMetaData(Blueprint.Get(), VariableName, nullptr, Key);
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint.Get());
}

void FTireflyBlueprintVariableMetaDataDetails::RenameMetaData(FName OldKey, FName NewKey)
{
	if (!Blueprint.IsValid() || VariableName.IsNone() || OldKey.IsNone() || NewKey.IsNone() || OldKey == NewKey)
	{
		return;
	}

	FString ExistingValue;
	if (!FBlueprintEditorUtils::GetBlueprintVariableMetaData(Blueprint.Get(), VariableName, nullptr, OldKey, ExistingValue))
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("RenameVariableMetaData", "Rename Blueprint Variable MetaData"));
	Blueprint->Modify();
	FBlueprintEditorUtils::RemoveBlueprintVariableMetaData(Blueprint.Get(), VariableName, nullptr, OldKey);
	FBlueprintEditorUtils::SetBlueprintVariableMetaData(Blueprint.Get(), VariableName, nullptr, NewKey, ExistingValue);
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint.Get());
}

#undef LOCTEXT_NAMESPACE
