// Copyright Tirefly. All Rights Reserved.


#include "FunctionParamOptions/SGraphPinStringList.h"

#include "FunctionParamOptions/SStringComboBox.h"


void SGraphPinStringList::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj, const TArray<TSharedPtr<FString>>& InStringList)
{
	StringList = InStringList;
	SGraphPin::Construct(SGraphPin::FArguments(), InGraphPinObj);
}

TSharedRef<SWidget>	SGraphPinStringList::GetDefaultValueWidget()
{
	TSharedPtr<FString> CurrentlySelectedString;

	if (GraphPinObj)
	{
		// Preserve previous selection, or set to first in list
		FString PreviousSelection = FString(*GraphPinObj->GetDefaultAsString());
		for (TSharedPtr<FString> ListStringPtr : StringList)
		{
			if (PreviousSelection == *ListStringPtr.Get())
			{
				CurrentlySelectedString = ListStringPtr;
				break;
			}
		}
	}
	
	// Create widget
	return SAssignNew(ComboBox, SStringComboBox)
		.ContentPadding(FMargin(6.0f, 2.0f))
		.OptionsSource(&StringList)
		.InitiallySelectedItem(CurrentlySelectedString)
		.OnSelectionChanged(this, &SGraphPinStringList::ComboBoxSelectionChanged)
		.Visibility(this, &SGraphPin::GetDefaultValueVisibility)
		;
}

void SGraphPinStringList::ComboBoxSelectionChanged(TSharedPtr<FString> StringItem, ESelectInfo::Type SelectInfo )
{
	FString String = StringItem.IsValid() ? *StringItem : TEXT("None");
	if (auto Schema = (GraphPinObj ? GraphPinObj->GetSchema() : NULL))
	{
		FString StringAsString = String;
		if(GraphPinObj->GetDefaultAsString() != StringAsString)
		{
			const FScopedTransaction Transaction( NSLOCTEXT("GraphEditor", "ChangeStringListPinValue", "Change String List Pin Value" ) );
			GraphPinObj->Modify();

			Schema->TrySetDefaultValue(*GraphPinObj, StringAsString);
		}
	}
}

