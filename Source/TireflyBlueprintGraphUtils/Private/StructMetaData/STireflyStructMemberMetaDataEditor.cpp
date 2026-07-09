// Copyright Tirefly. All Rights Reserved.

#include "StructMetaData/STireflyStructMemberMetaDataEditor.h"

#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/StructureEditorUtils.h"
#include "MetaDataEditor/STireflyMetaDataEditor.h"
#include "ScopedTransaction.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "StructUtils/UserDefinedStruct.h"
#include "UserDefinedStructure/UserDefinedStructEditorData.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"


#define LOCTEXT_NAMESPACE "STireflyStructMemberMetaDataEditor"


void STireflyStructMemberMetaDataEditor::Construct(const FArguments& InArgs)
{
	Struct = InArgs._Struct;

	TSharedPtr<SVerticalBox> MainBox;

	ChildSlot
	[
		SAssignNew(MainBox, SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("MemberMetaDataTitle", "Member Variable MetaData"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]
	];

	if (Struct.IsValid())
	{
		const TArray<FStructVariableDescription>& VarDescs = FStructureEditorUtils::GetVarDesc(Struct.Get());

		for (const FStructVariableDescription& VarDesc : VarDescs)
		{
			const FGuid VarGuid = VarDesc.VarGuid;
			const FString VarName = VarDesc.VarName.ToString();

			MainBox->AddSlot()
			.AutoHeight()
			.Padding(FMargin(4.f, 2.f))
			[
				SNew(SExpandableArea)
				.InitiallyCollapsed(true)
				.BorderImage(FAppStyle::GetBrush("DetailsView.CategoryTop"))
				.BorderBackgroundColor(FSlateColor::UseSubduedForeground())
				.HeaderContent()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(VarName))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
					]
				]
				.BodyContent()
				[
					SNew(SBox)
					.Padding(FMargin(4.f, 2.f))
					[
						SNew(STireflyMetaDataEditor)
						.OnGetMetaDataMap(FTireflyGetMetaDataMap::CreateLambda(
							[this, VarGuid]() { return GetMemberMetaDataMap(VarGuid); }))
						.OnSetMetaData(FTireflySetMetaData::CreateLambda(
							[this, VarGuid](FName Key, const FString& Value) { SetMemberMetaData(VarGuid, Key, Value); }))
						.OnRemoveMetaData(FTireflyRemoveMetaData::CreateLambda(
							[this, VarGuid](FName Key) { RemoveMemberMetaData(VarGuid, Key); }))
						.OnRenameMetaData(FTireflyRenameMetaData::CreateLambda(
							[this, VarGuid](FName OldKey, FName NewKey) { RenameMemberMetaData(VarGuid, OldKey, NewKey); }))
						.CommonKeys(TArray<FName>
						{
							TEXT("GetParamOptions"),
							TEXT("DisplayPriority"),
							TEXT("ForceShow"),
							TEXT("NoResetToDefault")
						})
						.KeyDescriptions(FTireflyMetaDataKeyDescriptions
						{
							{ TEXT("GetParamOptions"), LOCTEXT("GetParamOptionsDesc", "Provides a dropdown list of options for this member.") },
							{ TEXT("DisplayPriority"), LOCTEXT("DisplayPriorityDesc", "Sets the display order of this member in detail panels.") },
							{ TEXT("ForceShow"), LOCTEXT("ForceShowDesc", "Forces this member to be shown even if the struct pin is not split.") },
							{ TEXT("NoResetToDefault"), LOCTEXT("NoResetToDefaultDesc", "Hides the 'Reset to Default' option for this member.") }
						})
					]
				]
			];
		}
	}
}

TMap<FName, FString> STireflyStructMemberMetaDataEditor::GetMemberMetaDataMap(FGuid VarGuid) const
{
	TMap<FName, FString> Result;
	if (!Struct.IsValid())
	{
		return Result;
	}

	const FStructVariableDescription* VarDesc = FStructureEditorUtils::GetVarDescByGuid(Struct.Get(), VarGuid);
	if (VarDesc)
	{
		Result = VarDesc->MetaData;

		if (!VarDesc->ToolTip.IsEmpty())
		{
			Result.Add(TEXT("ToolTip"), VarDesc->ToolTip);
		}
	}
	return Result;
}

