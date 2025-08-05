// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "NormalizeAssets.generated.h"


UCLASS(Blueprintable)
class MYEDITOREXTEND_API UNormalizeAssets : public UEditorUtilityWidget
{
	GENERATED_BODY()
#pragma region AddPrefix
public:
	UFUNCTION(BlueprintCallable,Category="NormalizeName|AddPrefix")
	void NormalizeAssetName();

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NormalizeName",DisplayName="OverrideMismatchedPrefix")
	bool bShouldOverride=false;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NormalizeName",DisplayName="TrimIfLongerThanLimit")
	bool bShouldTrim=false;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="NormalizeName",meta=(EditCondition=bShouldTrim))
	int32 NameLengthLimit=15;
private:

	//资产名称对应前缀
	TMap<UClass* ,FString> PrefixMap = {
		{UBlueprint::StaticClass(),TEXT("BP_")},
		{UStaticMesh::StaticClass(),TEXT("SM_")},
		{UMaterial::StaticClass(), TEXT("M_")},
		{UMaterialInstanceConstant::StaticClass(),TEXT("MI_")},
		{UMaterialFunctionInterface::StaticClass(), TEXT("MF_")},
		{UParticleSystem::StaticClass(), TEXT("PS_")},
		{USoundCue::StaticClass(), TEXT("SC_")},
		{USoundWave::StaticClass(), TEXT("SW_")},
		{UTexture::StaticClass(), TEXT("T_")},
		{UTexture2D::StaticClass(), TEXT("T_")},
		{UUserWidget::StaticClass(), TEXT("WBP_")},
		{USkeletalMeshComponent::StaticClass(), TEXT("SK_")}
	};
#pragma endregion AddPrefix

#pragma region CleanUnunsedAsset
public:
	UFUNCTION(BlueprintCallable,Category="NormalizeName|RemoveUnused")
	void DeleteUnusedAssets();
#pragma endregion CleanUnunsedAsset 
};
