// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"


class UUserDefinedStruct;
class IAssetEditorInstance;
class FAssetEditorToolkit;


class FTireflyStructMetaDataEditorManager
{
public:
	FTireflyStructMetaDataEditorManager() = default;
	~FTireflyStructMetaDataEditorManager() = default;

	// 注册工具栏扩展，监听资产打开事件。
	void Register();

	// 注销工具栏扩展，解绑委托。
	void Unregister();

private:
	// 资产打开回调。
	void OnAssetOpenedInEditor(UObject* Asset, IAssetEditorInstance* EditorInstance);

	// 为结构体编辑器添加工具栏扩展。
	void AddToolbarExtenderForEditor(FAssetEditorToolkit* Toolkit, UUserDefinedStruct* Struct);

	// 打开或激活结构体自身 MetaData 窗口。
	void OpenStructMetaDataWindow(UUserDefinedStruct* Struct);

	// 打开或激活结构体成员变量 MetaData 窗口。
	void OpenMemberMetaDataWindow(UUserDefinedStruct* Struct);

private:
	// 资产打开事件委托句柄。
	FDelegateHandle AssetOpenedHandle;

	// 已添加扩展的编辑器集合，避免重复添加。
	TSet<const IAssetEditorInstance*> ExtendedEditors;

	// 结构体自身 MetaData 窗口映射，避免重复打开。
	TMap<UUserDefinedStruct*, TWeakPtr<class SWindow>> StructMetaDataWindows;

	// 结构体成员变量 MetaData 窗口映射，避免重复打开。
	TMap<UUserDefinedStruct*, TWeakPtr<class SWindow>> MemberMetaDataWindows;
};
