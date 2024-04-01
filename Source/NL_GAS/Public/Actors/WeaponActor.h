// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/WeaponInfo.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpec.h"
#include "WeaponActor.generated.h"

class UGameplayAbility;

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

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	FGameplayTag WeaponTag;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<USkeletalMesh> ViewWeaponMesh;

	bool bInitialized = false;

	bool bIsEquipped = false;

public:
	void InitalizeWeapon(const FGameplayTag& InWeaponTag);

	FORCEINLINE bool IsEquipped() const { return bIsEquipped; }

	FORCEINLINE USkeletalMesh* GetViewWeaponMesh() const { return ViewWeaponMesh; }

	FORCEINLINE const FGameplayTag& GetWeaponTag() const { return WeaponTag; }

	void SetWeaponState(bool bInIsEuipped);
};
