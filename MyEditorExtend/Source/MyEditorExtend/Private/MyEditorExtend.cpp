// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyEditorExtend.h"

#include "ContentBrowserModule.h"
#include "EditorAssetLibrary.h"
#include "NotifyTools.h"
#include "ObjectTools.h"
#include "EditorScriptingHelpers.h"
#include "SAdvancedDeleteTab.h"

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
	TSharedRef<FExtender> AssetsCleanExtender(new FExtender);
	if (!SelectedPaths.IsEmpty())
	{
		//2.设置锚点,绑在那个原有按键下面；锚点在EditorPreferences的Display UI Extension Points
		AssetsCleanExtender->AddMenuExtension(TEXT("Delete"), EExtensionHook::After, TSharedPtr<FUICommandList>(),
		                                      FMenuExtensionDelegate::CreateRaw(
			                                      this, &FMyEditorExtendModule::AddExtendButtonEntry));
	}
	CurrentSelectedPaths = SelectedPaths;
	return AssetsCleanExtender;
}

void FMyEditorExtendModule::AddExtendButtonEntry(FMenuBuilder& MenuBuilder)
{
	//实际添加按键
	//FText不能直接从TEXT生成，需要包一层
	MenuBuilder.AddMenuEntry(FText::FromString(TEXT("Delete Unused")), FText::FromString(TEXT("Delete Unused Assets")),
	                         FSlateIcon(),
	                         FExecuteAction::CreateRaw(this, &FMyEditorExtendModule::OnDeleteUnusedButtonClicked));

	MenuBuilder.AddMenuEntry(FText::FromString("Delete Empty Folders"), FText::FromString("Delete Empty Folders"),
	                         FSlateIcon(),
	                         FExecuteAction::CreateRaw(this, &FMyEditorExtendModule::OnDeleteEmptyClicked));

	MenuBuilder.AddMenuEntry(FText::FromString("Open Advanced Delete Window"),
	                         FText::FromString("A Standalone Window For Delete Action"), FSlateIcon(),
	                         FExecuteAction::CreateRaw(this, &FMyEditorExtendModule::OnOpenTabButtonClicked));
	//添加其他按键
}
#pragma region ContentBrowserTreeViewAction
void FMyEditorExtendModule::OnDeleteUnusedButtonClicked()
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
		if (!FPaths::ValidatePath(SinglePath))
		{
			continue;
		}
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

void FMyEditorExtendModule::OnDeleteEmptyClicked()
{
	//@TODO:需要修复重定向
	if (CurrentSelectedPaths.IsEmpty())
	{
		UNotifyTools::ShowMsgDialog(EAppMsgType::Ok,TEXT("No Valid Path Was Selected"));
		return;
	}
	TArray<FString> EmptyFolders;

	for (auto& SinglePath : CurrentSelectedPaths)
	{
		if (!FPaths::ValidatePath(SinglePath))
		{
			continue;
		}
		bool bHasAnyAsset = false;
		//有其他API可以拿到文件夹IFileManager::Get().FindFiles(),但是需要转换路径，比较麻烦
		TArray<FString> AssetsOrSubFolderInFolder = UEditorAssetLibrary::ListAssets(SinglePath, true, true);
		TSet<FString> CandidateFile(AssetsOrSubFolderInFolder);
		TSet<FString> FolderWithAsset;
		for (FString& AssetOrFolderPath : AssetsOrSubFolderInFolder)
		{
			//这两个路径下的不删除
			if (AssetOrFolderPath.Contains(TEXT("Developers")) || AssetOrFolderPath.Contains(TEXT("Collections")))
			{
				continue;
			}
			//返回的资产是带`.类型`的可以先把资产剔除
			int DotIndex = INDEX_NONE;
			AssetOrFolderPath.FindLastChar('.', DotIndex);
			if (DotIndex != INDEX_NONE)
			{
				//因为是直接初始化的，首先要把资产去掉
				FolderWithAsset.Emplace(AssetOrFolderPath);
				FString FailureReason;
				//这边函数的返回值和函数说明不符，返回的是资产路径去除了.
				FString ConvertedStr = EditorScriptingHelpers::ConvertAnyPathToLongPackagePath(
					AssetOrFolderPath, FailureReason);
				int LastSlashIndex = INDEX_NONE;
				ConvertedStr.FindLastChar('/', LastSlashIndex);
				if (LastSlashIndex != INDEX_NONE)
				{
					ConvertedStr.LeftInline(LastSlashIndex + 1);
				}
				FolderWithAsset.Emplace(ConvertedStr);
				bHasAnyAsset = true;
			}
			else if (!UEditorAssetLibrary::DoesDirectoryExist(AssetOrFolderPath))
			{
				FolderWithAsset.Emplace(AssetOrFolderPath);
			}
			//这个函数在5.6现在的版本有问题,无论下面有没有资产总是返回false，需要把window的区域中设置UTF8
			//bool bHaveAsset=UEditorAssetLibrary::DoesDirectoryHaveAssets(AssetOrFolderPath);
		}
		EmptyFolders = CandidateFile.Difference(FolderWithAsset).Array();
		if (!bHasAnyAsset)
		{
			EmptyFolders.Emplace(SinglePath);
		}
	}
	if (EmptyFolders.IsEmpty())
	{
		UNotifyTools::ShowMsgDialog(EAppMsgType::Ok,TEXT("Found No Empty Folder"));
		return;
	}
	EAppReturnType::Type UserChoice =
		UNotifyTools::ShowMsgDialog(EAppMsgType::YesNoCancel,TEXT("Delete Empty Folder?"));
	if (UserChoice == EAppReturnType::Yes)
	{
		int DeleteCounter = 0;
		for (const auto& EmptyFolder : EmptyFolders)
		{
			if (UEditorAssetLibrary::DeleteDirectory(EmptyFolder))
			{
				DeleteCounter++;
			}
			else
			{
				UNotifyTools::ShowCornerPopupMessage(FString::Printf(TEXT("Fail To Delete %s"), *EmptyFolder));
			}
		}
		if (DeleteCounter > 0)
		{
			UNotifyTools::ShowMsgDialog(EAppMsgType::Ok,
			                            FString::Printf(TEXT("Delete %d Empty Folder Successfully"), DeleteCounter));
		}
	}
}
#pragma endregion ContentBrowserTreeViewAction
#pragma region OpenWindowAction
void FMyEditorExtendModule::OnOpenTabButtonClicked()
{
	//注册新Tab
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AdvancedDeleteTab"),
	                                                  FOnSpawnTab::CreateRaw(
		                                                  this, &FMyEditorExtendModule::OnSpawnAdvancedDeleteTab)).
	                          SetDisplayName(FText::FromString(TEXT("Advanced Delete Tab")));
	//注意这里传入的参数应当和上边RegisterNomadTabSpawner的第一个参数**完全一致**
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvancedDeleteTab"));
}

