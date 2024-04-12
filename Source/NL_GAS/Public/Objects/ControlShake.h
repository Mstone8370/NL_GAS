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
	UPROPERTY(BlueprintReadWrite)
	float Duration = 1.f;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UCurveVector> Curve;

	UPROPERTY(BlueprintReadWrite)
	FRotator ShakeMagnitude = FRotator(1.f, 1.f, 0.f);
};

UCLASS()
class NL_GAS_API UControlShake : public UObject
{
	GENERATED_BODY()
	
public:
	UControlShake();

	float Duration;

	UPROPERTY()
	TObjectPtr<UCurveVector> Curve;

	FRotator ShakeMagnitude;

	/**
	* Update time with given DeltaTime and return delta rotation value.
	* @return Whether this Control Shake Object is still active after update.
	*/
	bool UpdateShake(float DeltaTime, FRotator& OutDeltaRotation);

	void Activate(float InDuration, UCurveVector* InCurve, FRotator InShakeMagnitude);

	void Activate(FControlShakeParams Params);

protected:
	bool bIsActive;

	float TimeElapsed;

	FVector CurveValue_Prev;
};
