// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "WeaponRecoilPattern.generated.h"

class UCurveVector;

USTRUCT(BlueprintType)
struct FWeaponRecoilInfo
{
	GENERATED_BODY();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UCurveVector> PatternSequence;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 LoopStartOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 LoopEndOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float RecoilOffsetResetTime = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UCurveVector> SingleRecoilCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SingleRecoilDuration = 1.f;
};

/**
 * 
 */
UCLASS()
class NL_GAS_API UWeaponRecoilPattern : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, FWeaponRecoilInfo> Data;

	bool HasRecoilPattern(const FGameplayTag& WeaponTag) const;

	FVector GetRecoilPatternAt(const FGameplayTag& WeaponTag, int32 Offset) const;
};
