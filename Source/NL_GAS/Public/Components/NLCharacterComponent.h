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

USTRUCT(BlueprintType)
struct FWeaponSlot
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 MaxSlotSize = 3;

	UPROPERTY(BlueprintReadOnly)
	uint8 CurrentSlot = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<AWeaponActor*> WeaponActorSlot;
};

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FGameplayTag> WeaponTagSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<class UAnimMontage> MontageTemp;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_WeaponSlot)
	FWeaponSlot WeaponSlot;

	UFUNCTION()
	void OnRep_WeaponSlot(const FWeaponSlot& OldSlot);

	// 현재 들고있는 무기 정보로 뷰 메시와 3인칭 캐릭터 메시 업데이트
	void UpdateMeshes(AWeaponActor* OldWeaponActor = nullptr, bool bIsSimulated = false);

	// 현재 들고있는 무기 정보로 3인칭 캐릭터 메시의 애니메이션 업데이트
	void UpdateOwningCharacterMesh(AWeaponActor* OldWeaponActor = nullptr);

	UPROPERTY(BlueprintReadOnly)
	bool bIsSwappingWeapon;

	uint8 WeaponSwapPendingSlot;

	FTimerHandle HolsterTimerHandle;
	FTimerHandle DrawTimerHandle;

	void RevertWeaponSwap();

	void OnWeaponHolstered();

	void OnWeaponDrawn();

	UFUNCTION()
	void OnWeaponBulletNumChanged(const AWeaponActor* Weapon, int32 NewBulletNum);

	void BindWeaponDelegate(AWeaponActor* Weapon);

	void UnBindWeaponDelegate(AWeaponActor* Weapon);

	void AttachWeaponToSocket(AWeaponActor* Weapon);

	void AttachWeaponToHand(AWeaponActor* Weapon);

	void ClearWeapons();

	/* 이 함수는 const가 아님에 주의해야함. current slot이 변경될 수 있음. */
	void OnCurrentWeaponDropped(); // for local controlled player

private:
	bool bStartupWeaponInitFinished;

	UPROPERTY()
	TArray<AWeaponActor*> InitializedStartupWeapons;

	UPROPERTY()
	TMap<FName, AWeaponActor*> WeaponSlotSocketMap;

public:	
	ANLCharacterBase* GetOwningCharacter() const;

	ANLPlayerCharacter* GetOwningPlayer() const;
	
	ANLPlayerState* GetOwningPlayerState() const;

	UAbilitySystemComponent* GetASC() const;

	UNLAbilitySystemComponent* GetNLASC() const;

	void UpdateWeaponTagSlot();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetWeaponSlotSize() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetWeaponSlotMaxSize() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AWeaponActor* GetWeaponActorAtSlot(uint8 Slot) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AWeaponActor* GetCurrentWeaponActor() const;

	FORCEINLINE int32 GetCurrentWeaponSlot() const { return WeaponSlot.CurrentSlot; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AWeaponActor* GetEquippedWeaponActorByTag(const FGameplayTag& WeaponTag) const;

	const FGameplayTag GetWeaponTagAtSlot(uint8 Slot) const;

	const FGameplayTag GetCurrentWeaponTag() const;

	void WeaponAdded(AWeaponActor* Weapon);

	void ValidateStartupWeapons();

	bool CanSwapWeaponSlot(int32 NewWeaponSlot) const;

	void TrySwapWeaponSlot(int32 NewWeaponSlot, bool bCheckCondition = true, bool bSkipHolsterAnim = false);

	void TrySwapWeaponSlot_Next(bool bPrev = false);

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

	float GetCurrentWeaponSpreadValue(bool bADS, bool bFalling, bool bCrouched, float CharacterSpeedSquared, int32 RecoilOffset) const;

	const FGameplayTag GetCurrentWeaponADSFOVTag() const;

	UFUNCTION(BlueprintCallable)
	void LowerWeapon();

	UFUNCTION(BlueprintCallable)
	void RaiseWeapon();

	void CheckCurrentWeaponReloadState();

	void HandleOwnerDeath();

	UFUNCTION(BlueprintCallable)
	void DropCurrentWeapon();

	void PickUp(AActor* Pickupable);

	void PickUpWeapon(AWeaponActor* WeaponActor);

	bool IsWeaponSlotFull() const;

	bool IsWeaponSlotEmpty() const;

	uint8 GetFirstEmptySlot() const;
};
