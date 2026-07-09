// Copyright Tirefly. All Rights Reserved.

#include "MetaDataEditor/TireflyBlueprintFunctionMetaDataDetails.h"

#include "BlueprintEditor.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/Blueprint.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_EditablePinBase.h"
#include "K2Node_FunctionEntry.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "MetaDataEditor/STireflyMetaDataEditor.h"
#include "ParamMetaData/TireflyBlueprintParamMetaDataExtension.h"
#include "ScopedTransaction.h"
#include "Styling/AppStyle.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Text/STextBlock.h"


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

	// 函数级 MetaData 分组（普通函数用 FKismetUserDeclaredFunctionMetadata，自定义事件用 sidecar）
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

	// 参数级 MetaData 分组
	if (FunctionEntry.IsValid())
	{
		const UK2Node_EditablePinBase* EntryNode = FunctionEntry.Get();
		TArray<FName> InputParamNames;

		for (const TSharedPtr<FUserPinInfo>& PinInfo : EntryNode->UserDefinedPins)
		{
			if (PinInfo.IsValid() && PinInfo->DesiredPinDirection == EGPD_Output)
			{
				InputParamNames.Add(PinInfo->PinName);
			}
		}

		if (InputParamNames.Num() > 0)
		{
			IDetailCategoryBuilder& ParamCategory = DetailBuilder.EditCategory(
				"ParametersMetaData", LOCTEXT("ParamMetaDataCategory", "Parameters Meta Data"));

			for (const FName& ParamName : InputParamNames)
			{
				ParamCategory.AddCustomRow(FText::FromName(ParamName))
				.WholeRowContent()
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
							.Text(FText::FromName(ParamName))
							.Font(IDetailLayoutBuilder::GetDetailFontBold())
						]
					]
					.BodyContent()
					[
						SNew(SBox)
						.Padding(FMargin(4.f, 2.f))
						[
							SNew(STireflyMetaDataEditor)
							.OnGetMetaDataMap(FTireflyGetMetaDataMap::CreateLambda(
								[this, ParamName]() { return GetParamMetaDataMap(ParamName); }))
							.OnSetMetaData(FTireflySetMetaData::CreateLambda(
								[this, ParamName](FName Key, const FString& Value) { SetParamMetaData(ParamName, Key, Value); }))
							.OnRemoveMetaData(FTireflyRemoveMetaData::CreateLambda(
								[this, ParamName](FName Key) { RemoveParamMetaData(ParamName, Key); }))
							.OnRenameMetaData(FTireflyRenameMetaData::CreateLambda(
								[this, ParamName](FName OldKey, FName NewKey) { RenameParamMetaData(ParamName, OldKey, NewKey); }))
							.CommonKeys(TArray<FName>
							{
								TEXT("GetParamOptions")
							})
							.KeyDescriptions(FTireflyMetaDataKeyDescriptions
							{
								{ TEXT("GetParamOptions"), LOCTEXT("GetParamOptionsDesc", "Provides a dropdown list of options for this parameter pin. Value is a function path returning TArray<FString>.") }
							})
						]
					]
				];
			}
		}
	}
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
		UObject* Obj = SelectedObject.Get();
		if (!Obj)
		{
			continue;
		}

		// 分支1：选中 UEdGraph（函数图或事件图）
		if (UEdGraph* Graph = Cast<UEdGraph>(Obj))
		{
			FunctionGraph = Graph;

			// 优先用引擎的 GetEntryNode
			FunctionEntry = FBlueprintEditorUtils::GetEntryNode(Graph);

			// fallback：GetEntryNode 可能因 IsEditable 返回 false 而返回 null
			if (!FunctionEntry.IsValid())
			{
				TArray<UK2Node_FunctionEntry*> EntryNodes;
				Graph->GetNodesOfClass(EntryNodes);
				if (EntryNodes.Num() > 0)
				{
					FunctionEntry = EntryNodes[0];
				}
			}

			// 如果仍然没找到，尝试找 CustomEvent 节点
			if (!FunctionEntry.IsValid())
			{
				TArray<UK2Node_CustomEvent*> CustomEventNodes;
				Graph->GetNodesOfClass(CustomEventNodes);
				if (CustomEventNodes.Num() > 0)
				{
					FunctionEntry = CustomEventNodes[0];
				}
			}

			return FunctionEntry.IsValid();
		}

		// 分支2：选中 UK2Node_FunctionEntry
		if (const UK2Node_FunctionEntry* FunctionEntryNode = Cast<UK2Node_FunctionEntry>(Obj))
		{
			FunctionGraph = FunctionEntryNode->GetGraph();
			FunctionEntry = const_cast<UK2Node_FunctionEntry*>(FunctionEntryNode);
			return true;
		}

		// 分支3：选中 UK2Node_CustomEvent
		if (const UK2Node_CustomEvent* CustomEventNode = Cast<UK2Node_CustomEvent>(Obj))
		{
			FunctionGraph = CustomEventNode->GetGraph();
			FunctionEntry = const_cast<UK2Node_CustomEvent*>(CustomEventNode);
			return true;
		}
	}

	return false;
}

