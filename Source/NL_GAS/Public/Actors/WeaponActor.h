// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/WeaponInfo.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"
#include "WeaponActor.generated.h"

class UGameplayAbility;

UENUM(BlueprintType)
enum EReloadState : uint8
{
	MagOut,
	MagIn,
	None,

	MAX
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBulletNumChangedSignature, const AWeaponActor*, Weapon, int32, NewBulletNum);

UCLASS(Blueprintable, BlueprintType)
class NL_GAS_API AWeaponActor : public AActor
{
	GENERATED_BODY()
	
public:
	AWeaponActor();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> WeaponMeshComponent;

	TSubclassOf<UGameplayAbility> PrimaryAbilityClass;
	FGameplayAbilitySpecHandle PrimaryAbilitySpecHandle;

	TSubclassOf<UGameplayAbility> SecondaryAbilityClass;
	FGameplayAbilitySpecHandle SecondaryAbilitySpecHandle;

	TSubclassOf<UGameplayAbility> ReloadAbilityClass;
	FGameplayAbilitySpecHandle ReloadAbilitySpecHandle;

	UPROPERTY(BlueprintReadOnly)
	FName WeaponName;

	FBulletNumChangedSignature BulletNumChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	FGameplayTag WeaponTag;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<USkeletalMesh> ViewWeaponMesh;

	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<UAnimInstance> WeaponAnimInstanceClass;

	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<UAnimInstance> ArmsAnimLayerClass;

	int32 MagSize;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentBulletNum)
	int32 CurrentBulletNum;

	UPROPERTY()
	FWeaponSpreadInfo SpreadInfo;

	UFUNCTION()
	void OnRep_CurrentBulletNum(int32 OldNum);

	bool bIsInitialized;

	bool bIsEverDrawn;

	bool bIsEquipped;

	bool bIsTacticalReload;

	EReloadState ReloadState;

	FGameplayTagContainer AttachmentTags;

	void SetBulletNum_Internal(int32 NewBulletNum);

public:
	void InitalizeWeapon(const FGameplayTag& InWeaponTag);

	FORCEINLINE bool IsEquipped() const { return bIsEquipped; }

	FORCEINLINE bool IsInitialized() const { return bIsInitialized; }

	FORCEINLINE bool IsEverDrawn() const { return bIsEverDrawn; }

	FORCEINLINE USkeletalMesh* GetViewWeaponMesh() const { return ViewWeaponMesh; }

	FORCEINLINE TSubclassOf<UAnimInstance> GetWeaponAnimInstanceClass() const { return WeaponAnimInstanceClass; }

	FORCEINLINE TSubclassOf<UAnimInstance> GetArmsAnimLayerClass() const { return ArmsAnimLayerClass; }

	FORCEINLINE const FGameplayTag& GetWeaponTag() const { return WeaponTag; }

	FORCEINLINE int32 GetCurrentBulletNum() const { return CurrentBulletNum; }

	FORCEINLINE bool IsMagEmpty() const { return CurrentBulletNum < 1; }

	bool CanAttack() const;

	void SetWeaponState(bool bInIsEuipped);

	void Drawn();

	void Holstered();

	bool CommitWeaponCost();

	FORCEINLINE bool CanReload() const { return CurrentBulletNum < MagSize || ReloadState < EReloadState::None; }

	void ReloadStateChanged(const FGameplayTag& StateTag);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool IsReloading() const { return ReloadState < EReloadState::None; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool IsTacticalReload() const { return bIsTacticalReload; }

	FORCEINLINE EReloadState GetReloadState() const { return ReloadState; }

	const FWeaponSpreadInfo* GetSpreadInfo() const;
};
