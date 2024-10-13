// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "ControlShake.generated.h"

class UCurveVector;

USTRUCT(BlueprintType)
struct FControlShakeParams
{
	GENERATED_BODY()

public:
	// To make it loop, set value to 0
	UPROPERTY(BlueprintReadWrite)
	float Duration = 1.f;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UCurveVector> Curve;

	void Clear()
	{
		Duration = 1.f;
		Curve = nullptr;
	}
};

UCLASS()
class NL_GAS_API UControlShake : public UObject
{
	GENERATED_BODY()
	
public:
	UControlShake();

	FControlShakeParams ControlShakeParams;

	/**
	* Update time with given DeltaTime and return shake value.
	* @return Whether this Control Shake Object is still active after update.
	*/
	bool UpdateShake(float DeltaTime, FRotator& OutShake);

	void Activate(float InDuration, UCurveVector* InCurve, FRotator InShakeMagnitude);

	void Reactivate(FRotator InShakeMagnitude);

	FORCEINLINE bool IsActive() const { return bIsActive; }

	FORCEINLINE FGameplayTag GetShakeTag() const { return ShakeTag; }

	void Clear();

	FRotator ShakeMagnitude;

protected:
	bool bIsActive;

	float TimeElapsed;

	FGameplayTag ShakeTag;
};
