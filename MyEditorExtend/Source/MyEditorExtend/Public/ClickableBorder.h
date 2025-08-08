// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
DECLARE_DELEGATE_TwoParams(FOnBorderDoubleClicked, const FGeometry& /*InGeometry*/,
	const FPointerEvent& /*InMouseEvent*/);
/**
 * 
 */
class MYEDITOREXTEND_API SClickableBorder : public SBorder
{
public:
	SLATE_BEGIN_ARGS(SClickableBorder): _InAssetData(nullptr)
		{
		}

		SLATE_ARGUMENT(TSharedPtr<FAssetData>, InAssetData)
		//把双击绑定传入函数
		SLATE_EVENT(FOnBorderDoubleClicked,CustomDoubleEvent)
		//必须，需要把SLOT槽暴露出来
		SLATE_DEFAULT_SLOT(FArguments, Child);
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<FAssetData> AssetData;
	FOnBorderDoubleClicked OnDoubleClickEvent;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
};
