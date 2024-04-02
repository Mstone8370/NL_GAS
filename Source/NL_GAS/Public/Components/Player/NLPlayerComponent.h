// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "NLPlayerComponent.generated.h"

class AWeaponActor;
class ANLPlayerState;
class ANLPlayerCharacter;
class UAbilitySystemComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NL_GAS_API UNLPlayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UNLPlayerComponent();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void AddStartupWeapons();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	uint8 MaxWeaponSlotSize;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentWeaponSlot)
	uint8 CurrentWeaponSlot;

	UFUNCTION()
	void OnRep_CurrentWeaponSlot(uint8 OldSlot);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WeaponActorSlot)
	TArray<AWeaponActor*> WeaponActorSlot;

	UFUNCTION()
	void OnRep_WeaponActorSlot();

private:
	bool bStartupWeaponInitFinished;

	TArray<AWeaponActor*> InitializedStartupWeapons;

public:	
	ANLPlayerCharacter* GetOwningPlayer() const;
	
	ANLPlayerState* GetOwningPlayerState() const;

	UAbilitySystemComponent* GetASC() const;

	AWeaponActor* GetWeaponActorAtSlot(uint8 Slot) const;

	AWeaponActor* GetCurrentWeaponActor() const;

	const FGameplayTag GetWeaponTagAtSlot(uint8 Slot) const;

	const FGameplayTag GetCurrentWeaponTag() const;

	void WeaponAdded(AWeaponActor* Weapon);

	void ValidateStartupWeapons();

	UFUNCTION(BlueprintCallable)
	void ChangeWeaponSlot_Simple(int32 NewWeaponSlot);
};
