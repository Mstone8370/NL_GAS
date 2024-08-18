// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ParticleData.generated.h"

class UMaterialInterface;
class UFXSystemAsset;

/**
* 레플리케이트 되어야하지만, 현재는 모든 정보가 항상 필요하므로
* NetSerialize를 구현하지 않음.
*/
USTRUCT(Blueprintable)
struct FParticleSpawnInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FVector Normal = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct FParticleInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UFXSystemAsset> HitImpactFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMaterialInterface> HitImpactDecalMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector DecalSize = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DecalLifeSpan = 30.f;

	float FadeScreenSize = 0.0001f;
};

/**
 * 
 */
UCLASS()
class NL_GAS_API UParticleData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, FParticleInfo> Data;

	const FParticleInfo* FindParticleDataByTag(const FGameplayTag& Tag);

	bool HasParticleData(const FGameplayTag& Tag) const;
};
