// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/NLAbilitySystemTypes.h"
#include "NLFunctionLibrary.generated.h"

struct FWeaponInfo;
struct FTaggedAnimMontageInfo;
struct FUIWeaponInfo;

/**
 * 
 */
UCLASS()
class NL_GAS_API UNLFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static const FWeaponInfo* GetWeaponInfoByTag(const UObject* WorldContextObject, const FGameplayTag& WeaponTag);

	static const FTaggedAnimMontageInfo* GetAnimMontageByTag(const UObject* WorldContextObject, const FGameplayTag& WeaponTag, const FGameplayTag& MontageTag);

	UFUNCTION(BlueprintCallable, Category = "NLFunctionLibrary")
	static void ApplyDamageEffect(const FDamageEffectParams& Params);

	UFUNCTION(BlueprintCallable, Category = "NLFunctionLibrary")
	static FString MakeHitboxInfoDataTablePath(const USkeletalMeshComponent* SkeletalMeshComponent);

	UFUNCTION(BlueprintCallable, Category = "NLFunctionLibrary")
	static void LoadHitboxComponents(USkeletalMeshComponent* SkeletalMeshComponent);

	UFUNCTION(BlueprintCallable, Category = "NLFunctionLibrary")
	static bool AssetExists(FString FullPath);
};
