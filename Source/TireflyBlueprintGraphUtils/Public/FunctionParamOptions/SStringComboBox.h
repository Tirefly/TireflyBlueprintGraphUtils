// Copyright Tirefly. All Rights Reserved.
 
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SComboBox.h"


class SWidget;


/**
 * A combo box that shows FString content.
 */
class TIREFLYBLUEPRINTGRAPHUTILS_API SStringComboBox : public SCompoundWidget
{
public:

	DECLARE_DELEGATE_RetVal_OneParam(FString, FGetStringComboLabel, TSharedPtr<FString>);
	typedef TSlateDelegates< TSharedPtr<FString> >::FOnSelectionChanged FOnStringSelectionChanged;

	SLATE_BEGIN_ARGS( SStringComboBox ) 
		: _ComboBoxStyle(&FCoreStyle::Get().GetWidgetStyle< FComboBoxStyle >("ComboBox"))
		, _ColorAndOpacity( FSlateColor::UseForeground() )
		, _ContentPadding(FMargin(4.0, 2.0))
		, _OnGetStringLabelForItem()
		{}

		SLATE_STYLE_ARGUMENT(FComboBoxStyle, ComboBoxStyle)

		/** Selection of FStrings to pick from */
		SLATE_ARGUMENT( TArray< TSharedPtr<FString> >*, OptionsSource )

		/** Sets the font used to draw the text */
		SLATE_ATTRIBUTE(FSlateFontInfo, Font)

		/** Text color and opacity */
		SLATE_ATTRIBUTE( FSlateColor, ColorAndOpacity )

		/** Visual padding of the button content for the combobox */
		SLATE_ATTRIBUTE( FMargin, ContentPadding )

		/** Called when the FString is chosen. */
		SLATE_EVENT( FOnStringSelectionChanged, OnSelectionChanged)

		/** Called when the combo box is opened */
		SLATE_EVENT( FOnComboBoxOpening, OnComboBoxOpening )

		/** Called when combo box needs to establish selected item */
		SLATE_ARGUMENT( TSharedPtr<FString>, InitiallySelectedItem )

		/** [Optional] Called to get the label for the currently selected item */
		SLATE_EVENT( FGetStringComboLabel, OnGetStringLabelForItem ) 
	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs );

	/** Called to create a widget for each FString */
	TSharedRef<SWidget> MakeItemWidget( TSharedPtr<FString> StringItem );

	void SetSelectedItem (TSharedPtr<FString> NewSelection);

	/** Returns the currently selected FString */
	TSharedPtr<FString> GetSelectedItem()
	{
		return SelectedItem;
	}

	/** Request to reload the name options in the combobox from the OptionsSource attribute */
	void RefreshOptions();

	/** Clears the selected item in the name combo */
	void ClearSelection();

private:
	TSharedPtr<FString> OnGetSelection() const {return SelectedItem;}

	/** Called when selection changes in the combo pop-up */
	void OnSelectionChanged(TSharedPtr<FString> Selection, ESelectInfo::Type SelectInfo);

	/** Helper method to get the text for a given item in the combo box */
	FText GetSelectedStringLabel() const;

	FText GetItemStringLabel(TSharedPtr<FString> StringItem) const;

private:

	/** Called to get the text label for an item */
	FGetStringComboLabel GetTextLabelForItem;

	/** The FString item selected */
	TSharedPtr<FString> SelectedItem;

	/** Array of shared pointers to FStrings so combo widget can work on them */
	TArray< TSharedPtr<FString> > Strings;

	/** The combo box */
	TSharedPtr< SComboBox< TSharedPtr<FString> > >	StringCombo;

	/** Forwarding Delegate */
	FOnStringSelectionChanged SelectionChanged;

	/** Sets the font used to draw the text */
	TAttribute< FSlateFontInfo > Font;
};
