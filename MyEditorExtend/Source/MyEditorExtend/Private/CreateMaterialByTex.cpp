// Fill out your copyright notice in the Description page of Project Settings.


#include "CreateMaterialByTex.h"

#include <cwctype>

#include "AssetToolsModule.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "Factories/MaterialFactoryNew.h"
#include "Materials/MaterialExpressionTextureSample.h"


void UCreateMaterialByTex::CreateMaterialFromSelectedTex()
{
	if (bUseCustomName)
	{
		if (MaterialName == this->StaticClass()->GetDefaultObject<UCreateMaterialByTex>()->MaterialName)
		{
			GEngine->AddOnScreenDebugMessage(0, 10.0f, FColor::Red,TEXT("Please Check Material!"));
			return;
		}
	}
	TArray<FAssetData> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<UTexture2D*> AssetAsTexture;
	FString OutTexturePath;
	bool ProcessSuccess = ProcessSelectedData(SelectedAssets, AssetAsTexture, OutTexturePath);
	if (!ProcessSuccess)
	{
		return;
	}
	if (CheckNameIsUsed(OutTexturePath, MaterialName))
	{
		return;
	}
	UMaterial* NewMaterial = CreateMaterial(MaterialName, OutTexturePath);
	if (!NewMaterial)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,TEXT("Fail To Create Material"));
		return;
	}
	//把纹理连接到材质节点上
	uint32 PinCount = 0;
	for (UTexture2D* const& Texture2D : AssetAsTexture)
	{
		AddTextureNodeToMaterial(NewMaterial, Texture2D, PinCount);
	}
	if (PinCount > 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
		                                 FString::Printf(TEXT("Successfully Connected %d Pins"), PinCount));
	}
	GEngine->AddOnScreenDebugMessage(0, 10.0f, FColor::Green, OutTexturePath);
}

void UCreateMaterialByTex::RemoveResSuffix()
{
	TArray<UObject*> SelectedAsset = UEditorUtilityLibrary::GetSelectedAssetsOfClass(UTexture2D::StaticClass());
	if (SelectedAsset.IsEmpty())
	{
		return;
	}
	for (auto Asset : SelectedAsset)
	{
		if (Asset != nullptr)
		{
			FString OldName = Asset->GetName();
			if (std::towlower(OldName[OldName.Len() - 1]) == 'k' && std::iswdigit(OldName[OldName.Len() - 2]))
			{
				UEditorUtilityLibrary::RenameAsset(Asset, OldName.LeftChop(3));
			}
		}
	}
	UEditorAssetLibrary::SaveLoadedAssets(SelectedAsset, true);
}

bool UCreateMaterialByTex::ProcessSelectedData(const TArray<FAssetData>& SelectedDataProcess,
                                               TArray<UTexture2D*>& OutTextureArray, FString& OutTexturePath)
{
	if (SelectedDataProcess.IsEmpty())
	{
		return false;
	}
	bool bMaterialNameSet = false;
	for (auto AssetData : SelectedDataProcess)
	{
		UObject* AssetObject = AssetData.GetAsset();
		if (nullptr == AssetObject)
		{
			return false;
		}
		UTexture2D* Texture = Cast<UTexture2D>(AssetObject);
		if (nullptr == Texture)
		{
			GEngine->AddOnScreenDebugMessage(2, 10.0f, FColor::Red, FString::Printf(
				                                 TEXT("Selected Asset %s Is Not A Texture"),
				                                 *AssetData.AssetName.ToString()));
			continue;
		}
		OutTextureArray.Push(Texture);
		if (OutTexturePath.IsEmpty())
		{
			OutTexturePath = AssetData.PackagePath.ToString();
		}
		if (!bUseCustomName && !bMaterialNameSet)
		{
			MaterialName = AssetData.AssetName.ToString();
			MaterialName.RemoveFromStart(TEXT("T_"));
			MaterialName.InsertAt(0,TEXT("M_"));
			bMaterialNameSet = true;
		}
	}
	return !OutTextureArray.IsEmpty();
}


bool UCreateMaterialByTex::CheckNameIsUsed(const FString& FolderPath, const FString& CheckedName)
{
	auto FolderAssetPath = UEditorAssetLibrary::ListAssets(FolderPath, false);
	for (auto AssetPath : FolderAssetPath)
	{
		FString AssetName = FPaths::GetBaseFilename(AssetPath);
		if (AssetName == CheckedName)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red,
			                                 FString::Printf(TEXT("Name %s Was Used"), *AssetName));
			return true;
		}
	}
	return false;
}

UMaterial* UCreateMaterialByTex::CreateMaterial(const FString& MaterialSetName, const FString& FolderPath)
{
	FAssetToolsModule& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();
	UObject* CreateAsset = AssetTools.Get().CreateAsset(MaterialSetName, FolderPath, UMaterial::StaticClass(),
	                                                    MaterialFactory);
	return Cast<UMaterial>(CreateAsset);
}

