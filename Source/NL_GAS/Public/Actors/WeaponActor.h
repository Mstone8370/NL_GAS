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
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> ViewWeaponMesh;

	FGameplayAbilitySpec PrimaryAbilitySpec;

	FGameplayAbilitySpec SecondaryAbilitySpec;

	FGameplayAbilitySpec ReloadAbilitySpec;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	FGameplayTag WeaponTag;

	bool bInitialized = false;

public:
	void InitalizeWeapon(const FGameplayTag& InWeaponTag);

};