TSharedRef<SDockTab> FMyEditorExtendModule::OnSpawnAdvancedDeleteTab(const FSpawnTabArgs& Args)
{
	
	TSharedRef<SDockTab> Tab = SNew(SDockTab).TabRole(NomadTab);
	Tab->SetContent(
		SNew(SAdvancedDeleteTab).AssetDataArray(GetAllAssetsDataUnderSelectedFolder())
	);
	return Tab;
}

TSet<TSharedPtr<FAssetData>> FMyEditorExtendModule::GetAllAssetsDataUnderSelectedFolder()
{
	TSet<TSharedPtr<FAssetData>> SelectedAssetData;
	for (auto SelectedPath : CurrentSelectedPaths)
	{
		TArray<FString> AssetsUnderPath = UEditorAssetLibrary::ListAssets(SelectedPath);
		for (const FString& Asset : AssetsUnderPath)
		{
			if (Asset.Contains(TEXT("Developers")) || Asset.Contains(TEXT("Collections")))
			{
				continue;
			}
			if (!UEditorAssetLibrary::DoesAssetExist(Asset))
			{
				continue;
			}
			FAssetData AssetData = UEditorAssetLibrary::FindAssetData(Asset);
			SelectedAssetData.Emplace(MakeShared<FAssetData>(AssetData));
		}
	}
	return SelectedAssetData;
}

bool FMyEditorExtendModule::DeleteGivenAssets(const TArray<TSharedPtr<FAssetData>>&TargetAssets)
{
	TArray<FAssetData> AssetsToDelete;
	for (const TSharedPtr<FAssetData>& SingleAsset : TargetAssets)
	{
		if (!UEditorAssetLibrary::DoesAssetExist(SingleAsset->GetObjectPathString()))
		{
			UNotifyTools::ShowCornerPopupMessage(FString::Printf(TEXT("Asset:%s Doesn't Exist"),*SingleAsset->GetExportTextName()));
			continue;
		}
		AssetsToDelete.Emplace(*SingleAsset.Get());
	}
	int DeleteCount= ObjectTools::DeleteAssets(AssetsToDelete);
	return DeleteCount==TargetAssets.Num();
}

void FMyEditorExtendModule::OpenPathInContentBrowser(const FString& AssetPath)
{
	UEditorAssetLibrary::SyncBrowserToObjects(TArray<FString>{AssetPath});
}

#pragma endregion OpenWindowAction
#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMyEditorExtendModule, MyEditorExtend)
