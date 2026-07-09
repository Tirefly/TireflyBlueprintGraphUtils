// Copyright Tirefly. All Rights Reserved.

#include "StructMetaData/TireflyStructMetaDataEditorManager.h"

#include "Editor.h"
#include "Framework/Commands/UIAction.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/AppStyle.h"
#include "StructMetaData/STireflyStructMemberMetaDataEditor.h"
#include "StructMetaData/STireflyStructMetaDataEditor.h"
#include "StructUtils/UserDefinedStruct.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"


#define LOCTEXT_NAMESPACE "FTireflyStructMetaDataEditorManager"


void FTireflyStructMetaDataEditorManager::Register()
{
	if (GEditor)
	{
		if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
		{
			AssetOpenedHandle = AssetEditorSubsystem->OnAssetOpenedInEditor().AddRaw(
				this, &FTireflyStructMetaDataEditorManager::OnAssetOpenedInEditor);
		}
	}
}

void FTireflyStructMetaDataEditorManager::Unregister()
{
	if (GEditor)
	{
		if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
		{
			if (AssetOpenedHandle.IsValid())
			{
				AssetEditorSubsystem->OnAssetOpenedInEditor().Remove(AssetOpenedHandle);
				AssetOpenedHandle.Reset();
			}
		}
	}

	ExtendedEditors.Empty();
	StructMetaDataWindows.Empty();
	MemberMetaDataWindows.Empty();
}

void FTireflyStructMetaDataEditorManager::OnAssetOpenedInEditor(UObject* Asset, IAssetEditorInstance* EditorInstance)
{
	UUserDefinedStruct* Struct = Cast<UUserDefinedStruct>(Asset);
	if (!Struct || !EditorInstance)
	{
		return;
	}

	// IAssetEditorInstance 不继承 TSharedFromThis，直接用原始指针操作
	// cast 为 FAssetEditorToolkit（IAssetEditorInstance 是它的基接口）
	FAssetEditorToolkit* Toolkit = static_cast<FAssetEditorToolkit*>(EditorInstance);
	if (!Toolkit)
	{
		return;
	}

	// 用 toolkit 原始指针去重
	if (ExtendedEditors.Contains(EditorInstance))
	{
		return;
	}
	ExtendedEditors.Add(EditorInstance);

	AddToolbarExtenderForEditor(Toolkit, Struct);
}

void FTireflyStructMetaDataEditorManager::AddToolbarExtenderForEditor(FAssetEditorToolkit* Toolkit, UUserDefinedStruct* Struct)
{
	TWeakObjectPtr<UUserDefinedStruct> WeakStruct(Struct);

	TSharedPtr<FExtender> Extender = MakeShared<FExtender>();
	Extender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		Toolkit->GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateLambda([this, WeakStruct](FToolBarBuilder& ToolbarBuilder)
		{
			ToolbarBuilder.BeginSection("TireflyStructMetaData");

			// Struct MetaData 按钮
			ToolbarBuilder.AddToolBarButton(
				FUIAction(FExecuteAction::CreateLambda([this, WeakStruct]()
				{
					if (!WeakStruct.IsValid())
					{
						return;
					}

					OpenStructMetaDataWindow(WeakStruct.Get());
				})),
				NAME_None,
				LOCTEXT("StructMetaDataButtonLabel", "Struct MetaData"),
				LOCTEXT("StructMetaDataButtonTooltip", "Edit metadata on the struct itself."),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Edit")
			);

			// Member MetaData 按钮
			ToolbarBuilder.AddToolBarButton(
				FUIAction(FExecuteAction::CreateLambda([this, WeakStruct]()
				{
					if (!WeakStruct.IsValid())
					{
						return;
					}

					OpenMemberMetaDataWindow(WeakStruct.Get());
				})),
				NAME_None,
				LOCTEXT("MemberMetaDataButtonLabel", "Member MetaData"),
				LOCTEXT("MemberMetaDataButtonTooltip", "Edit metadata on individual member variables."),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Edit")
			);

			ToolbarBuilder.EndSection();
		})
	);

	Toolkit->AddToolbarExtender(Extender);
	Toolkit->RegenerateMenusAndToolbars();
}

void FTireflyStructMetaDataEditorManager::OpenStructMetaDataWindow(UUserDefinedStruct* Struct)
{
	if (!Struct)
	{
		return;
	}

	if (TWeakPtr<SWindow>* ExistingWindow = StructMetaDataWindows.Find(Struct))
	{
		if (TSharedPtr<SWindow> Window = ExistingWindow->Pin())
		{
			Window->BringToFront(true);
			Window->ShowWindow();
			return;
		}
	}

	TWeakObjectPtr<UUserDefinedStruct> WeakStruct(Struct);
	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("StructMetaDataWindowTitle", "Struct MetaData"))
		.ClientSize(FVector2D(500, 400))
		.SizingRule(ESizingRule::UserSized)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		[
			SNew(STireflyStructMetaDataEditor)
			.Struct(WeakStruct)
		];

	Window->SetOnWindowClosed(FOnWindowClosed::CreateLambda([this, Struct](const TSharedRef<SWindow>&)
	{
		StructMetaDataWindows.Remove(Struct);
	}));

	StructMetaDataWindows.Add(Struct, Window);
	FSlateApplication::Get().AddWindow(Window);
}

void FTireflyStructMetaDataEditorManager::OpenMemberMetaDataWindow(UUserDefinedStruct* Struct)
{
	if (!Struct)
	{
		return;
	}

	if (TWeakPtr<SWindow>* ExistingWindow = MemberMetaDataWindows.Find(Struct))
	{
		if (TSharedPtr<SWindow> Window = ExistingWindow->Pin())
		{
			Window->BringToFront(true);
			Window->ShowWindow();
			return;
		}
	}

	TWeakObjectPtr<UUserDefinedStruct> WeakStruct(Struct);
	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("MemberMetaDataWindowTitle", "Member MetaData"))
		.ClientSize(FVector2D(500, 500))
		.SizingRule(ESizingRule::UserSized)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.SupportsMinimize(false)
		.SupportsMaximize(false)
		[
			SNew(STireflyStructMemberMetaDataEditor)
			.Struct(WeakStruct)
		];

	Window->SetOnWindowClosed(FOnWindowClosed::CreateLambda([this, Struct](const TSharedRef<SWindow>&)
	{
		MemberMetaDataWindows.Remove(Struct);
	}));

	MemberMetaDataWindows.Add(Struct, Window);
	FSlateApplication::Get().AddWindow(Window);
}

#undef LOCTEXT_NAMESPACE
