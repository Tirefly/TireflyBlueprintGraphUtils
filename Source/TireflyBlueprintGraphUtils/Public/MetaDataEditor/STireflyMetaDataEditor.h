// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"


class SComboButton;
class STableViewBase;
template <typename ItemType> class SListView;


// MetaData 编辑器中的单行数据。
struct FTireflyMetaDataEditorRow
{
	// MetaData 名称。
	FName Key = NAME_None;

	// MetaData 值。
	FString Value;
};


using FTireflyMetaDataMap = TMap<FName, FString>;

// 常用 MetaData Key 的鼠标悬停描述表。
using FTireflyMetaDataKeyDescriptions = TMap<FName, FText>;

DECLARE_DELEGATE_RetVal(FTireflyMetaDataMap, FTireflyGetMetaDataMap);
DECLARE_DELEGATE_TwoParams(FTireflySetMetaData, FName /*Key*/, const FString& /*Value*/);
DECLARE_DELEGATE_OneParam(FTireflyRemoveMetaData, FName /*Key*/);
DECLARE_DELEGATE_TwoParams(FTireflyRenameMetaData, FName /*OldKey*/, FName /*NewKey*/);


// 类 TMap 的 MetaData Key/Value 编辑控件。
class STireflyMetaDataEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STireflyMetaDataEditor) {}
		// 读取当前 MetaData 映射。
		SLATE_EVENT(FTireflyGetMetaDataMap, OnGetMetaDataMap)

		// 设置指定 Key 的值。
		SLATE_EVENT(FTireflySetMetaData, OnSetMetaData)

		// 删除指定 Key。
		SLATE_EVENT(FTireflyRemoveMetaData, OnRemoveMetaData)

		// 重命名指定 Key。
		SLATE_EVENT(FTireflyRenameMetaData, OnRenameMetaData)

		// 常用 MetaData Key 建议列表。
		SLATE_ARGUMENT(TArray<FName>, CommonKeys)

		// 常用 MetaData Key 的鼠标悬停描述。
		SLATE_ARGUMENT(FTireflyMetaDataKeyDescriptions, KeyDescriptions)
	SLATE_END_ARGS()

public:
	// 构造控件。
	void Construct(const FArguments& InArgs);

private:
	// 刷新行数据。
	void RefreshRows();

	// 刷新可用常用 Key 建议列表。
	void RefreshCommonKeyOptions();

	// 生成表格行。
	TSharedRef<ITableRow> MakeRowWidget(TSharedPtr<FTireflyMetaDataEditorRow> RowItem, const TSharedRef<STableViewBase>& OwnerTable);

	// 生成常用 Key 下拉项。
	TSharedRef<SWidget> MakeCommonKeyWidget(TSharedPtr<FName> Item) const;

	// 添加一个默认 MetaData 项。
	FReply OnAddClicked();

	// 常用 Key 选择回调。
	void OnCommonKeySelected(TSharedPtr<FName> SelectedItem, ESelectInfo::Type SelectInfo);

	// 提交 Key 修改。
	void OnKeyCommitted(const FText& NewText, ETextCommit::Type CommitType, TSharedPtr<FTireflyMetaDataEditorRow> RowItem);

	// 提交 Value 修改。
	void OnValueCommitted(const FText& NewText, ETextCommit::Type CommitType, TSharedPtr<FTireflyMetaDataEditorRow> RowItem);

	// 删除指定行。
	FReply OnRemoveClicked(TSharedPtr<FTireflyMetaDataEditorRow> RowItem);

	// Key 是否可提交。
	bool CanCommitKey(FName OldKey, FName NewKey, FText& OutErrorText) const;

	// 添加指定 Key。
	void AddKey(FName Key);

	// 是否是引擎常用 MetaData Key。
	static bool IsEngineManagedKey(FName Key);

private:
	// 读取当前 MetaData 映射。
	FTireflyGetMetaDataMap OnGetMetaDataMap;

	// 设置指定 Key 的值。
	FTireflySetMetaData OnSetMetaData;

	// 删除指定 Key。
	FTireflyRemoveMetaData OnRemoveMetaData;

	// 重命名指定 Key。
	FTireflyRenameMetaData OnRenameMetaData;

	// 表格行数据。
	TArray<TSharedPtr<FTireflyMetaDataEditorRow>> Rows;

	// 常用 Key 建议列表。
	TArray<FName> CommonKeys;

	// 常用 Key 的鼠标悬停描述。
	FTireflyMetaDataKeyDescriptions KeyDescriptions;

	// 当前可用常用 Key 建议列表。
	TArray<TSharedPtr<FName>> AvailableCommonKeyOptions;

	// 常用 Key 下拉框。
	TSharedPtr<SComboBox<TSharedPtr<FName>>> CommonKeyComboBox;

	// 表格控件。
	TSharedPtr<SListView<TSharedPtr<FTireflyMetaDataEditorRow>>> RowListView;
};
