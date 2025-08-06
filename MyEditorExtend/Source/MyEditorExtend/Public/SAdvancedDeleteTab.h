// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class MYEDITOREXTEND_API SAdvancedDeleteTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAdvancedDeleteTab)
		{
		}

		SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>,AssetDataArray)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
private:
	TArray<TSharedPtr<FAssetData>> DisplayAssetData;
	TSharedRef<ITableRow> GetGenerateRowData(TSharedPtr<FAssetData> DisplayAssetData, const TSharedRef<STableViewBase>& OwnerTable);
};
