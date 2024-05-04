// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
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

	UPROPERTY(BlueprintReadWrite)
	FRotator ShakeMagnitude = FRotator(1.f, 1.f, 1.f);

	void Clear()
	{
		Duration = 1.f;
		Curve = nullptr;
		ShakeMagnitude = FRotator(1.f, 1.f, 1.f);
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

	void Activate(FControlShakeParams InParams);

	FORCEINLINE bool IsActive() const { return bIsActive; }

	void Clear();

protected:
	bool bIsActive;

	float TimeElapsed;
};
