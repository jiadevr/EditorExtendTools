// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "CreateMaterialByTex.generated.h"


class UMaterialExpressionTextureSample;
enum class ETextureType:uint8
{
	BaseColor,
	SingleChannel_Roughness,
	SingleChannel_Metallic,
	SingleChannel_AO,
	PackedChannel_ORM,
	Normal,
	Mask,
	MAX
};
/*enum class EChannelPackingType:uint8
{
	NoChannelPacking,
	ORMPacking,
	MAX
};*/
/**
 * 
 */
UCLASS()
class MYEDITOREXTEND_API UCreateMaterialByTex : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	/**
	 * 根据选定的纹理创建材质
	 */
	UFUNCTION(BlueprintCallable)
	void CreateMaterialFromSelectedTex();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CreateMaterialFromTex")
	FString MaterialName = "M_";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CreateMaterialFromTex")
	bool bUseCustomName = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CreateMaterialFromTex")
	bool bUsePackagedORM=false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CreateMaterialFromTex")
	bool bCreateMaterialInstance=false;
#pragma region TextureSuffix
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextureSuffix")
	TArray<FString> BaseColorArray = {
		TEXT("_BaseColor"),
		TEXT("_Albedo"),
		TEXT("_Diffuse"),
		TEXT("_diff")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextureSuffix")
	TArray<FString> MetallicArray = {
		TEXT("_Metallic"),
		TEXT("_metal")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextureSuffix")
	TArray<FString> RoughnessArray = {
		TEXT("_Roughness"),
		TEXT("_RoughnessMap"),
		TEXT("_rough")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextureSuffix")
	TArray<FString> NormalArray = {
		TEXT("_Normal"),
		TEXT("_NormalMap"),
		TEXT("_nor")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextureSuffix")
	TArray<FString> AmbientOcclusionArray = {
		TEXT("_AmbientOcclusion"),
		TEXT("_AmbientOcclusionMap"),
		TEXT("_AO")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TextureSuffix")
	TArray<FString> ORMArray = {
		TEXT("_ORM"),
		TEXT("_orm"),
		TEXT("_ARM"),
		TEXT("_arm")
	};
#pragma endregion TextureSuffix
	/**
	 * 移除后缀的分辨率后缀
	 */
	UFUNCTION(BlueprintCallable)
	void RemoveResSuffix();

private:
	/**
	 * 把选中的Asset中Texture2D类型挑出来，使用自定义名称（bUseCustomName）时保留自定义，否则使用第一个有效元素的名称作为材质名称
	 * @param SelectedDataProcess 选中的Asset
	 * @param OutTextureArray 选中的是Texture2D类型的资产
	 * @param OutTexturePath 第一个元素的位置
	 * @return 为空时返回false
	 */
	bool ProcessSelectedData(const TArray<FAssetData>& SelectedDataProcess, TArray<UTexture2D*>& OutTextureArray,
							 FString& OutTexturePath);

	/**
	 * 检查是否有同名材质
	 * @param FolderPath 文件路径，上边传入的是第一张贴图的文件位置‘/Game/’开头
	 * @param CheckedName 比对名称
	 * @return 已经存在同名时返回ture；
	 */
	bool CheckNameIsUsed(const FString& FolderPath, const FString& CheckedName);

	UMaterial* CreateMaterial(const FString& MaterialSetName, const FString& FolderPath);
	
	/**
	 * 尝试将纹理连接到材质球
	 * @param TargetMaterial 目标材质资产
	 * @param SelectedTexture 目标纹理资产
	 * @param PinCount 节点
	 */
	void AddTextureNodeToMaterial(UMaterial* TargetMaterial, UTexture2D* SelectedTexture, uint32& PinCount);
	void SetSelectedTextureAssetSettings(UTexture2D* SelectedTexture,ETextureType TextureType);
	void SetNodeParamInMaterial(UMaterial* TargetMaterial, UTexture2D* SelectedTexture,UMaterialExpressionTextureSample* TextureSample,ETextureType TextureType,int32 OffsetX,int32 OffsetY);
	bool TryConnectTexToBaseColor(UMaterial* TargetMaterial, UTexture2D* SelectedTexture,UMaterialExpressionTextureSample* TextureSample);
	bool TryConnectTexToMetallic(UMaterial* TargetMaterial, UTexture2D* SelectedTexture,UMaterialExpressionTextureSample* TextureSample);
	bool TryConnectTexToRoughness(UMaterial* TargetMaterial, UTexture2D* SelectedTexture,UMaterialExpressionTextureSample* TextureSample);
	bool TryConnectTexToNormal(UMaterial* TargetMaterial, UTexture2D* SelectedTexture,UMaterialExpressionTextureSample* TextureSample);
	bool TryConnectTexToAO(UMaterial* TargetMaterial, UTexture2D* SelectedTexture,UMaterialExpressionTextureSample* TextureSample);
	bool TryConnectORM(UMaterial*  TargetMaterial, UTexture2D* SelectedTexture,UMaterialExpressionTextureSample* TextureSample);
	//创建材质实例
	void CreateMaterialInstanceFromMaterial(UMaterial* TargetMaterial, const FString& FolderPath);
};