void STireflyStructMemberMetaDataEditor::SetMemberMetaData(FGuid VarGuid, FName Key, const FString& Value)
{
	if (!Struct.IsValid() || !VarGuid.IsValid() || Key.IsNone())
	{
		return;
	}

	if (Key == TEXT("ToolTip"))
	{
		FStructureEditorUtils::ChangeVariableTooltip(Struct.Get(), VarGuid, Value);
		return;
	}

	// 不能使用 FStructureEditorUtils::SetMetaData 处理空字符串，
	// 因为它会把空字符串视为“移除该 Key”。这里直接操作 VarDesc，
	// 允许空字符串作为合法 MetaData 值存在。
	const FScopedTransaction Transaction(LOCTEXT("SetMemberMetaData", "Set Member MetaData"));
	Struct->Modify();

	TArray<FStructVariableDescription>& VarDescs = FStructureEditorUtils::GetVarDesc(Struct.Get());
	for (FStructVariableDescription& VarDesc : VarDescs)
	{
		if (VarDesc.VarGuid == VarGuid)
		{
			VarDesc.MetaData.Add(Key, Value);
			break;
		}
	}

	Struct->MarkPackageDirty();
	FStructureEditorUtils::OnStructureChanged(Struct.Get());
}

void STireflyStructMemberMetaDataEditor::RemoveMemberMetaData(FGuid VarGuid, FName Key)
{
	if (!Struct.IsValid() || !VarGuid.IsValid() || Key.IsNone())
	{
		return;
	}

	if (Key == TEXT("ToolTip"))
	{
		FStructureEditorUtils::ChangeVariableTooltip(Struct.Get(), VarGuid, FString());
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("RemoveMemberMetaData", "Remove Member MetaData"));
	Struct->Modify();

	// FStructureEditorUtils 没有公开 RemoveMetaData，直接操作 VarDesc
	TArray<FStructVariableDescription>& VarDescs = FStructureEditorUtils::GetVarDesc(Struct.Get());
	for (FStructVariableDescription& VarDesc : VarDescs)
	{
		if (VarDesc.VarGuid == VarGuid)
		{
			VarDesc.MetaData.Remove(Key);
			break;
		}
	}

	Struct->MarkPackageDirty();
	FStructureEditorUtils::OnStructureChanged(Struct.Get());
}

void STireflyStructMemberMetaDataEditor::RenameMemberMetaData(FGuid VarGuid, FName OldKey, FName NewKey)
{
	if (!Struct.IsValid() || !VarGuid.IsValid() || OldKey.IsNone() || NewKey.IsNone() || OldKey == NewKey)
	{
		return;
	}

	const FStructVariableDescription* VarDesc = FStructureEditorUtils::GetVarDescByGuid(Struct.Get(), VarGuid);
	if (!VarDesc)
	{
		return;
	}

	const FString* Found = VarDesc->MetaData.Find(OldKey);
	if (!Found && OldKey != TEXT("ToolTip"))
	{
		return;
	}

	const FString ExistingValue = (OldKey == TEXT("ToolTip"))
		? VarDesc->ToolTip
		: *Found;

	if (OldKey == TEXT("ToolTip"))
	{
		FStructureEditorUtils::ChangeVariableTooltip(Struct.Get(), VarGuid, FString());
		if (NewKey == TEXT("ToolTip"))
		{
			FStructureEditorUtils::ChangeVariableTooltip(Struct.Get(), VarGuid, ExistingValue);
		}
		else
		{
			const FScopedTransaction Transaction(LOCTEXT("RenameMemberMetaData", "Rename Member MetaData"));
			Struct->Modify();
			TArray<FStructVariableDescription>& VarDescs = FStructureEditorUtils::GetVarDesc(Struct.Get());
			for (FStructVariableDescription& Desc : VarDescs)
			{
				if (Desc.VarGuid == VarGuid)
				{
					Desc.MetaData.Add(NewKey, ExistingValue);
					break;
				}
			}
			Struct->MarkPackageDirty();
			FStructureEditorUtils::OnStructureChanged(Struct.Get());
		}
		return;
	}

	if (NewKey == TEXT("ToolTip"))
	{
		const FScopedTransaction Transaction(LOCTEXT("RenameMemberMetaData", "Rename Member MetaData"));
		Struct->Modify();
		TArray<FStructVariableDescription>& VarDescs = FStructureEditorUtils::GetVarDesc(Struct.Get());
		for (FStructVariableDescription& Desc : VarDescs)
		{
			if (Desc.VarGuid == VarGuid)
			{
				Desc.MetaData.Remove(OldKey);
				break;
			}
		}
		FStructureEditorUtils::ChangeVariableTooltip(Struct.Get(), VarGuid, ExistingValue);
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("RenameMemberMetaData", "Rename Member MetaData"));
	Struct->Modify();

	TArray<FStructVariableDescription>& VarDescs = FStructureEditorUtils::GetVarDesc(Struct.Get());
	for (FStructVariableDescription& Desc : VarDescs)
	{
		if (Desc.VarGuid == VarGuid)
		{
			Desc.MetaData.Remove(OldKey);
			Desc.MetaData.Add(NewKey, ExistingValue);
			break;
		}
	}

	Struct->MarkPackageDirty();
	FStructureEditorUtils::OnStructureChanged(Struct.Get());
}

#undef LOCTEXT_NAMESPACE
