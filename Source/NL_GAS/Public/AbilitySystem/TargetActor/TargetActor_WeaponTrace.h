// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/EngineTypes.h"
#include "Abilities/GameplayAbilityTargetActor_SingleLineTrace.h"
#include "TargetActor_WeaponTrace.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class NL_GAS_API ATargetActor_WeaponTrace : public AGameplayAbilityTargetActor_SingleLineTrace
{
	GENERATED_UCLASS_BODY()
	
protected:
	virtual FHitResult PerformTrace(AActor* InSourceActor) override;
};
