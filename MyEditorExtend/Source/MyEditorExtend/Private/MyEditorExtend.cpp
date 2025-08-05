// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyEditorExtend.h"

#include "ContentBrowserModule.h"
#include "EditorAssetLibrary.h"
#include "NotifyTools.h"
#include "ObjectTools.h"

#define LOCTEXT_NAMESPACE "FMyEditorExtendModule"

void FMyEditorExtendModule::StartupModule()
{
	InitialContentBrowserExtend();
}

void FMyEditorExtendModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FMyEditorExtendModule::InitialContentBrowserExtend()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(
		TEXT("ContentBrowser"));
	//获取内容浏览器菜单扩展器
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentMenuExtend = ContentBrowserModule.
		GetAllPathViewContextMenuExtenders();

	//似乎必须用这个方式，这个东西会把把选择的路径传过来
	ContentMenuExtend.Add(
		FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FMyEditorExtendModule::CreatedAndPlaceExtender));
}

TSharedRef<FExtender> FMyEditorExtendModule::CreatedAndPlaceExtender(const TArray<FString>& SelectedPaths)
{
	//1.创建一个FExtender
	TSharedRef<FExtender> DeleteUnusedExtender(new FExtender);
	if (!SelectedPaths.IsEmpty())
	{
		//2.设置锚点,绑在那个原有按键下面；锚点在EditorPreferences的Display UI Extension Points
		DeleteUnusedExtender->AddMenuExtension(TEXT("Delete"), EExtensionHook::After, TSharedPtr<FUICommandList>(),
		                                       FMenuExtensionDelegate::CreateRaw(
			                                       this, &FMyEditorExtendModule::AddDeleteUnusedEntry));
	}
	CurrentSelectedPaths = SelectedPaths;
	return DeleteUnusedExtender;
}

void FMyEditorExtendModule::AddDeleteUnusedEntry(FMenuBuilder& MenuBuilder)
{
	//实际添加按键
	//FText不能直接从TEXT生成，需要包一层
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Delete Unused")),FText::FromString(TEXT("Delete Unused Assets")), FSlateIcon(),
	                         FExecuteAction::CreateRaw(this, &FMyEditorExtendModule::OnDeleteUnusedButtonClick));
}

void FMyEditorExtendModule::OnDeleteUnusedButtonClick()
{
	//业务逻辑
	if (CurrentSelectedPaths.IsEmpty())
	{
		UNotifyTools::ShowMsgDialog(EAppMsgType::Ok,TEXT("No Folder Was Selected!"));
		return;
	}
	TArray<FAssetData> UnusedAssets;
	for (const FString& SinglePath : CurrentSelectedPaths)
	{
		TArray<FString> AssetsInSelectedFolder = UEditorAssetLibrary::ListAssets(SinglePath);
		if (AssetsInSelectedFolder.IsEmpty())
		{
			continue;
		}
		for (const FString& AssetPath : AssetsInSelectedFolder)
		{
			//这两个路径下的不删除
			if (AssetPath.Contains(TEXT("Developers")) || AssetPath.Contains(TEXT("Collections")))
			{
				continue;
			}
			if (!UEditorAssetLibrary::DoesAssetExist(AssetPath))
			{
				continue;
			}
			TArray<FString> ReferenceSource = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPath);
			if (ReferenceSource.IsEmpty())
			{
				FAssetData TargetAssetData = UEditorAssetLibrary::FindAssetData(AssetPath);
				UnusedAssets.Emplace(TargetAssetData);
			}
		}
	}
	if (UnusedAssets.IsEmpty())
	{
		UNotifyTools::ShowCornerPopupMessage(TEXT("All Assets In Selected Path Has Reference,No Asset Was Delete"));
	}
	else
	{
		ObjectTools::DeleteAssets(UnusedAssets);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMyEditorExtendModule, MyEditorExtend)
