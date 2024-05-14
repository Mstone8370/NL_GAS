// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "DamageTextWidgetComponent.generated.h"

/**
 * 
 */
UCLASS()
class NL_GAS_API UDamageTextWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintNativeEvent)
	void UpdateValue(float InValue, bool bIsCriticalHit);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float UpdateWaitTime = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float LifeTime = 1.f;

protected:
	virtual void Initialize();

	UFUNCTION(BlueprintNativeEvent)
	void OnStopWatingUpdate();

	UFUNCTION()
	void OnLifeTimeExpired();

	UPROPERTY(BlueprintReadOnly)
	float Value = 0.f;

	FTimerHandle UpdateWaitTimerHandle;

	FTimerHandle LifeTimerHandle;

public:
	bool IsWatingUpdate() const;
};
