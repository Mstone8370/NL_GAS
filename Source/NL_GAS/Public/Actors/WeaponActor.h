// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/WeaponInfo.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"
#include "Abstract/Interactable.h"
#include "WeaponActor.generated.h"

class UGameplayAbility;
class USphereComponent;

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
class NL_GAS_API AWeaponActor : public AInteractable
{
	GENERATED_BODY()
	
public:
	AWeaponActor();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRep_AttachmentReplication() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> WeaponMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> PickUpCollision;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, meta = (Categories = "Weapon"))
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

	virtual void OnRep_IsInteracting() override;

	virtual void OnRep_Owner() override;

	bool bIsTacticalReload;

	EReloadState ReloadState;

	FGameplayTagContainer AttachmentTags;

	void SetBulletNum_Internal(int32 NewBulletNum);

	FGameplayTag IronsightADSFOVTag;

public:
	UFUNCTION(BlueprintCallable)
	void InitializeWeapon(const FGameplayTag& InWeaponTag, bool bForceInit = false);
	
	//~Begin Interactable
public:
	virtual bool CanInteract() const override;
protected:
	virtual void OnStartInteraction_Implementation(APawn* Interactor);
	virtual void OnEndInteraction_Implementation();
	//~End Interactable

public:
	FORCEINLINE bool IsEquipped() const { return bIsInteracting; }

	FORCEINLINE bool IsInitialized() const { return bIsInitialized; }

	FORCEINLINE bool IsEverDrawn() const { return bIsEverDrawn; }

	FORCEINLINE USkeletalMesh* GetViewWeaponMesh() const { return ViewWeaponMesh; }

	FORCEINLINE TSubclassOf<UAnimInstance> GetWeaponAnimInstanceClass() const { return WeaponAnimInstanceClass; }

	FORCEINLINE TSubclassOf<UAnimInstance> GetArmsAnimLayerClass() const { return ArmsAnimLayerClass; }

	FORCEINLINE const FGameplayTag& GetWeaponTag() const { return WeaponTag; }

	const FGameplayTag& GetADSFOVTag() const;

	FORCEINLINE int32 GetCurrentBulletNum() const { return CurrentBulletNum; }

	FORCEINLINE bool IsMagEmpty() const { return CurrentBulletNum < 1; }

	bool CanAttack() const;

	void SetWeaponState(bool bInIsEuipped);

	void Drawn();

	void Holstered();

	// Same with AInteractable::StartInteraction
	void PickedUp(APawn* Interactor);

	// Same with AInteractable::EndInteraction
	void Dropped();

	bool CommitWeaponCost();

	FORCEINLINE bool CanReload() const { return CurrentBulletNum < MagSize || ReloadState < EReloadState::None; }

	void ReloadStateChanged(const FGameplayTag& StateTag);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool IsReloading() const { return ReloadState < EReloadState::None; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool IsTacticalReload() const { return bIsTacticalReload; }

	FORCEINLINE EReloadState GetReloadState() const { return ReloadState; }

	const FWeaponSpreadInfo* GetSpreadInfo() const;

	void CheckReloadState();
};
