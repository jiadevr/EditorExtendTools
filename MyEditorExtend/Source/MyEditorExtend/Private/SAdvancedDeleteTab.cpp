// Fill out your copyright notice in the Description page of Project Settings.


#include "SAdvancedDeleteTab.h"

#include "ClickableBorder.h"
#include "MyEditorExtend.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SAdvancedDeleteTab::Construct(const FArguments& InArgs)
{
	//启用键盘交互
	bCanSupportFocus = true;
	//初始化参数
	CheckedAssets.Empty();
	CheckBoxes.Empty();
	ComboBoxElems.Empty();
	ComboBoxElems.Emplace(MakeShared<FName>(TEXT("ListAllAssets")));
	ComboBoxElems.Emplace(MakeShared<FName>(TEXT("ListUnusedAssets")));
	//定义字体样式
	HeadingFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	HeadingFont.Size = 30;
	NormalFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	NormalFont.Size = 12;
	//存储传入的AssetData
	DisplayAssetData = InArgs._AssetDataArray;
	DisplayAssetDataArray = DisplayAssetData.Array();
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
				ConstructComboBox()
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Check")))
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			.VAlign(VAlign_Fill)
			[
				ConstructListView()
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.0f)
			[
				ConstructButton(
					TEXT("SelectedAll"), FOnClicked::CreateSP(this, &SAdvancedDeleteTab::OnSelectAllButtonClicked))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.0f)
			[
				ConstructButton(
					TEXT("DeselectedAll"), FOnClicked::CreateSP(this, &SAdvancedDeleteTab::OnDeselectAllButtonClicked))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.0f)
			[
				ConstructButton(
					TEXT("DeleteAll"), FOnClicked::CreateSP(this, &SAdvancedDeleteTab::OnDeleteAllButtonClicked))
			]

		]
	];
}

FMyEditorExtendModule& SAdvancedDeleteTab::GetMainModule() const
{
	return FModuleManager::LoadModuleChecked<FMyEditorExtendModule>(TEXT("MyEditorExtend"));
}

TSharedRef<SListView<TSharedPtr<FAssetData>>> SAdvancedDeleteTab::ConstructListView()
{
	ListViewComponent = SNew(SListView<TSharedPtr<FAssetData>>)
		.ListItemsSource(&DisplayAssetDataArray)
		.OnGenerateRow(this, &SAdvancedDeleteTab::GetGenerateRowData);
	return ListViewComponent.ToSharedRef();
}

void SAdvancedDeleteTab::OnListViewRowWasDoubleClicked(TSharedPtr<FAssetData> ClickedAsset)
{
	UE_LOG(LogTemp, Display, TEXT("Asset %s Was Clicked"), *ClickedAsset->GetObjectPathString())
	GetMainModule().OpenPathInContentBrowser(ClickedAsset->GetObjectPathString());
}

void SAdvancedDeleteTab::RefreshListView()
{
	if (ListViewComponent.IsValid())
	{
		DisplayAssetDataArray = DisplayAssetData.Array();
		ListViewComponent->RebuildList();
	}
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
				//5.6不能支持直接在拼凑的一行里直接绑定OnMouseDoubleClicked等事件了，必须用子类包一层，这边直接包裹了名称TextBlock
				//继承SBorder直接用[]包裹，不用+Slot
				SNew(SClickableBorder)
				                      .InAssetData(SingleDisplayAssetData)
				//委托方式支持传入自定义函数，静态单播支持带负载
				                      .CustomDoubleEvent(this,
				                                         &SAdvancedDeleteTab::HandleTextContainerDoubleClicked,
				                                         SingleDisplayAssetData)
				[
					ConstructTextBlock(SingleAssetDisplayName)
				]
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.FillWidth(0.3f)
			[
				ConstructTextBlock(SingleAssetClassName)
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Fill)
			.FillWidth(0.3f)
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
	CheckBoxes.Emplace(CheckBox);
	return CheckBox;
}

