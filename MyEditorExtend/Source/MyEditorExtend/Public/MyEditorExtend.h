// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FMyEditorExtendModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

#pragma region ContentBrowserExtend

private:
	void InitialContentBrowserExtend();
	TSharedRef<FExtender> CreatedAndPlaceExtender(const TArray<FString>& SelectedPaths);
	void AddExtendButtonEntry(FMenuBuilder& MenuBuilder);
	TArray<FString> CurrentSelectedPaths;
	//删除没有引用的资源
	void OnDeleteUnusedButtonClicked();
	//删除空文件夹
	void OnDeleteEmptyClicked();
	//打开一个Slate窗口进行操作
	void OnOpenTabButtonClicked();
	TSharedRef<SDockTab> OnSpawnAdvancedDeleteTab(const FSpawnTabArgs& Args);
	
public:
	//获取当前路径下所有资产
	TSet<TSharedPtr<FAssetData>> GetAllAssetsDataUnderSelectedFolder();
	//Slate窗口中的删除控制
	bool DeleteGivenAssets(const TArray<TSharedPtr<FAssetData>>& TargetAssets);
	//Slate窗口中定位到给定资产
	void OpenPathInContentBrowser(const FString& AssetPath);
	//Slate窗口中寻找未使用资产
	TSet<TSharedPtr<FAssetData>> ListAllUnusedAssets(const TSet<TSharedPtr<FAssetData>>& SelectedAssets) const;
#pragma endregion  ContentBrowserExtend
};
