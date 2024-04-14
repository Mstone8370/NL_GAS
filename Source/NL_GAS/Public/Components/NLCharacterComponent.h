// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "NLCharacterComponent.generated.h"

class AWeaponActor;
class ANLPlayerState;
class ANLPlayerCharacter;
class ANLCharacterBase;
class UAbilitySystemComponent;
class UNLAbilitySystemComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NL_GAS_API UNLCharacterComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UNLCharacterComponent();

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

	// ���� ����ִ� ���� ������ ������ Hidden ���¿� �޽��� �ִϸ��̼� ������Ʈ
	void UpdateOwningCharacterMesh(AWeaponActor* OldWeaponActor = nullptr);

	UPROPERTY(BlueprintReadOnly)
	bool bIsSwappingWeapon;

	uint8 WeaponSwapPendingSlot;

	FTimerHandle HolsterTimerHandle;
	FTimerHandle DrawTimerHandle;

	void CancelWeaponSwap();

	void OnWeaponHolstered();

	void OnWeaponDrawn();

private:
	bool bStartupWeaponInitFinished;

	TArray<AWeaponActor*> InitializedStartupWeapons;

public:	
	ANLCharacterBase* GetOwningCharacter() const;

	ANLPlayerCharacter* GetOwningPlayer() const;
	
	ANLPlayerState* GetOwningPlayerState() const;

	UAbilitySystemComponent* GetASC() const;

	UNLAbilitySystemComponent* GetNLASC() const;

	AWeaponActor* GetWeaponActorAtSlot(uint8 Slot) const;

	AWeaponActor* GetCurrentWeaponActor() const;

	AWeaponActor* GetEquippedWeaponActorByTag(const FGameplayTag& WeaponTag) const;

	const FGameplayTag GetWeaponTagAtSlot(uint8 Slot) const;

	const FGameplayTag GetCurrentWeaponTag() const;

	void WeaponAdded(AWeaponActor* Weapon);

	void ValidateStartupWeapons();

	bool CanSwapWeaponSlot(int32 NewWeaponSlot) const;

	void TrySwapWeaponSlot(int32 NewWeaponSlot);

	bool CanAttack() const;

	UFUNCTION(BlueprintCallable)
	bool CommitWeaponCost(bool& bIsLast);

	float PlayCurrentWeaponMontage(const FGameplayTag& MontageTag);

	float PlayCurrentWeaponMontageAndSetCallback(const FGameplayTag& MontageTag, FTimerHandle& OutTimerHandle, FTimerDelegate TimerDelegate, bool bOnBlendOut = true);

	UFUNCTION(BlueprintCallable)
	bool IsCurrentWeaponMagEmpty() const;

	UFUNCTION(BlueprintCallable)
	bool CanReload() const;

	UFUNCTION(BlueprintCallable)
	bool StartReload();

	void OnWeaponReloadStateChanged(const FGameplayTag& WeaponTag, const FGameplayTag& StateTag);
};