FName FTireflyBlueprintFunctionMetaDataDetails::GetFunctionName() const
{
	// Custom Event：使用 CustomFunctionName（编译后的 UFunction 名）
	if (FunctionEntry.IsValid())
	{
		if (const UK2Node_CustomEvent* CustomEvent = Cast<UK2Node_CustomEvent>(FunctionEntry.Get()))
		{
			return CustomEvent->CustomFunctionName;
		}
	}

	// 普通函数：使用函数图名（与 UFunction 名一致）
	if (FunctionGraph.IsValid())
	{
		return FunctionGraph->GetFName();
	}
	return NAME_None;
}

TMap<FName, FString> FTireflyBlueprintFunctionMetaDataDetails::GetMetaDataMap() const
{
	TMap<FName, FString> Result;
	if (const FKismetUserDeclaredFunctionMetadata* MetaData = GetFunctionMetaData())
	{
		Result = MetaData->GetMetaDataMap();
	}
	else if (Blueprint.IsValid())
	{
		// Custom Event：从 sidecar 读取
		const UTireflyBlueprintParamMetaDataExtension* Extension =
			UTireflyBlueprintParamMetaDataExtension::Find(Blueprint.Get());
		if (Extension)
		{
			const FName FuncName = GetFunctionName();
			if (const FTireflyFunctionParamMetaData* FuncMeta = Extension->FindFunction(FuncName))
			{
				Result = FuncMeta->FunctionMetaData;
			}
		}
	}
	return Result;
}

void FTireflyBlueprintFunctionMetaDataDetails::SetMetaData(FName Key, const FString& Value)
{
	if (!Blueprint.IsValid() || Key.IsNone())
	{
		return;
	}

	if (FKismetUserDeclaredFunctionMetadata* MetaData = GetFunctionMetaData())
	{
		const FScopedTransaction Transaction(LOCTEXT("SetFunctionMetaData", "Set Blueprint Function MetaData"));
		Blueprint->Modify();
		FBlueprintEditorUtils::ModifyFunctionMetaData(FunctionGraph.Get());
		MetaData->SetMetaData(Key, FString(Value));
		FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint.Get());
	}
	else
	{
		// Custom Event：写入 sidecar
		const FScopedTransaction Transaction(LOCTEXT("SetFunctionMetaData", "Set Blueprint Function MetaData"));
		UTireflyBlueprintParamMetaDataExtension* Extension =
			UTireflyBlueprintParamMetaDataExtension::GetOrCreate(Blueprint.Get());
		if (!Extension)
		{
			return;
		}
		Blueprint->Modify();
		Extension->Modify();
		FTireflyFunctionParamMetaData* FuncMeta = Extension->FindOrAddFunction(GetFunctionName());
		FuncMeta->FunctionMetaData.Add(Key, Value);
		FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint.Get());
	}
}

