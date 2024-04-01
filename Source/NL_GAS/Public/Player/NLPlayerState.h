// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "NLPlayerState.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class AWeaponActor;

DECLARE_MULTICAST_DELEGATE_OneParam(FCurrentWeaponSlotChangedSignature, uint8 /*NewSlot*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FWeaponTagAddedSignature, const FGameplayTag& /*AddedTag*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FWeaponTagRemovedSignature, const FGameplayTag& /*RemovedTag*/);

/**
 * 
 */
UCLASS()
class NL_GAS_API ANLPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	ANLPlayerState();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FCurrentWeaponSlotChangedSignature CurrentWeaponSlotChanged;
	FWeaponTagAddedSignature WeaponTagAdded;
	FWeaponTagRemovedSignature WeaponTagRemoved;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 MaxSlotSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FGameplayTag> StartupWeapons;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentWeaponSlot)
	uint8 CurrentWeaponSlot;

	UFUNCTION()
	void OnRep_CurrentWeaponSlot(uint8 OldSlot);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	TArray<AWeaponActor*> WeaponActorSlot;

public:
	//~ Begin AbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End AbilitySystemInterface

	FORCEINLINE UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	const int32 GetCurrentWeaponSlot() const { return CurrentWeaponSlot; }

	AWeaponActor* GetCurrentWeapon() const;

	AWeaponActor* GetWeaponAtSlot(uint8 InSlot) const;

	UFUNCTION(BlueprintCallable)
	void ChangeWeaponSlot(int32 NewWeaponSlot);

	const FGameplayTag GetCurrentWeaponTag() const;

	void AddStartupWeapons();
};
