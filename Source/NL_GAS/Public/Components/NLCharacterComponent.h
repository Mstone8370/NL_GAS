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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponSlotChangedSignature, const TArray<FGameplayTag>&, WeaponSlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FWeaponSwappedSignature, const FGameplayTag&, FromWeaponTag, int32, FromSlotNum, const FGameplayTag&, ToWeaponTag, int32, ToSlotNum);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCurrentWeaponBulletNumChangedSignature, int32, NewBulletNum);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NL_GAS_API UNLCharacterComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UNLCharacterComponent();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void AddStartupWeapons();

	FWeaponSlotChangedSignature WeaponSlotChanged;
	FWeaponSwappedSignature WeaponSwapped;
	FCurrentWeaponBulletNumChangedSignature CurrentWeaponBulletNumChanged;

	UPROPERTY(BlueprintReadOnly)
	TArray<FGameplayTag> WeaponTagSlot;

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

	UPROPERTY(BlueprintReadOnly)
	bool bIsSwappingWeapon;

	uint8 WeaponSwapPendingSlot;

	FTimerHandle HolsterTimerHandle;
	FTimerHandle DrawTimerHandle;

	void CancelWeaponSwap();

	void OnWeaponHolstered();

	void OnWeaponDrawn();

	UFUNCTION()
	void OnWeaponBulletNumChanged(const AWeaponActor* Weapon, int32 NewBulletNum);

	void BindWeaponDelegate(AWeaponActor* Weapon);

	void UnBindWeaponDelegate(AWeaponActor* Weapon);

private:
	bool bStartupWeaponInitFinished;

	TArray<AWeaponActor*> InitializedStartupWeapons;

public:	
	ANLCharacterBase* GetOwningCharacter() const;

	ANLPlayerCharacter* GetOwningPlayer() const;
	
	ANLPlayerState* GetOwningPlayerState() const;

	UAbilitySystemComponent* GetASC() const;

	UNLAbilitySystemComponent* GetNLASC() const;

	void UpdateWeapnTagSlot();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetWeaponSlotSize() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetWeaponSlotMaxSize() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AWeaponActor* GetWeaponActorAtSlot(uint8 Slot) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AWeaponActor* GetCurrentWeaponActor() const;

	FORCEINLINE int32 GetCurrentWeaponSlot() const { return CurrentWeaponSlot; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AWeaponActor* GetEquippedWeaponActorByTag(const FGameplayTag& WeaponTag) const;

	const FGameplayTag GetWeaponTagAtSlot(uint8 Slot) const;

	const FGameplayTag GetCurrentWeaponTag() const;

	void WeaponAdded(AWeaponActor* Weapon);

	void ValidateStartupWeapons();

	bool CanSwapWeaponSlot(int32 NewWeaponSlot) const;

	void TrySwapWeaponSlot(int32 NewWeaponSlot);

	bool CanAttack() const;

	bool CommitWeaponCost(bool& bIsLast);

	float PlayCurrentWeaponMontage(const FGameplayTag& MontageTag, FName StartSectionName = NAME_None);

	float PlayCurrentWeaponMontageAndSetCallback(const FGameplayTag& MontageTag, FTimerHandle& OutTimerHandle, FTimerDelegate TimerDelegate, bool bOnBlendOut = true, FName StartSectionName = NAME_None);

	UFUNCTION(BlueprintCallable)
	bool IsCurrentWeaponMagEmpty() const;

	UFUNCTION(BlueprintCallable)
	bool CanReload() const;

	UFUNCTION(BlueprintCallable)
	bool StartReload();

	void OnWeaponReloadStateChanged(const FGameplayTag& WeaponTag, const FGameplayTag& StateTag);

	float GetCurrentWeaponSpreadValue(bool bADS, bool bFalling, bool bCrouched, float CharacterSpeedSquared, int32 RecoilOffset, bool bVisual = true) const;
};