void FTireflyBlueprintFunctionMetaDataDetails::RemoveMetaData(FName Key)
{
	if (!Blueprint.IsValid() || Key.IsNone())
	{
		return;
	}

	if (FKismetUserDeclaredFunctionMetadata* MetaData = GetFunctionMetaData())
	{
		const FScopedTransaction Transaction(LOCTEXT("RemoveFunctionMetaData", "Remove Blueprint Function MetaData"));
		Blueprint->Modify();
		FBlueprintEditorUtils::ModifyFunctionMetaData(FunctionGraph.Get());
		MetaData->RemoveMetaData(Key);
		FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint.Get());
	}
	else
	{
		// Custom Event：从 sidecar 删除
		const FScopedTransaction Transaction(LOCTEXT("RemoveFunctionMetaData", "Remove Blueprint Function MetaData"));
		UTireflyBlueprintParamMetaDataExtension* Extension =
			UTireflyBlueprintParamMetaDataExtension::GetOrCreate(Blueprint.Get());
		if (!Extension)
		{
			return;
		}
		Blueprint->Modify();
		Extension->Modify();
		FTireflyFunctionParamMetaData* FuncMeta = Extension->FindOrAddFunction(GetFunctionName());
		FuncMeta->FunctionMetaData.Remove(Key);
		FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint.Get());
	}
}

void FTireflyBlueprintFunctionMetaDataDetails::RenameMetaData(FName OldKey, FName NewKey)
{
	if (!Blueprint.IsValid() || OldKey.IsNone() || NewKey.IsNone() || OldKey == NewKey)
	{
		return;
	}

	if (FKismetUserDeclaredFunctionMetadata* MetaData = GetFunctionMetaData())
	{
		if (!MetaData->HasMetaData(OldKey))
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
	else
	{
		// Custom Event：从 sidecar 读取并重命名
		const UTireflyBlueprintParamMetaDataExtension* ConstExtension =
			UTireflyBlueprintParamMetaDataExtension::Find(Blueprint.Get());
		if (!ConstExtension)
		{
			return;
		}
		const FName FuncName = GetFunctionName();
		const FTireflyFunctionParamMetaData* ConstFuncMeta = ConstExtension->FindFunction(FuncName);
		if (!ConstFuncMeta)
		{
			return;
		}
		const FString* Found = ConstFuncMeta->FunctionMetaData.Find(OldKey);
		if (!Found)
		{
			return;
		}
		const FString ExistingValue = *Found;

		const FScopedTransaction Transaction(LOCTEXT("RenameFunctionMetaData", "Rename Blueprint Function MetaData"));
		UTireflyBlueprintParamMetaDataExtension* Extension =
			UTireflyBlueprintParamMetaDataExtension::GetOrCreate(Blueprint.Get());
		FTireflyFunctionParamMetaData* FuncMeta = Extension->FindOrAddFunction(FuncName);
		FuncMeta->FunctionMetaData.Remove(OldKey);
		FuncMeta->FunctionMetaData.Add(NewKey, ExistingValue);
		Blueprint->Modify();
		Extension->Modify();
		FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint.Get());
	}
}

FKismetUserDeclaredFunctionMetadata* FTireflyBlueprintFunctionMetaDataDetails::GetFunctionMetaData() const
{
	return FunctionGraph.IsValid()
		? FBlueprintEditorUtils::GetGraphFunctionMetaData(FunctionGraph.Get())
		: nullptr;
}

TMap<FName, FString> FTireflyBlueprintFunctionMetaDataDetails::GetParamMetaDataMap(FName ParamName) const
{
	TMap<FName, FString> Result;

	if (!Blueprint.IsValid() || ParamName.IsNone())
	{
		return Result;
	}

	const UTireflyBlueprintParamMetaDataExtension* Extension =
		UTireflyBlueprintParamMetaDataExtension::Find(Blueprint.Get());
	if (!Extension)
	{
		return Result;
	}

	const FName FuncName = GetFunctionName();
	const FTireflyFunctionParamMetaData* FuncMeta = Extension->FindFunction(FuncName);
	if (!FuncMeta)
	{
		return Result;
	}

	for (const FTireflyParamMetaDataEntry& ParamEntry : FuncMeta->ParamMetaData)
	{
		if (ParamEntry.ParamName == ParamName)
		{
			Result = ParamEntry.MetaData;
			break;
		}
	}

	return Result;
}

