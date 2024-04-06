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

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	FGameplayTag WeaponTag;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<USkeletalMesh> ViewWeaponMesh;

	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<UAnimInstance> ArmsAnimLayerClass;

	int32 MagSize;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentBulletNum)
	int32 CurrentBulletNum;

	UFUNCTION()
	void OnRep_CurrentBulletNum(int32 OldNum);

	bool bIsInitialized;

	bool bIsEquipped;

	EReloadState ReloadState;

public:
	void InitalizeWeapon(const FGameplayTag& InWeaponTag);

	FORCEINLINE bool IsEquipped() const { return bIsEquipped; }

	FORCEINLINE bool IsInitialized() const { return bIsInitialized; }

	FORCEINLINE USkeletalMesh* GetViewWeaponMesh() const { return ViewWeaponMesh; }

	FORCEINLINE TSubclassOf<UAnimInstance> GetArmsAnimLayerClass() const { return ArmsAnimLayerClass; }

	FORCEINLINE const FGameplayTag& GetWeaponTag() const { return WeaponTag; }

	FORCEINLINE bool IsMagEmpty() const { return CurrentBulletNum < 1; }

	bool CanAttack() const;

	void SetWeaponState(bool bInIsEuipped);

	void Drawn();

	void Holstered();

	bool CommitWeaponCost();
};