void SAdvancedDeleteTab::OnCheckBoxStateChange(ECheckBoxState NewState, TSharedPtr<FAssetData> SingleDisplayAssetData)
{
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		if (CheckedAssets.Contains(SingleDisplayAssetData))
		{
			CheckedAssets.Remove(SingleDisplayAssetData);
		}
		break;

	case ECheckBoxState::Checked:
		CheckedAssets.Add(SingleDisplayAssetData);
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

void SAdvancedDeleteTab::HandleTextContainerDoubleClicked(const FGeometry& InGeometry,
                                                          const FPointerEvent& InPointerEvent,
                                                          const TSharedPtr<FAssetData> TargetAssetData)
{
	GetMainModule().OpenPathInContentBrowser(TargetAssetData->GetObjectPathString());
	UE_LOG(LogTemp, Display, TEXT("Was Clicked"));
}


TSharedRef<SButton> SAdvancedDeleteTab::ConstructDeleteButton(TSharedPtr<FAssetData> SingleDisplayAssetData)
{
	TSharedRef<SButton> Button = SNew(SButton).Text(FText::FromString("Delete")).OnClicked(
			this, &SAdvancedDeleteTab::OnDeleteButtonWasClicked, SingleDisplayAssetData);
	return Button;
}

FReply SAdvancedDeleteTab::OnDeleteButtonWasClicked(TSharedPtr<FAssetData> SingleDisplayAssetData)
{
	TArray<TSharedPtr<FAssetData>> DeleteAssetArray;
	DeleteAssetArray.Emplace(SingleDisplayAssetData);
	bool bDeleteSuccessfully = GetMainModule().DeleteGivenAssets(DeleteAssetArray);
	if (bDeleteSuccessfully && DisplayAssetData.Contains(SingleDisplayAssetData))
	{
		DisplayAssetData.Remove(SingleDisplayAssetData);
	}
	RefreshListView();
	return FReply::Handled();
}

TSharedRef<SButton> SAdvancedDeleteTab::ConstructButton(FString ButtonText, FOnClicked BindEvent)
{
	TSharedRef<SButton> Button = SNew(SButton)
		.ContentPadding(FMargin(5.0f))
		.OnClicked(BindEvent)
		[
			ConstructTextBlock(ButtonText)
		];
	return Button;
}

FReply SAdvancedDeleteTab::OnSelectAllButtonClicked()
{
	if (!CheckBoxes.IsEmpty())
	{
		for (const TSharedPtr<SCheckBox>& CheckBox : CheckBoxes)
		{
			if (!CheckBox->IsChecked())
			{
				CheckBox->SetIsChecked(ECheckBoxState::Checked);
			}
		}
	}
	return FReply::Handled();
}

FReply SAdvancedDeleteTab::OnDeselectAllButtonClicked()
{
	if (!CheckBoxes.IsEmpty())
	{
		for (const TSharedPtr<SCheckBox>& CheckBox : CheckBoxes)
		{
			if (CheckBox->IsChecked())
			{
				CheckBox->SetIsChecked(ECheckBoxState::Unchecked);
			}
		}
	}
	return FReply::Handled();
}

FReply SAdvancedDeleteTab::OnDeleteAllButtonClicked()
{
	if (!CheckedAssets.IsEmpty())
	{
		TArray<TSharedPtr<FAssetData>> CheckAssetsArray = CheckedAssets.Array();
		TArray<int32> A{100, 10};
		TArray<int32> B = MoveTemp(A);
		bool bDeleteSuccessfully = GetMainModule().DeleteGivenAssets(CheckAssetsArray);
		if (bDeleteSuccessfully)
		{
			DisplayAssetData = DisplayAssetData.Difference(CheckedAssets);
			DisplayAssetDataArray = DisplayAssetData.Array();
			RefreshListView();
			CheckedAssets.Empty();
		}
	}
	return FReply::Handled();
}

TSharedRef<SComboBox<TSharedPtr<FName>>> SAdvancedDeleteTab::ConstructComboBox()
{
	TSharedRef<SComboBox<TSharedPtr<FName>>> ComboBox = SNew(SComboBox<TSharedPtr<FName>>)
		.OptionsSource(&ComboBoxElems)
		.OnGenerateWidget(this, &SAdvancedDeleteTab::OnGeneratedComboContent)
		.OnSelectionChanged(this, &SAdvancedDeleteTab::OnComboBoxSelectionChanged)
		[
			SAssignNew(ComboDisplayTextBlock, STextBlock)
			.Text(FText::FromString(TEXT("List Assets Option")))
		];
	return ComboBox;
}

TSharedRef<SWidget> SAdvancedDeleteTab::OnGeneratedComboContent(TSharedPtr<FName> SourceItem)
{
	TSharedRef<STextBlock> ComboBoxText = SNew(STextBlock)
		.Text(FText::FromName(*SourceItem.Get()));
	return ComboBoxText;
}

void SAdvancedDeleteTab::OnComboBoxSelectionChanged(TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectInfo)
{
	ComboDisplayTextBlock->SetText(FText::FromName(*NewSelection.Get()));
	CheckedAssets.Empty();
	CheckBoxes.Empty();
	ComboDisplayTextBlock->SetText(FText::FromName(*NewSelection.Get()));
	if (*NewSelection.Get() == FName("ListAllAssets"))
	{
		DisplayAssetData = GetMainModule().GetAllAssetsDataUnderSelectedFolder();
	}
	else if (*NewSelection.Get() == FName("ListUnusedAssets"))
	{
		DisplayAssetData = GetMainModule().ListAllUnusedAssets(DisplayAssetData);
	}
	RefreshListView();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