void FTireflyBlueprintFunctionMetaDataDetails::SetParamMetaData(FName ParamName, FName Key, const FString& Value)
{
	if (!Blueprint.IsValid() || ParamName.IsNone() || Key.IsNone())
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("SetParamMetaData", "Set Parameter MetaData"));
	UTireflyBlueprintParamMetaDataExtension* Extension =
		UTireflyBlueprintParamMetaDataExtension::GetOrCreate(Blueprint.Get());
	if (!Extension)
	{
		return;
	}

	Blueprint->Modify();
	Extension->Modify();

	const FName FuncName = GetFunctionName();
	FTireflyFunctionParamMetaData* FuncMeta = Extension->FindOrAddFunction(FuncName);
	FTireflyParamMetaDataEntry* ParamEntry = Extension->FindOrAddParam(*FuncMeta, ParamName);
	ParamEntry->MetaData.Add(Key, Value);

	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint.Get());
}

void FTireflyBlueprintFunctionMetaDataDetails::RemoveParamMetaData(FName ParamName, FName Key)
{
	if (!Blueprint.IsValid() || ParamName.IsNone() || Key.IsNone())
	{
		return;
	}

	const UTireflyBlueprintParamMetaDataExtension* ConstExtension =
		UTireflyBlueprintParamMetaDataExtension::Find(Blueprint.Get());
	if (!ConstExtension)
	{
		return;
	}

	const FName FuncName = GetFunctionName();
	const FTireflyFunctionParamMetaData* ConstFuncMeta = ConstExtension->FindFunction(FuncName);
	if (!ConstFuncMeta)
	{
		return;
	}

	bool bFound = false;
	for (const FTireflyParamMetaDataEntry& ParamEntry : ConstFuncMeta->ParamMetaData)
	{
		if (ParamEntry.ParamName == ParamName && ParamEntry.MetaData.Contains(Key))
		{
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("RemoveParamMetaData", "Remove Parameter MetaData"));
	UTireflyBlueprintParamMetaDataExtension* Extension =
		UTireflyBlueprintParamMetaDataExtension::GetOrCreate(Blueprint.Get());
	FTireflyFunctionParamMetaData* FuncMeta = Extension->FindOrAddFunction(FuncName);
	FTireflyParamMetaDataEntry* ParamEntry = Extension->FindOrAddParam(*FuncMeta, ParamName);

	ParamEntry->MetaData.Remove(Key);

	Blueprint->Modify();
	Extension->Modify();
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint.Get());
}

void FTireflyBlueprintFunctionMetaDataDetails::RenameParamMetaData(FName ParamName, FName OldKey, FName NewKey)
{
	if (!Blueprint.IsValid() || ParamName.IsNone() || OldKey.IsNone() || NewKey.IsNone() || OldKey == NewKey)
	{
		return;
	}

	const UTireflyBlueprintParamMetaDataExtension* ConstExtension =
		UTireflyBlueprintParamMetaDataExtension::Find(Blueprint.Get());
	if (!ConstExtension)
	{
		return;
	}

	const FName FuncName = GetFunctionName();
	const FTireflyFunctionParamMetaData* ConstFuncMeta = ConstExtension->FindFunction(FuncName);
	if (!ConstFuncMeta)
	{
		return;
	}

	FString ExistingValue;
	bool bFound = false;
	for (const FTireflyParamMetaDataEntry& ParamEntry : ConstFuncMeta->ParamMetaData)
	{
		if (ParamEntry.ParamName == ParamName)
		{
			const FString* Found = ParamEntry.MetaData.Find(OldKey);
			if (Found)
			{
				ExistingValue = *Found;
				bFound = true;
			}
			break;
		}
	}

	if (!bFound)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("RenameParamMetaData", "Rename Parameter MetaData"));
	UTireflyBlueprintParamMetaDataExtension* Extension =
		UTireflyBlueprintParamMetaDataExtension::GetOrCreate(Blueprint.Get());
	FTireflyFunctionParamMetaData* FuncMeta = Extension->FindOrAddFunction(FuncName);
	FTireflyParamMetaDataEntry* ParamEntry = Extension->FindOrAddParam(*FuncMeta, ParamName);

	ParamEntry->MetaData.Remove(OldKey);
	ParamEntry->MetaData.Add(NewKey, MoveTemp(ExistingValue));

	Blueprint->Modify();
	Extension->Modify();
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint.Get());
}

#undef LOCTEXT_NAMESPACE
