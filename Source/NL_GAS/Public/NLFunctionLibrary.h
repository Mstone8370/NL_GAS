// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "NLFunctionLibrary.generated.h"

struct FTaggedWeaponInfo;

/**
 * 
 */
UCLASS()
class NL_GAS_API UNLFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static const FTaggedWeaponInfo* GetWeaponInfoByTag(const UObject* WorldContextObject, const FGameplayTag& WeaponTag);
};
