// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AimPunchData.generated.h"

class UCurveVector;

USTRUCT(BlueprintType)
struct FTaggedAimPunch
{
	GENERATED_BODY();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UCurveVector> ControlShakeCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Duration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseMagnitude;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CriticalMagnitude;
};

UCLASS()
class NL_GAS_API UAimPunchData : public UDataAsset
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, FTaggedAimPunch> Data;

public:
	const FTaggedAimPunch* GetAimPunchData(const FGameplayTag& DamageType) const;
};
