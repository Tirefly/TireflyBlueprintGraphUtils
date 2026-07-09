// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"


class IBlueprintEditor;
class IDetailLayoutBuilder;
class UBlueprint;
class UEdGraph;
class UK2Node_EditablePinBase;
struct FKismetUserDeclaredFunctionMetadata;
struct FUserPinInfo;


// 蓝图函数 MetaData Details 扩展。
class FTireflyBlueprintFunctionMetaDataDetails : public IDetailCustomization
{
public:
	// 创建 Details 扩展实例。
	static TSharedPtr<IDetailCustomization> MakeInstance(TSharedPtr<IBlueprintEditor> BlueprintEditor);

public:
	// 构造函数。
	explicit FTireflyBlueprintFunctionMetaDataDetails(TWeakPtr<IBlueprintEditor> InBlueprintEditor);

public:
	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	// 尝试初始化当前函数编辑目标。
	bool TryInitializeTarget(IDetailLayoutBuilder& DetailBuilder);

	// 获取当前函数名。
	FName GetFunctionName() const;

	// 获取当前函数 MetaData。
	TMap<FName, FString> GetMetaDataMap() const;

	// 设置当前函数 MetaData。
	void SetMetaData(FName Key, const FString& Value);

	// 删除当前函数 MetaData。
	void RemoveMetaData(FName Key);

	// 重命名当前函数 MetaData。
	void RenameMetaData(FName OldKey, FName NewKey);

	// 获取当前函数的用户声明 MetaData。
	FKismetUserDeclaredFunctionMetadata* GetFunctionMetaData() const;

private:
	// 获取指定参数的 MetaData 映射。
	TMap<FName, FString> GetParamMetaDataMap(FName ParamName) const;

	// 设置指定参数的 MetaData。
	void SetParamMetaData(FName ParamName, FName Key, const FString& Value);

	// 删除指定参数的 MetaData。
	void RemoveParamMetaData(FName ParamName, FName Key);

	// 重命名指定参数的 MetaData。
	void RenameParamMetaData(FName ParamName, FName OldKey, FName NewKey);

private:
	// 当前蓝图编辑器。
	TWeakPtr<IBlueprintEditor> BlueprintEditor;

	// 当前蓝图。
	TWeakObjectPtr<UBlueprint> Blueprint;

	// 当前函数图（普通函数有效，Custom Event 为其所在事件图）。
	TWeakObjectPtr<UEdGraph> FunctionGraph;

	// 当前函数/事件入口节点。
	TWeakObjectPtr<UK2Node_EditablePinBase> FunctionEntry;
};