void UCreateMaterialByTex::AddTextureNodeToMaterial(UMaterial* TargetMaterial, UTexture2D* SelectedTexture,
                                                    uint32& PinCount)
{
	if (nullptr == TargetMaterial || nullptr == SelectedTexture)
	{
		return;
	}
	UMaterialExpressionTextureSample* TextureSample = NewObject<UMaterialExpressionTextureSample>(TargetMaterial);
	if (!TextureSample)
	{
		return;
	}
	if (!TargetMaterial->HasBaseColorConnected())
	{
		if (TryConnectTexToBaseColor(TargetMaterial, SelectedTexture, TextureSample))
		{
			PinCount++;
			return;
		}
	}
	if (!TargetMaterial->HasNormalConnected())
	{
		if (TryConnectTexToNormal(TargetMaterial, SelectedTexture, TextureSample))
		{
			PinCount++;
			return;
		}
	}
	if (bUsePackagedORM)
	{
		if (!TargetMaterial->HasMetallicConnected() && !TargetMaterial->HasRoughnessConnected() && !TargetMaterial->
			HasAmbientOcclusionConnected())
		{
			if (TryConnectORM(TargetMaterial, SelectedTexture, TextureSample))
			{
				PinCount+=3;
				return;
			}
		}
	}
	else
	{
		if (!TargetMaterial->HasMetallicConnected())
		{
			if (TryConnectTexToMetallic(TargetMaterial, SelectedTexture, TextureSample))
			{
				PinCount++;
				return;
			}
		}
		if (!TargetMaterial->HasAmbientOcclusionConnected())
		{
			if (TryConnectTexToAO(TargetMaterial, SelectedTexture, TextureSample))
			{
				PinCount++;
				return;
			}
		}
		if (!TargetMaterial->HasRoughnessConnected())
		{
			if (TryConnectTexToRoughness(TargetMaterial, SelectedTexture, TextureSample))
			{
				PinCount++;
				return;
			}
		}
	}
}

bool UCreateMaterialByTex::TryConnectTexToBaseColor(UMaterial* TargetMaterial, UTexture2D* SelectedTexture,
                                                    UMaterialExpressionTextureSample* TextureSample)
{
	//从数组中找匹配的，这个肯定可以优化
	for (const auto& BaseColorSuffix : BaseColorArray)
	{
		if (SelectedTexture->GetName().Contains(BaseColorSuffix))
		{
			SetSelectedTextureAssetSettings(SelectedTexture, ETextureType::BaseColor);
			TextureSample->Texture = SelectedTexture;
			SetNodeParamInMaterial(TargetMaterial, SelectedTexture, TextureSample, ETextureType::BaseColor, 400, 0);
			return true;
		}
	}
	return false;
}

bool UCreateMaterialByTex::TryConnectTexToMetallic(UMaterial* TargetMaterial, UTexture2D* SelectedTexture,
                                                   UMaterialExpressionTextureSample* TextureSample)
{
	for (const auto& MetallicSuffix : MetallicArray)
	{
		if (SelectedTexture->GetName().Contains(MetallicSuffix))
		{
			//这边是对灰阶纹理资产的设置
			SetSelectedTextureAssetSettings(SelectedTexture, ETextureType::SingleChannel_Metallic);
			//设置采样模式
			TextureSample->Texture = SelectedTexture;
			TextureSample->SamplerType = SAMPLERTYPE_LinearColor;
			//创建表达节点
			SetNodeParamInMaterial(TargetMaterial, SelectedTexture, TextureSample, ETextureType::SingleChannel_Metallic,
			                       400, 400);
			return true;
		}
	}
	return false;
}

bool UCreateMaterialByTex::TryConnectTexToRoughness(UMaterial* TargetMaterial, UTexture2D* SelectedTexture,
                                                    UMaterialExpressionTextureSample* TextureSample)
{
	for (const auto& RoughnessSuffix : RoughnessArray)
	{
		if (SelectedTexture->GetName().Contains(RoughnessSuffix))
		{
			SetSelectedTextureAssetSettings(SelectedTexture, ETextureType::SingleChannel_Roughness);
			//设置采样模式
			TextureSample->Texture = SelectedTexture;
			TextureSample->SamplerType = SAMPLERTYPE_LinearColor;
			//创建表达节点
			SetNodeParamInMaterial(TargetMaterial, SelectedTexture, TextureSample,
			                       ETextureType::SingleChannel_Roughness, 400, 720);
			return true;
		}
	}
	return false;
}

bool UCreateMaterialByTex::TryConnectTexToNormal(UMaterial* TargetMaterial, UTexture2D* SelectedTexture,
                                                 UMaterialExpressionTextureSample* TextureSample)
{
	for (const auto& NormalSuffix : NormalArray)
	{
		if (SelectedTexture->GetName().Contains(NormalSuffix))
		{
			SetSelectedTextureAssetSettings(SelectedTexture, ETextureType::Normal);
			//设置采样模式
			TextureSample->Texture = SelectedTexture;
			TextureSample->SamplerType = SAMPLERTYPE_Normal;
			//创建表达节点
			SetNodeParamInMaterial(TargetMaterial, SelectedTexture, TextureSample,
			                       ETextureType::Normal, 400, 1000);
			return true;
		}
	}
	return false;
}

