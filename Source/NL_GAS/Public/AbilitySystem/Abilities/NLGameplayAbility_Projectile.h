// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/NLGameplayAbility_WeaponPrimary.h"
#include "NLGameplayAbility_Projectile.generated.h"

/**
 * 
 */
UCLASS()
class NL_GAS_API UNLGameplayAbility_Projectile : public UNLGameplayAbility_WeaponPrimary
{
	GENERATED_BODY()
	
public:
	// TODO: 현재는 이 C++클래스가 사용되지 않는데, 사용되게 될 경우엔 아래의 변수가 필요해질 예정.
	/*
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (Categories = "Projectile"))
	FGameplayTag ProjectileTag;
	*/
};
