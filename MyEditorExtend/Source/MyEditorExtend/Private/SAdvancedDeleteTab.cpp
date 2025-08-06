// Fill out your copyright notice in the Description page of Project Settings.


#include "SAdvancedDeleteTab.h"

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SAdvancedDeleteTab::Construct(const FArguments& InArgs)
{
	//启用键盘交互
	bCanSupportFocus=true;
	//定义字体样式
	FSlateFontInfo TitleFontInfo=FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	TitleFontInfo.Size=30;
	//存储传入的AssetData
	DisplayAssetData=InArgs._AssetDataArray;
	//获取传入参数在InArgs中获取时会多一个下划线
	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString("Advanced Delete"))
			.Font(TitleFontInfo)
			.Justification(ETextJustify::Center)
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Tips")))
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SScrollBox)
			+SScrollBox::Slot()
			[
				SNew(SListView<TSharedPtr<FAssetData>>)
				.ItemHeight(244.0f)
				.ListItemsSource(&DisplayAssetData)
				//逐行显示效果
				.OnGenerateRow(this,& SAdvancedDeleteTab::GetGenerateRowData)
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
		]
	];
}

TSharedRef<ITableRow> SAdvancedDeleteTab::GetGenerateRowData(TSharedPtr<FAssetData> SingleDisplayAssetData,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	FString SingleAssetDisplayName= SingleDisplayAssetData.Get()->AssetName.ToString();
	auto ListViewChild=SNew(STableRow<TSharedPtr<FAssetData>>,OwnerTable)
	[
		SNew(STextBlock)
		.Text(FText::FromString(SingleAssetDisplayName))
	];
	return ListViewChild;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
