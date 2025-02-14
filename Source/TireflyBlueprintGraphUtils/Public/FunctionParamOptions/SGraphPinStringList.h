// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SStringComboBox.h"
#include "SGraphPin.h"


class FString;
class SWidget;
class UEdGraphPin;


class TIREFLYBLUEPRINTGRAPHUTILS_API SGraphPinStringList : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SGraphPinStringList) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj, const TArray<TSharedPtr<FString>>& InStringList);

protected:

	/**
	 *	Function to create class specific widget.
	 *
	 *	@return Reference to the newly created widget object
	 */
	virtual TSharedRef<SWidget>	GetDefaultValueWidget() override;

	/**
	 *	Function to set the newly selected index
	 *
	 * @param NewSelection The newly selected item in the combo box
	 * @param SelectInfo Provides context on how the selection changed
	 */
	void ComboBoxSelectionChanged(TSharedPtr<FString> StringItem, ESelectInfo::Type SelectInfo);

	TSharedPtr<class SStringComboBox>	ComboBox;

	/** The actual list of FString values to choose from */
	TArray<TSharedPtr<FString>> StringList;
};
