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
	void AddDeleteUnusedEntry(FMenuBuilder& MenuBuilder);
	TArray<FString> CurrentSelectedPaths;
	void OnDeleteUnusedButtonClick();
#pragma endregion  ContentBrowserExtend 
};
