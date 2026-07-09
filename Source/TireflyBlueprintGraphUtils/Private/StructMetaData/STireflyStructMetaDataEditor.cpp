// Copyright Tirefly. All Rights Reserved.

#include "StructMetaData/STireflyStructMetaDataEditor.h"

#include "Kismet2/StructureEditorUtils.h"
#include "MetaDataEditor/STireflyMetaDataEditor.h"
#include "ScopedTransaction.h"
#include "Styling/CoreStyle.h"
#include "StructUtils/UserDefinedStruct.h"
#include "UObject/MetaData.h"
#include "UObject/Field.h"
#include "UObject/UnrealType.h"
#include "UserDefinedStructure/UserDefinedStructEditorData.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "STireflyStructMetaDataEditor"


void STireflyStructMetaDataEditor::Construct(const FArguments& InArgs)
{
	Struct = InArgs._Struct;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("StructMetaDataTitle", "Struct MetaData"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.Padding(4.f)
		[
			SNew(STireflyMetaDataEditor)
			.OnGetMetaDataMap(FTireflyGetMetaDataMap::CreateSP(this, &STireflyStructMetaDataEditor::GetMetaDataMap))
			.OnSetMetaData(FTireflySetMetaData::CreateSP(this, &STireflyStructMetaDataEditor::SetMetaData))
			.OnRemoveMetaData(FTireflyRemoveMetaData::CreateSP(this, &STireflyStructMetaDataEditor::RemoveMetaData))
			.OnRenameMetaData(FTireflyRenameMetaData::CreateSP(this, &STireflyStructMetaDataEditor::RenameMetaData))
			.CommonKeys(TArray<FName>
			{
				TEXT("HiddenByDefault"),
				TEXT("DisableSplitPin"),
				TEXT("DisplayName")
			})
			.KeyDescriptions(FTireflyMetaDataKeyDescriptions
			{
				{ TEXT("HiddenByDefault"), LOCTEXT("HiddenByDefaultDesc", "Hides the struct pin by default in detail panels.") },
				{ TEXT("DisableSplitPin"), LOCTEXT("DisableSplitPinDesc", "Prevents the struct pin from being split into individual members.") },
				{ TEXT("DisplayName"), LOCTEXT("DisplayNameDesc", "Sets the display name of the struct in the editor.") }
			})
		]
	];
}

TMap<FName, FString> STireflyStructMetaDataEditor::GetMetaDataMap() const
{
	TMap<FName, FString> Result;
	if (Struct.IsValid())
	{
		if (const UUserDefinedStructEditorData* StructEditorData = Cast<UUserDefinedStructEditorData>(Struct->EditorData))
		{
			if (!StructEditorData->ToolTip.IsEmpty())
			{
				Result.Add(TEXT("ToolTip"), StructEditorData->ToolTip);
			}
		}

		if (TMap<FName, FString>* MetaMap = FMetaData::GetMapForObject(Struct.Get()))
		{
			for (const TPair<FName, FString>& Pair : *MetaMap)
			{
				Result.Add(Pair.Key, Pair.Value);
			}
		}
	}
	return Result;
}

void STireflyStructMetaDataEditor::SetMetaData(FName Key, const FString& Value)
{
	if (!Struct.IsValid() || Key.IsNone())
	{
		return;
	}

	if (Key == TEXT("ToolTip"))
	{
		FStructureEditorUtils::ChangeTooltip(Struct.Get(), Value);
		Struct->MarkPackageDirty();
		FStructureEditorUtils::OnStructureChanged(Struct.Get());
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("SetStructMetaData", "Set Struct MetaData"));
	Struct->Modify();
	Struct->SetMetaData(Key, *Value);
	Struct->MarkPackageDirty();
	FStructureEditorUtils::OnStructureChanged(Struct.Get());
}

void STireflyStructMetaDataEditor::RemoveMetaData(FName Key)
{
	if (!Struct.IsValid() || Key.IsNone())
	{
		return;
	}

	if (Key == TEXT("ToolTip"))
	{
		FStructureEditorUtils::ChangeTooltip(Struct.Get(), FString());
		Struct->MarkPackageDirty();
		FStructureEditorUtils::OnStructureChanged(Struct.Get());
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("RemoveStructMetaData", "Remove Struct MetaData"));
	Struct->Modify();
	Struct->RemoveMetaData(Key);
	Struct->MarkPackageDirty();
	FStructureEditorUtils::OnStructureChanged(Struct.Get());
}

void STireflyStructMetaDataEditor::RenameMetaData(FName OldKey, FName NewKey)
{
	if (!Struct.IsValid() || OldKey.IsNone() || NewKey.IsNone() || OldKey == NewKey)
	{
		return;
	}

	if (!Struct->HasMetaData(OldKey))
	{
		if (OldKey != TEXT("ToolTip"))
		{
			return;
		}
	}

	const FString ExistingValue = (OldKey == TEXT("ToolTip"))
		? FStructureEditorUtils::GetTooltip(Struct.Get())
		: Struct->GetMetaData(OldKey);

	if (OldKey == TEXT("ToolTip"))
	{
		FStructureEditorUtils::ChangeTooltip(Struct.Get(), FString());
		if (NewKey == TEXT("ToolTip"))
		{
			FStructureEditorUtils::ChangeTooltip(Struct.Get(), ExistingValue);
		}
		else
		{
			Struct->SetMetaData(NewKey, *ExistingValue);
			Struct->MarkPackageDirty();
			FStructureEditorUtils::OnStructureChanged(Struct.Get());
		}
		return;
	}

	if (NewKey == TEXT("ToolTip"))
	{
		const FScopedTransaction Transaction(LOCTEXT("RenameStructMetaData", "Rename Struct MetaData"));
		Struct->Modify();
		Struct->RemoveMetaData(OldKey);
		FStructureEditorUtils::ChangeTooltip(Struct.Get(), ExistingValue);
		Struct->MarkPackageDirty();
		FStructureEditorUtils::OnStructureChanged(Struct.Get());
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("RenameStructMetaData", "Rename Struct MetaData"));
	Struct->Modify();
	Struct->RemoveMetaData(OldKey);
	Struct->SetMetaData(NewKey, *ExistingValue);
	Struct->MarkPackageDirty();
	FStructureEditorUtils::OnStructureChanged(Struct.Get());
}

#undef LOCTEXT_NAMESPACE
