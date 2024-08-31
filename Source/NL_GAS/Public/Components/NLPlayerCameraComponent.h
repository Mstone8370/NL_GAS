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

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> HealthPPMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float HealthPPMatThresholdMax;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float HealthPPMatThresholdMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InterpSpeed;

protected:
	void InterpFOV(float DeltaTime);

	float BaseFOV;

	float TargetFOV;

	float CurrentFOV;

	bool bDoInterp;

	float CurrentInterpSpeed;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UMaterialInstanceDynamic> HealthPPMatInst;

public:
	UFUNCTION(BlueprintCallable)
	void SetBaseFOV(float InBaseFOV);

	UFUNCTION(BlueprintCallable)
	void SetTargetFOV(float InTargetFOV, float TransientInterpSpeed = -1.f);

	FORCEINLINE float GetBaseFOV() const { return BaseFOV; }

	void OnPlayerHealthChanged(float HealthPercent);
};
