// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"


class IBlueprintEditor;
class IDetailLayoutBuilder;
class UBlueprint;
class UEdGraph;
struct FKismetUserDeclaredFunctionMetadata;


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
	// 当前蓝图编辑器。
	TWeakPtr<IBlueprintEditor> BlueprintEditor;

	// 当前蓝图。
	TWeakObjectPtr<UBlueprint> Blueprint;

	// 当前函数图。
	TWeakObjectPtr<UEdGraph> FunctionGraph;
};
