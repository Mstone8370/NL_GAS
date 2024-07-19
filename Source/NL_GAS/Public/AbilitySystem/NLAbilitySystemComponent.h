// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "NLAbilitySystemComponent.generated.h"

class AWeaponActor;

/**
 * 
 */
UCLASS()
class NL_GAS_API UNLAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	void AbilityInputTagPressed(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);
	
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	void ClearAbilityInput();

	void AddAbilities(const TMap<FGameplayTag, TSubclassOf<UGameplayAbility>>& Abilities);

	void WeaponAdded(AWeaponActor* Weapon);

	void WeaponDropped(AWeaponActor* Weapon);

	void WeaponHolstered(const AWeaponActor* Weapon);

	void WeaponDrawn(const AWeaponActor* Weapon);

protected:
	virtual void OnRep_ActivateAbilities() override;

	// Handles to abilities that had their input pressed this frame.
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	// Handles to abilities that had their input released this frame.
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// Handles to abilities that have their input held.
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
};
