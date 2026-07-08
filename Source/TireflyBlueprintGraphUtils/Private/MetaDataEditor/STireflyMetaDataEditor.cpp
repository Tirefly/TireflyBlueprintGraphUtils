// Copyright Tirefly. All Rights Reserved.

#include "MetaDataEditor/STireflyMetaDataEditor.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"


#define LOCTEXT_NAMESPACE "STireflyMetaDataEditor"


void STireflyMetaDataEditor::Construct(const FArguments& InArgs)
{
	OnGetMetaDataMap = InArgs._OnGetMetaDataMap;
	OnSetMetaData = InArgs._OnSetMetaData;
	OnRemoveMetaData = InArgs._OnRemoveMetaData;
	OnRenameMetaData = InArgs._OnRenameMetaData;
	CommonKeys = InArgs._CommonKeys;
	KeyDescriptions = InArgs._KeyDescriptions;

	RefreshRows();
	RefreshCommonKeyOptions();

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 4.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("AddMetaData", "Add"))
				.ToolTipText(LOCTEXT("AddMetaDataTooltip", "Add a new MetaData entry."))
				.OnClicked(this, &STireflyMetaDataEditor::OnAddClicked)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 0.f, 0.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(180.f)
				[
					SAssignNew(CommonKeyComboBox, SComboBox<TSharedPtr<FName>>)
					.OptionsSource(&AvailableCommonKeyOptions)
					.OnGenerateWidget(this, &STireflyMetaDataEditor::MakeCommonKeyWidget)
					.OnSelectionChanged(this, &STireflyMetaDataEditor::OnCommonKeySelected)
					.OnComboBoxOpening(this, &STireflyMetaDataEditor::RefreshCommonKeyOptions)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("CommonKeys", "Common Keys"))
					]
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 2.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(.35f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("KeyHeader", "MetaData Name"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(.55f)
			.Padding(4.f, 0.f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ValueHeader", "MetaData Value"))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(58.f)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(360.f)
		[
			SAssignNew(RowListView, SListView<TSharedPtr<FTireflyMetaDataEditorRow>>)
			.ListItemsSource(&Rows)
			.OnGenerateRow(this, &STireflyMetaDataEditor::MakeRowWidget)
		]
	];
}

void STireflyMetaDataEditor::RefreshRows()
{
	Rows.Reset();
	if (!OnGetMetaDataMap.IsBound())
	{
		return;
	}

	TMap<FName, FString> MetaDataMap = OnGetMetaDataMap.Execute();
	MetaDataMap.KeySort([](const FName& A, const FName& B)
	{
		return A.LexicalLess(B);
	});

	for (const TPair<FName, FString>& Pair : MetaDataMap)
	{
		TSharedPtr<FTireflyMetaDataEditorRow> Row = MakeShared<FTireflyMetaDataEditorRow>();
		Row->Key = Pair.Key;
		Row->Value = Pair.Value;
		Rows.Add(Row);
	}

	if (RowListView.IsValid())
	{
		RowListView->RequestListRefresh();
	}

	RefreshCommonKeyOptions();
}

void STireflyMetaDataEditor::RefreshCommonKeyOptions()
{
	AvailableCommonKeyOptions.Reset();
	const TMap<FName, FString> CurrentMetaDataMap = OnGetMetaDataMap.IsBound()
		? OnGetMetaDataMap.Execute()
		: TMap<FName, FString>();

	for (const FName& Key : CommonKeys)
	{
		if (!Key.IsNone() && !CurrentMetaDataMap.Contains(Key))
		{
			AvailableCommonKeyOptions.Add(MakeShared<FName>(Key));
		}
	}

	if (CommonKeyComboBox.IsValid())
	{
		CommonKeyComboBox->RefreshOptions();
	}
}

TSharedRef<ITableRow> STireflyMetaDataEditor::MakeRowWidget(TSharedPtr<FTireflyMetaDataEditorRow> RowItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	const FText KeyTooltip = IsEngineManagedKey(RowItem->Key)
		? LOCTEXT("EngineManagedKeyTooltip", "This is a common engine-managed MetaData key. Editing it may overlap with existing editor fields.")
		: FText::GetEmpty();

	return SNew(STableRow<TSharedPtr<FTireflyMetaDataEditorRow>>, OwnerTable)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(.35f)
		[
			SNew(SEditableTextBox)
			.Text(FText::FromName(RowItem->Key))
			.ToolTipText(KeyTooltip)
			.OnTextCommitted(this, &STireflyMetaDataEditor::OnKeyCommitted, RowItem)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(.55f)
		.Padding(4.f, 0.f)
		[
			SNew(SEditableTextBox)
			.Text(FText::FromString(RowItem->Value))
			.OnTextCommitted(this, &STireflyMetaDataEditor::OnValueCommitted, RowItem)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("RemoveMetaData", "Remove"))
			.ToolTipText(LOCTEXT("RemoveMetaDataTooltip", "Remove this MetaData entry."))
			.OnClicked(this, &STireflyMetaDataEditor::OnRemoveClicked, RowItem)
		]
	];
}