bool UCreateMaterialByTex::TryConnectTexToAO(UMaterial* TargetMaterial, UTexture2D* SelectedTexture,
                                             UMaterialExpressionTextureSample* TextureSample)
{
	for (const auto& AmbientOcclusionSuffix : AmbientOcclusionArray)
	{
		if (SelectedTexture->GetName().Contains(AmbientOcclusionSuffix))
		{
			SetSelectedTextureAssetSettings(SelectedTexture, ETextureType::SingleChannel_AO);
			//设置采样模式
			TextureSample->Texture = SelectedTexture;
			TextureSample->SamplerType = SAMPLERTYPE_LinearColor;
			//创建表达节点
			SetNodeParamInMaterial(TargetMaterial, SelectedTexture, TextureSample,
			                       ETextureType::SingleChannel_Roughness, 400, 920);
			return true;
		}
	}
	return false;
}

bool UCreateMaterialByTex::TryConnectORM(UMaterial* TargetMaterial, UTexture2D* SelectedTexture,
                                         UMaterialExpressionTextureSample* TextureSample)
{
	for (const auto& ORMSuffix : ORMArray)
	{
		if (SelectedTexture->GetName().Contains(ORMSuffix))
		{
			SetSelectedTextureAssetSettings(SelectedTexture, ETextureType::PackedChannel_ORM);
			//设置采样模式
			TextureSample->Texture = SelectedTexture;
			TextureSample->SamplerType = SAMPLERTYPE_LinearColor;
			//创建表达节点
			SetNodeParamInMaterial(TargetMaterial, SelectedTexture, TextureSample,
			                       ETextureType::PackedChannel_ORM, -400, -920);
			return true;
		}
	}
	return false;
}

void UCreateMaterialByTex::SetSelectedTextureAssetSettings(UTexture2D* SelectedTexture, ETextureType TextureType)
{
	switch (TextureType)
	{
	case ETextureType::BaseColor:
		SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
		SelectedTexture->SRGB = true;
		break;
	case ETextureType::PackedChannel_ORM:
		[[fallthrough]];
	case ETextureType::SingleChannel_Roughness:
		[[fallthrough]];
	case ETextureType::SingleChannel_Metallic:
		[[fallthrough]];
	case ETextureType::SingleChannel_AO:
		SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Default;
		SelectedTexture->SRGB = false;
		break;
	case ETextureType::Normal:
		SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Normalmap;
		SelectedTexture->SRGB = false;
		break;
	case ETextureType::Mask:
		SelectedTexture->CompressionSettings = TextureCompressionSettings::TC_Masks;
		SelectedTexture->SRGB = false;
		break;
	}
	SelectedTexture->PostEditChange();
}

void UCreateMaterialByTex::SetNodeParamInMaterial(UMaterial* TargetMaterial, UTexture2D* SelectedTexture,
                                                  UMaterialExpressionTextureSample* TextureSample,
                                                  ETextureType TextureType, int32 OffsetX, int32 OffsetY)
{
	TargetMaterial->GetExpressionCollection().AddExpression(TextureSample);
	EMaterialProperty TargetEndPoint = EMaterialProperty::MP_MAX;
	int32 ConnectChannelID = 0;
	if (TextureType != ETextureType::PackedChannel_ORM)
	{
		switch (TextureType)
		{
		case ETextureType::BaseColor:
			TargetEndPoint = EMaterialProperty::MP_BaseColor;
			break;
		case ETextureType::SingleChannel_Roughness:
			TargetEndPoint = EMaterialProperty::MP_Roughness;
			ConnectChannelID = 1;
			break;
		case ETextureType::SingleChannel_Metallic:
			TargetEndPoint = EMaterialProperty::MP_Metallic;
			ConnectChannelID = 1;
			break;
		case ETextureType::SingleChannel_AO:
			TargetEndPoint = EMaterialProperty::MP_AmbientOcclusion;
			ConnectChannelID = 1;
			break;
		case ETextureType::Normal:
			TargetEndPoint = EMaterialProperty::MP_Normal;
			break;
		case ETextureType::Mask:
			TargetEndPoint = EMaterialProperty::MP_OpacityMask;
			ConnectChannelID = 0;
			break;
		}

		ensureAlwaysMsgf(TargetEndPoint!=EMaterialProperty::MP_MAX, TEXT("Error MaterialPropertyTarget"));

		TargetMaterial->GetExpressionInputForProperty(TargetEndPoint)->Connect(ConnectChannelID, TextureSample);
	}
	else
	{
		TargetMaterial->GetExpressionInputForProperty(MP_AmbientOcclusion)->Connect(1, TextureSample);
		TargetMaterial->GetExpressionInputForProperty(MP_Roughness)->Connect(2, TextureSample);
		TargetMaterial->GetExpressionInputForProperty(MP_Metallic)->Connect(3, TextureSample);
	}

	TargetMaterial->PostEditChange();
	TextureSample->MaterialExpressionEditorX += OffsetX;
	TextureSample->MaterialExpressionEditorY -= OffsetY;
}
