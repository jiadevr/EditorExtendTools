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

		SLATE_ARGUMENT(TSet<TSharedPtr<FAssetData>>,AssetDataArray)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
private:
	//字体类设置
	FSlateFontInfo HeadingFont;
	FSlateFontInfo NormalFont;
	TSet<TSharedPtr<FAssetData>> DisplayAssetData;
	//ListView传入的不能为Set转出的右值，必须保存TArray
	TArray<TSharedPtr<FAssetData>> DisplayAssetDataArray;
	TSharedRef<SListView<TSharedPtr<FAssetData>>> ConstructListView();
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> ListViewComponent;
	void RefreshListView();
	TSharedRef<ITableRow> GetGenerateRowData(TSharedPtr<FAssetData> DisplayAssetData, const TSharedRef<STableViewBase>& OwnerTable);
	//CheckBox构造
	TSharedRef<SCheckBox> ConstructCheckBox(TSharedPtr<FAssetData> DisplayAssetData);
	void OnCheckBoxStateChange(ECheckBoxState NewState, TSharedPtr<FAssetData> DisplayAssetData);
	//资产名称和资产类型文字
	TSharedRef<STextBlock> ConstructTextBlock(FString DisplayText,bool bIsHeading=false);
	//删除功能按钮
	TSharedRef<SButton> ConstructDeleteButton(TSharedPtr<FAssetData> DisplayAssetData);
	//参数靠静态单播负载进来
	FReply OnDeleteButtonWasClicked(TSharedPtr<FAssetData> DisplayAssetData);
};
