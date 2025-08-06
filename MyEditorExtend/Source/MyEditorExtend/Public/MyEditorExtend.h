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
	void OnDeleteUnusedButtonClick();
	//删除空文件夹
	void OnDeleteEmptyClick();
	
	
#pragma endregion  ContentBrowserExtend
	
};
