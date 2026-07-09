// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"


class UUserDefinedStruct;
struct FStructVariableDescription;


/**
 * 用户定义结构体成员变量 MetaData 编辑窗口控件。
 * 按成员变量分组，每个成员一个可折叠的 STireflyMetaDataEditor。
 */
class STireflyStructMemberMetaDataEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STireflyStructMemberMetaDataEditor) {}
		// 目标结构体资产。
		SLATE_ARGUMENT(TWeakObjectPtr<UUserDefinedStruct>, Struct)
	SLATE_END_ARGS()

	// 构造控件。
	void Construct(const FArguments& InArgs);

private:
	// 获取指定成员变量的 MetaData 映射。
	TMap<FName, FString> GetMemberMetaDataMap(FGuid VarGuid) const;

	// 设置指定成员变量的 MetaData。
	void SetMemberMetaData(FGuid VarGuid, FName Key, const FString& Value);

	// 删除指定成员变量的 MetaData。
	void RemoveMemberMetaData(FGuid VarGuid, FName Key);

	// 重命名指定成员变量的 MetaData。
	void RenameMemberMetaData(FGuid VarGuid, FName OldKey, FName NewKey);

private:
	// 目标结构体资产。
	TWeakObjectPtr<UUserDefinedStruct> Struct;
};
