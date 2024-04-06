// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "WeaponInfo.generated.h"

class UGameplayAbility;

/**
 * Single Weapon Info with Gameplay Tag
 */
USTRUCT(BlueprintType)
struct FWeaponInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName WeaponName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<USkeletalMesh> ViewModelMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> PropMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAnimInstance> ArmsAnimLayerClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWeaponAnimInfo> WeaponAnimInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWeaponAnimInfo> ArmsAnimInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAnimInstance> CharacterMeshAnimBP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> PrimaryAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> SecondaryAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> ReloadAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MagSize;
};

UCLASS()
class NL_GAS_API UTaggedWeaponInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FWeaponInfo WeaponInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag WeaponTag;
};

UCLASS()
class NL_GAS_API UTaggedWeaponInfoList : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TObjectPtr<UTaggedWeaponInfo>> TaggedWeaponInfos;

	const FWeaponInfo* FindTaggedWeaponInfoByTag(const FGameplayTag& InWeaponTag) const;
};

/**
 * Single Weapon Animation Montage Info with Gameplay Tag
 */
USTRUCT(BlueprintType)
struct FWeaponAnims
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAnimInstance> AnimBP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> Draw;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DrawTime = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> DrawFirst;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DrawFirstTime = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> Holster;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float HolsterTime = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> PrimaryAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> SecondaryAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> ReloadLong;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ReloadLongTime = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> ReloadShort;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ReloadShortTime = 0.f;
};

UCLASS()
class NL_GAS_API UWeaponAnimInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FWeaponAnims WeaponAnimInfo;
};
