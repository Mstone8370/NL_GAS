// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "PlayerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class NL_GAS_API IPlayerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanSwapWeaponSlot(int32 NewSlot);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void TrySwapWeaponSlot(int32 NewSlot);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void GetWeaponHandIKLocation(FName LeftIKSocketName, FName RightIKSocketName, FVector& OutLeftIKLocation, FVector& OutRightIKLocation) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	float PlayCurrentWeaponMontage(const FGameplayTag& MontageTag);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void WeaponFired(TSubclassOf<UCameraShakeBase> CameraShakeBaseClass);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool StartReload();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnWeaponReloadStateChanged(const FGameplayTag& WeaponTag, const FGameplayTag& StateTag);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	float GetWeaponSpreadValue();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CommitWeaponCost(bool& bIsLast);
};
