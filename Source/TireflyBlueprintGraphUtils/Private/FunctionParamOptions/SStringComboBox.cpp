// Copyright Tirefly. All Rights Reserved.


#include "FunctionParamOptions/SStringComboBox.h"

#include "SlateOptMacros.h"
#include "InputCoreTypes.h"


void SStringComboBox::Construct( const FArguments& InArgs )
{
	SelectionChanged = InArgs._OnSelectionChanged;
	GetTextLabelForItem = InArgs._OnGetStringLabelForItem;
	Font = InArgs._Font;

	// Then make widget
	this->ChildSlot
	[
		SAssignNew(StringCombo, SComboBox< TSharedPtr<FString> > )
		.ComboBoxStyle(InArgs._ComboBoxStyle)
		.OptionsSource(InArgs._OptionsSource)
		.OnGenerateWidget(this, &SStringComboBox::MakeItemWidget)
		.OnSelectionChanged(this, &SStringComboBox::OnSelectionChanged)
		.OnComboBoxOpening(InArgs._OnComboBoxOpening)
		.InitiallySelectedItem(InArgs._InitiallySelectedItem)
		.ContentPadding(InArgs._ContentPadding)
		[
			SNew(STextBlock)
				.ColorAndOpacity(InArgs._ColorAndOpacity)
				.Text(this, &SStringComboBox::GetSelectedStringLabel)
				.Font(InArgs._Font)
		]
	];
	SelectedItem = StringCombo->GetSelectedItem();
}

FText SStringComboBox::GetItemStringLabel(TSharedPtr<FString> StringItem) const
{
	if (!StringItem.IsValid())
	{
		return FText::GetEmpty();
	}

	return (GetTextLabelForItem.IsBound())
		? FText::FromString(GetTextLabelForItem.Execute(StringItem))
		: FText::FromString(*StringItem);
}

FText SStringComboBox::GetSelectedStringLabel() const
{
	TSharedPtr<FString> StringItem = StringCombo->GetSelectedItem();
	return GetItemStringLabel(StringItem);
}

TSharedRef<SWidget> SStringComboBox::MakeItemWidget( TSharedPtr<FString> StringItem ) 
{
	check( StringItem.IsValid() );

	return SNew(STextBlock)
		.Text(this, &SStringComboBox::GetItemStringLabel, StringItem)
		.Font(Font);
}

void SStringComboBox::OnSelectionChanged (TSharedPtr<FString> Selection, ESelectInfo::Type SelectInfo)
{
	if (Selection.IsValid())
	{
		SelectedItem = Selection;
	}
	SelectionChanged.ExecuteIfBound(Selection, SelectInfo);
}

void SStringComboBox::SetSelectedItem(TSharedPtr<FString> NewSelection)
{
	StringCombo->SetSelectedItem(NewSelection);
}

void SStringComboBox::RefreshOptions()
{
	StringCombo->RefreshOptions();
}

void SStringComboBox::ClearSelection( )
{
	StringCombo->ClearSelection();
}
