// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ControlShakeData.generated.h"

class UCurveVector;

USTRUCT(BlueprintType)
struct FTaggedControlShake
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UCurveVector> ShakeCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Duration = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseMagnitude = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CriticalMagnitude = 0.f;
};

UCLASS()
class NL_GAS_API UControlShakeData : public UDataAsset
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, FTaggedControlShake> Data;

public:
	const FTaggedControlShake* GetControlShakeData(const FGameplayTag& ShakeTag) const;
};
