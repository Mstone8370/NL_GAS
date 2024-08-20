// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ProjectileData.generated.h"

class ANLProjectile;

USTRUCT(Blueprintable)
struct FProjectileSpawnInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FVector Direction = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FGuid Id = FGuid();
};

USTRUCT(BlueprintType)
struct FProjectileInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<ANLProjectile> ProjectileClass;
};

/**
 * 
 */
UCLASS()
class NL_GAS_API UProjectileData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, FProjectileInfo> Data;

	const FProjectileInfo* FindProjectileDataByTag(const FGameplayTag& Tag);

	bool HasProjectileData(const FGameplayTag& Tag) const;
};