TSharedRef<SWidget> STireflyMetaDataEditor::MakeCommonKeyWidget(TSharedPtr<FName> Item) const
{
	const FText TooltipText = (Item.IsValid() && KeyDescriptions.Contains(*Item))
		? KeyDescriptions[*Item]
		: FText::GetEmpty();

	return SNew(STextBlock)
		.Text(Item.IsValid() ? FText::FromName(*Item) : FText::GetEmpty())
		.ToolTipText(TooltipText);
}

FReply STireflyMetaDataEditor::OnAddClicked()
{
	AddKey(TEXT("NewMetaData"));
	return FReply::Handled();
}

void STireflyMetaDataEditor::OnCommonKeySelected(TSharedPtr<FName> SelectedItem, ESelectInfo::Type SelectInfo)
{
	if (SelectInfo != ESelectInfo::Direct && SelectedItem.IsValid())
	{
		AddKey(*SelectedItem);
		RefreshCommonKeyOptions();
	}
}

void STireflyMetaDataEditor::OnKeyCommitted(const FText& NewText, ETextCommit::Type CommitType, TSharedPtr<FTireflyMetaDataEditorRow> RowItem)
{
	if (!RowItem.IsValid())
	{
		return;
	}

	const FName OldKey = RowItem->Key;
	const FName NewKey(*NewText.ToString().TrimStartAndEnd());
	FText ErrorText;
	if (!CanCommitKey(OldKey, NewKey, ErrorText))
	{
		FMessageDialog::Open(EAppMsgType::Ok, ErrorText);
		RefreshRows();
		return;
	}

	if (OldKey != NewKey && OnRenameMetaData.IsBound())
	{
		OnRenameMetaData.Execute(OldKey, NewKey);
		RefreshRows();
	}
}

void STireflyMetaDataEditor::OnValueCommitted(const FText& NewText, ETextCommit::Type CommitType, TSharedPtr<FTireflyMetaDataEditorRow> RowItem)
{
	if (!RowItem.IsValid() || !OnSetMetaData.IsBound())
	{
		return;
	}

	OnSetMetaData.Execute(RowItem->Key, NewText.ToString());
	RefreshRows();
}

FReply STireflyMetaDataEditor::OnRemoveClicked(TSharedPtr<FTireflyMetaDataEditorRow> RowItem)
{
	if (!RowItem.IsValid() || !OnRemoveMetaData.IsBound())
	{
		return FReply::Handled();
	}

	if (IsEngineManagedKey(RowItem->Key))
	{
		const EAppReturnType::Type Choice = FMessageDialog::Open(
			EAppMsgType::YesNo,
			FText::Format(LOCTEXT("RemoveEngineManagedKey", "'{0}' is a common engine-managed MetaData key. Remove it?"), FText::FromName(RowItem->Key)));
		if (Choice != EAppReturnType::Yes)
		{
			return FReply::Handled();
		}
	}

	OnRemoveMetaData.Execute(RowItem->Key);
	RefreshRows();
	return FReply::Handled();
}

bool STireflyMetaDataEditor::CanCommitKey(FName OldKey, FName NewKey, FText& OutErrorText) const
{
	if (NewKey.IsNone())
	{
		OutErrorText = LOCTEXT("EmptyMetaDataKey", "MetaData name cannot be empty.");
		return false;
	}

	if (OldKey == NewKey)
	{
		return true;
	}

	if (OnGetMetaDataMap.IsBound() && OnGetMetaDataMap.Execute().Contains(NewKey))
	{
		OutErrorText = FText::Format(LOCTEXT("DuplicateMetaDataKey", "MetaData name '{0}' already exists."), FText::FromName(NewKey));
		return false;
	}

	return true;
}

void STireflyMetaDataEditor::AddKey(FName Key)
{
	if (!OnGetMetaDataMap.IsBound() || !OnSetMetaData.IsBound())
	{
		return;
	}

	TMap<FName, FString> MetaDataMap = OnGetMetaDataMap.Execute();
	FName FinalKey = Key;
	if (FinalKey.IsNone())
	{
		FinalKey = TEXT("NewMetaData");
	}

	if (MetaDataMap.Contains(FinalKey))
	{
		const FString BaseName = FinalKey.ToString();
		int32 Index = 1;
		do
		{
			FinalKey = FName(*FString::Printf(TEXT("%s_%d"), *BaseName, Index++));
		}
		while (MetaDataMap.Contains(FinalKey));
	}

	OnSetMetaData.Execute(FinalKey, FString());
	RefreshRows();
}

bool STireflyMetaDataEditor::IsEngineManagedKey(FName Key)
{
	static const TSet<FName> EngineManagedKeys =
	{
		TEXT("Category"),
		TEXT("ToolTip"),
		TEXT("DisplayName"),
		TEXT("ClampMin"),
		TEXT("ClampMax"),
		TEXT("UIMin"),
		TEXT("UIMax")
	};

	return EngineManagedKeys.Contains(Key);
}

#undef LOCTEXT_NAMESPACE
