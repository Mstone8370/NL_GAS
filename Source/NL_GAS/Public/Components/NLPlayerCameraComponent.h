// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "NLPlayerCameraComponent.generated.h"

/**
 * 
 */
UCLASS()
class NL_GAS_API UNLPlayerCameraComponent : public UCameraComponent
{
	GENERATED_BODY()
	
public:
	UNLPlayerCameraComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InterpSpeed;

protected:
	void InterpFOV(float DeltaTime);

	float BaseFOV;

	float TargetFOV;

	float CurrentFOV;

	bool bDoInterp;

	float CurrentInterpSpeed;

public:
	UFUNCTION(BlueprintCallable)
	void SetBaseFOV(float InBaseFOV);

	UFUNCTION(BlueprintCallable)
	void SetTargetFOV(float InTargetFOV, float TransientInterpSpeed = -1.f);

	FORCEINLINE float GetBaseFOV() const { return BaseFOV; }

};
