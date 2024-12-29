// Copyright Tirefly. All Rights Reserved.


#include "FunctionParamOptions/TireflyParamOptionPinFactory.h"

#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "SGraphPinNameList.h"
#include "FunctionParamOptions/SGraphPinStringList.h"


TSharedPtr<SGraphPin> FTireflyParamOptionPinFactory::CreatePin(UEdGraphPin* InPin) const
{
    // 检查节点引脚类型是否为名称类型或字符类型
	if (InPin->PinType.PinCategory != UEdGraphSchema_K2::PC_Name &&
		InPin->PinType.PinCategory != UEdGraphSchema_K2::PC_String)
	{
		return nullptr;
	}

    // 尝试将节点引脚的外部对象转换为函数调用节点
	UK2Node_CallFunction* CallFunctionNode = Cast<UK2Node_CallFunction>(InPin->GetOuter());
	if (!CallFunctionNode)
	{
		return nullptr;
	}

    // 从函数调用节点的元数据中获取参数选项的函数名
	FString PinOptionFunctionName = CallFunctionNode->GetPinMetaData(InPin->PinName, FName("GetParamOptions"));
	if (PinOptionFunctionName.IsEmpty())
	{
		return nullptr;
	}

	UObject* OwningObject = nullptr;
	UFunction* FuncToExec = nullptr;
	
    // 处理具有命名空间的函数名
	if (PinOptionFunctionName.Contains(TEXT(".")))
	{
		FuncToExec = FindObject<UFunction>(nullptr, *PinOptionFunctionName, true);
		OwningObject = FuncToExec ? FuncToExec->GetOuter() : nullptr;
	}
	else
    // 处理蓝图定义的默认对象
	{
		if (const UBlueprint* OwningBlueprint = CallFunctionNode->GetBlueprint())
		{
			if (OwningBlueprint->GeneratedClass)
			{
				OwningObject = OwningBlueprint->GeneratedClass->GetDefaultObject();
				FuncToExec = OwningObject->FindFunction(FName(*PinOptionFunctionName));
			}
		}
	}
	
	if (!OwningObject || !FuncToExec)
	{
		return nullptr;
	}

	// 处理名称类型的函数返回
    if (InPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Name)
    {
    	// 执行函数，并处理返回的选项名
    	TArray<TSharedPtr<FName>> OutOptions;
    	TArray<FName> OptionNames;
    	OwningObject->ProcessEvent(FuncToExec, &OptionNames);
    	for (const FName& OptionName : OptionNames)
    	{
    		OutOptions.Add(MakeShareable(new FName(OptionName)));
    	}
	
    	// 创建并返回一个新的节点引脚，用于显示选项列表
    	return SNew(SGraphPinNameList, InPin, OutOptions);
    }

	// 处理字符类型的函数返回
	if (InPin->PinType.PinCategory == UEdGraphSchema_K2::PC_String)
	{
		// 执行函数，并处理返回的选项名
		TArray<TSharedPtr<FString>> OutOptions;
		TArray<FString> OptionNames;
		OwningObject->ProcessEvent(FuncToExec, &OptionNames);
		for (const FString& OptionName : OptionNames)
		{
			OutOptions.Add(MakeShareable(new FString(OptionName)));
		}
	
		// 创建并返回一个新的节点引脚，用于显示选项列表
		return SNew(SGraphPinStringList, InPin, OutOptions);
	}

	return nullptr;
}
