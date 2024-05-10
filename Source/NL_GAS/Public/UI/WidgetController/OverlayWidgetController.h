// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/NLWidgetController.h"
#include "GameplayTagContainer.h"
#include "OverlayWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponSlotUpdatedSignature, const TArray<FGameplayTag>&, WeaponSlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCurrentWeaponUpdatedSignature, const FGameplayTag&, WeaponTag, const int32, SlotNum);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBulletNumUpdatedSignature, const int32, BulletNum);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttributeUpdatedSignature, float, Value);

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class NL_GAS_API UOverlayWidgetController : public UNLWidgetController
{
	GENERATED_BODY()

public:
	virtual void BindEvents() override;

	virtual void BroadcastInitialValues() override;

	UPROPERTY(BlueprintAssignable)
	FWeaponSlotUpdatedSignature WeaponSlotUpdated;

	UPROPERTY(BlueprintAssignable)
	FCurrentWeaponUpdatedSignature CurrentWeaponUpdated;

	UPROPERTY(BlueprintAssignable)
	FBulletNumUpdatedSignature BulletNumUpdated;

	UPROPERTY(BlueprintAssignable)
	FAttributeUpdatedSignature HealthUpdated;

	UPROPERTY(BlueprintAssignable)
	FAttributeUpdatedSignature MaxHealthUpdated;

protected:
	UFUNCTION()
	void OnWeaponSlotChanged(const TArray<FGameplayTag>& WeaponTagSlot);

	UFUNCTION()
	void OnWeaponSwapped(const FGameplayTag& FromWeaponTag, int32 FromSlotNum, const FGameplayTag& ToWeaponTag, int32 ToSlotNum);
	
	UFUNCTION()
	void OnCurrentWeaponBulletNumChanged(int32 NewBulletNum);
};
