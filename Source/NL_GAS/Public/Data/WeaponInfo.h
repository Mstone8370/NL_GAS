// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "WeaponInfo.generated.h"

class UGameplayAbility;
class UWeaponRecoilPattern;
class UCurveVector;

USTRUCT(BlueprintType)
struct FWeaponSpreadInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Hip = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ADS = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Crouch = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Fall = 4.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Additive_Walk = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Additive_Recoil = .2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 RecoilOffsetMax = 10;
};

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
	TSubclassOf<UAnimInstance> WeaponAnimBP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UTaggedAnimMontages> AnimMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UAnimInstance> CharacterMeshAnimBP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> PrimaryAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> SecondaryAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> ReloadAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MagSize = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FWeaponSpreadInfo SpreadInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "FOV"))
	FGameplayTag IronsightADSFOVTag;
};

UCLASS()
class NL_GAS_API UTaggedWeaponInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FWeaponInfo WeaponInfo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Weapon"))
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

USTRUCT(BlueprintType)
struct FTaggedAnimMontageInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> ArmsAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UAnimMontage> WeaponAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float PlayLengthOverride = 0.f;
};

UCLASS()
class NL_GAS_API UTaggedAnimMontages : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, FTaggedAnimMontageInfo> Data;
};

USTRUCT(BlueprintType)
struct FUIWeaponInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName WeaponName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName WeaponNameShort;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UTexture2D> WeaponTexture;
};

UCLASS()
class NL_GAS_API UUITaggedWeaponInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<FGameplayTag, FUIWeaponInfo> Data;

	const FUIWeaponInfo* FindUIWeaponInfoByTag(const FGameplayTag& InWeaponTag) const;
};
