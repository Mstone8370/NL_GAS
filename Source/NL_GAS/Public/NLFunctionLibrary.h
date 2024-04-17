// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
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

};
