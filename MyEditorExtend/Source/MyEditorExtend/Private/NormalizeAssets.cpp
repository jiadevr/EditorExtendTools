// Fill out your copyright notice in the Description page of Project Settings.


#include "NormalizeAssets.h"

#include "EditorAssetLibrary.h"
#include "EditorUtilityLibrary.h"
#include "NotifyTools.h"
#include "ObjectTools.h"

void UNormalizeAssets::NormalizeAssetName()
{
	TArray<UObject*> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssets();
	int32 Counter = 0;
	for (auto SingleAsset : SelectedAssets)
	{
		//表中没有这类对象，可能是非打包资产，略过
		if (!PrefixMap.Contains(SingleAsset->GetClass()))
		{
			UNotifyTools::ShowCornerPopupMessage(
				FString::Printf(TEXT("No Match Prefix For Asset %s"), *SingleAsset->GetName()));
			continue;
		}
		FString PrefixStr = *PrefixMap.Find(SingleAsset->GetClass());
		FString OldName = SingleAsset->GetName();
		//检测原有前缀是什么
		int32 FirstUnderlineIndex = INDEX_NONE;
		OldName.FindChar('_', FirstUnderlineIndex);
		FString NewName = OldName;
		bool bHadChanged = false;
		//最长前缀WBP_中_Index为3,这里检测的是有无前缀
		if (FirstUnderlineIndex != INDEX_NONE && FirstUnderlineIndex <= 3)
		{
			//检查是不是在集合中的前缀
			FString OldPrefix = OldName.Left(FirstUnderlineIndex + 1);
			bool bMatchExactly = PrefixStr.Equals(OldPrefix, ESearchCase::CaseSensitive);
			//已经完全匹配,遍历到下一个
			if (bMatchExactly)
			{
				continue;
			}
			//未完全匹配，检测是否是大小写差异;或者直接选择了强制替换
			if (bShouldOverride || PrefixStr.Equals(OldPrefix, ESearchCase::IgnoreCase))
			{
				NewName = OldName.Replace(*OldPrefix, *PrefixStr);
				bHadChanged = true;
			}
		}
		//没有前缀或者不是大小写匹配问题,或者强制替换
		if (!bHadChanged || bShouldOverride)
		{
			NewName = PrefixStr + OldName;
		}
		//检测资源名称长度是是否过长
		if (bShouldTrim && NewName.Len() > NameLengthLimit)
		{
			NewName.LeftInline(NameLengthLimit);
			//@TODO:可能要判断重名
		}
		UEditorUtilityLibrary::RenameAsset(SingleAsset, NewName);
		Counter++;
	}
	UNotifyTools::ShowMsgDialog(EAppMsgType::Ok,
	                            FString::Printf(
		                            TEXT("Finish Normalize Name Action Search %d Assets, Change %d"),
		                            SelectedAssets.Num(), Counter));
}

void UNormalizeAssets::DeleteUnusedAssets()
{
		TArray<FAssetData> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssetData();
	//@TODO:可能需要根据选择数量决定原地删还是待辅助变量删
	TArray<FAssetData> UnusedAssets;
	for (const FAssetData& AssetData : SelectedAssets)
	{
		//@TODO:这个引用关系可以优化参考HotPatcher
		TArray<FString> ReferenceSource = UEditorAssetLibrary::FindPackageReferencersForAsset(
			AssetData.GetObjectPathString());
		if (ReferenceSource.IsEmpty())
		{
			UnusedAssets.Emplace(AssetData);
		}
	}
	if (!UnusedAssets.IsEmpty())
	{
		int32 DeleteCount = ObjectTools::DeleteAssets(UnusedAssets);
		UNotifyTools::ShowMsgDialog(EAppMsgType::Ok,
		                            FString::Printf(
			                            TEXT("Select %d Assets,Delete %d Assets"), SelectedAssets.Num(), DeleteCount));
		return;
	}
	UNotifyTools::ShowMsgDialog(EAppMsgType::Ok,TEXT("All Asset Was Refenced"));
}	
