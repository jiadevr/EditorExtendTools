// Fill out your copyright notice in the Description page of Project Settings.


#include "SAdvancedDeleteTab.h"

#include "MyEditorExtend.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SAdvancedDeleteTab::Construct(const FArguments& InArgs)
{
	//启用键盘交互
	bCanSupportFocus = true;
	//定义字体样式
	HeadingFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	HeadingFont.Size = 30;
	NormalFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	NormalFont.Size = 12;
	//存储传入的AssetData
	DisplayAssetData = InArgs._AssetDataArray;
	//获取传入参数在InArgs中获取时会多一个下划线
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString("Advanced Delete"))
			.Font(HeadingFont)
			.Justification(ETextJustify::Center)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Tips")))
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			.VAlign(VAlign_Fill)
			[
				SNew(SListView<TSharedPtr<FAssetData>>)
				                                       .ItemHeight(24.0f)
				                                       .ListItemsSource(&DisplayAssetData)
				//逐行显示效果
				                                       .OnGenerateRow(this, &SAdvancedDeleteTab::GetGenerateRowData)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
		]
	];
}

TSharedRef<ITableRow> SAdvancedDeleteTab::GetGenerateRowData(TSharedPtr<FAssetData> SingleDisplayAssetData,
                                                             const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!SingleDisplayAssetData.IsValid())
	{
		return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable);
	}
	FString SingleAssetDisplayName = SingleDisplayAssetData.Get()->AssetName.ToString();
	//GetClass的类被弃用了，需要转一下
	FString SingleAssetClassName = SingleDisplayAssetData.Get()->AssetClassPath.ToString();
	int DotIndex = INDEX_NONE;
	SingleAssetClassName.FindLastChar('.', DotIndex);
	if (DotIndex != INDEX_NONE)
	{
		SingleAssetClassName.RightChopInline(DotIndex + 1);
	}
	auto ListViewChild = SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.FillWidth(0.05f)
			[
				ConstructCheckBox(SingleDisplayAssetData)
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.FillWidth(0.3f)
			[
				ConstructTextBlock(SingleAssetDisplayName)
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				ConstructTextBlock(SingleAssetClassName)
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Fill)
			[
				ConstructDeleteButton(SingleDisplayAssetData)
			]
		];
	return ListViewChild;
}

TSharedRef<SCheckBox> SAdvancedDeleteTab::ConstructCheckBox(TSharedPtr<FAssetData> SingleDisplayAssetData)
{
	TSharedRef<SCheckBox> CheckBox = SNew(SCheckBox)
	                                                .Type(ESlateCheckBoxType::Type::CheckBox)
									//这个函数可以带负载传参
	                                                .OnCheckStateChanged(
		                                                this, &SAdvancedDeleteTab::OnCheckBoxStateChange,
		                                                SingleDisplayAssetData)
	                                                .Visibility(EVisibility::Visible);
	return CheckBox;
}

void SAdvancedDeleteTab::OnCheckBoxStateChange(ECheckBoxState NewState, TSharedPtr<FAssetData> SingleDisplayAssetData)
{
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
		                                 FString::Printf(
			                                 TEXT("%s was Unchecked"), *SingleDisplayAssetData->GetAsset()->GetName()));
		break;

	case ECheckBoxState::Checked:
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green,
		                                 FString::Printf(
			                                 TEXT("%s was Checked"), *SingleDisplayAssetData->GetAsset()->GetName()));
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green,TEXT("111111"));
		break;

	case ECheckBoxState::Undetermined:
		break;
	}
}

TSharedRef<STextBlock> SAdvancedDeleteTab::ConstructTextBlock(FString DisplayText, bool bIsHeading/*=false*/)
{
	TSharedRef<STextBlock> TextBlock = SNew(STextBlock)
		.Text(FText::FromString(DisplayText))
		.Font(bIsHeading ? HeadingFont : NormalFont)
		.ColorAndOpacity(FColor::White);
	return TextBlock;
}

TSharedRef<SButton> SAdvancedDeleteTab::ConstructDeleteButton(TSharedPtr<FAssetData> SingleDisplayAssetData)
{
	TSharedRef<SButton> Button = SNew(SButton).Text(FText::FromString("Delete")).OnClicked(
			this, &SAdvancedDeleteTab::OnDeleteButtonWasClicked, SingleDisplayAssetData);
	return Button;
}

FReply SAdvancedDeleteTab::OnDeleteButtonWasClicked(TSharedPtr<FAssetData> SingleDisplayAssetData)
{
	FMyEditorExtendModule MainModule=FModuleManager::LoadModuleChecked<FMyEditorExtendModule>(TEXT("MyEditorExtend"));
	TArray<TSharedPtr<FAssetData>> DeleteAssetArray;
	DeleteAssetArray.Emplace(SingleDisplayAssetData);
	MainModule.DeleteGivenAssets(DeleteAssetArray);
	return FReply::Handled();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
