// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"


class IBlueprintEditor;
class IDetailLayoutBuilder;
class UBlueprint;


// 蓝图变量 MetaData Details 扩展。
class FTireflyBlueprintVariableMetaDataDetails : public IDetailCustomization
{
public:
	// 创建 Details 扩展实例。
	static TSharedPtr<IDetailCustomization> MakeInstance(TSharedPtr<IBlueprintEditor> BlueprintEditor);

public:
	// 构造函数。
	explicit FTireflyBlueprintVariableMetaDataDetails(TWeakPtr<IBlueprintEditor> InBlueprintEditor);

public:
	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	// 尝试初始化当前变量编辑目标。
	bool TryInitializeTarget(IDetailLayoutBuilder& DetailBuilder);

	// 获取当前变量 MetaData。
	TMap<FName, FString> GetMetaDataMap() const;

	// 设置当前变量 MetaData。
	void SetMetaData(FName Key, const FString& Value);

	// 删除当前变量 MetaData。
	void RemoveMetaData(FName Key);

	// 重命名当前变量 MetaData。
	void RenameMetaData(FName OldKey, FName NewKey);

private:
	// 当前蓝图编辑器。
	TWeakPtr<IBlueprintEditor> BlueprintEditor;

	// 当前蓝图。
	TWeakObjectPtr<UBlueprint> Blueprint;

	// 当前变量名。
	FName VariableName = NAME_None;
};
