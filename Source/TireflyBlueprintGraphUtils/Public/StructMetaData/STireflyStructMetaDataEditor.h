// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"


class UUserDefinedStruct;


/**
 * 用户定义结构体自身 MetaData 编辑窗口控件。
 * 编辑结构体 UObject 级别的 MetaData。
 */
class STireflyStructMetaDataEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STireflyStructMetaDataEditor) {}
		// 目标结构体资产。
		SLATE_ARGUMENT(TWeakObjectPtr<UUserDefinedStruct>, Struct)
	SLATE_END_ARGS()

	// 构造控件。
	void Construct(const FArguments& InArgs);

private:
	// 获取结构体 MetaData 映射。
	TMap<FName, FString> GetMetaDataMap() const;

	// 设置结构体 MetaData。
	void SetMetaData(FName Key, const FString& Value);

	// 删除结构体 MetaData。
	void RemoveMetaData(FName Key);

	// 重命名结构体 MetaData。
	void RenameMetaData(FName OldKey, FName NewKey);

private:
	// 目标结构体资产。
	TWeakObjectPtr<UUserDefinedStruct> Struct;
};
