// Fill out your copyright notice in the Description page of Project Settings.


#include "ClickableBorder.h"

#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SClickableBorder::Construct(const FArguments& InArgs)
{
	AssetData = InArgs._InAssetData;
	OnDoubleClickEvent=InArgs._CustomDoubleEvent;
	SBorder::Construct(
		SBorder::FArguments()
		.BorderBackgroundColor(FLinearColor(0.25f,0.25f,0.25f,0.25f))
		.Padding(2.0f)
		.OnMouseDoubleClick(this, &SClickableBorder::OnMouseButtonDoubleClick)
		[
			//把传入的Child挂载到SBorder
			InArgs._Child.Widget
		]);
}

FReply SClickableBorder::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
	if (OnDoubleClickEvent.IsBound())
	{
		OnDoubleClickEvent.Execute(InMyGeometry,InMouseEvent);
		return FReply::Handled();
	}
	return SBorder::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
