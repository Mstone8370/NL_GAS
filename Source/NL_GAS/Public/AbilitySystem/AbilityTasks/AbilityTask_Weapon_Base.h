// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_Weapon_Base.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponTargetDataSignature, const FGameplayAbilityTargetDataHandle&, DataHandle);

/**
 * 
 */
UCLASS()
class NL_GAS_API UAbilityTask_Weapon_Base : public UAbilityTask
{
	GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable)
    FWeaponTargetDataSignature ValidData;

    UPROPERTY(BlueprintAssignable)
    FWeaponTargetDataSignature Cancelled;

    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "WaitWeaponTargetData", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
    static UAbilityTask_Weapon_Base* CreateWeaponTargetData(UGameplayAbility* OwningAbility, float InTraceLength, uint8 InTraceCount);

    float TraceLength;

    uint8 TraceCount;

    UPROPERTY(BlueprintReadOnly)
    FGameplayAbilityTargetDataHandle ReceivedDataHandle;

private:
    // Async task
    virtual void Activate() override;

    virtual void SendWeaponTargetData();

    void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag);

    void OnTargetDataReplicatedCancelledCallback();

protected:
    virtual void SingleLineTrace(const UObject* WorldContextObject, TArray<AActor*> ActorsToIgnore, FHitResult& OutHitResult, FVector TraceStart, FVector TraceEnd, float Length);
};
