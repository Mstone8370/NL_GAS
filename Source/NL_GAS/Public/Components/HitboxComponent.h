// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "HitboxComponent.generated.h"

/**
 * 
 */
UCLASS( meta = (BlueprintSpawnableComponent) )
class NL_GAS_API UHitboxComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UHitboxComponent();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsWeakHitbox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bIsWeakHitbox"))
	float CriticalHitDamageMultiplier;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool IsWeakHitbox() const { return bIsWeakHitbox; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetCriticalHitDamageMultiplier() const { return CriticalHitDamageMultiplier; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetIsWeakHitbox(bool bInIsWeakHitbox) { bIsWeakHitbox = bInIsWeakHitbox; }
};
