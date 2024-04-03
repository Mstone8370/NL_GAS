// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "NLPlayerComponent.generated.h"

class AWeaponActor;
class ANLPlayerState;
class ANLPlayerCharacter;
class ANLCharacterBase;
class UAbilitySystemComponent;

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

	// 현재 들고있는 무기 정보로 무기의 Hidden 상태와 메시의 애니메이션 업데이트
	void UpdateOwningCharacterMesh(AWeaponActor* OldWeaponActor = nullptr);

private:
	bool bStartupWeaponInitFinished;

	TArray<AWeaponActor*> InitializedStartupWeapons;

public:	
	ANLCharacterBase* GetOwningPlayer() const;
	
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

	bool CanAttack() const;

	UFUNCTION(BlueprintCallable)
	bool CommitWeaponCost();
};
