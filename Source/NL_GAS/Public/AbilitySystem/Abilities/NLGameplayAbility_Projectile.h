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
	// TODO: ����� �� C++Ŭ������ ������ �ʴµ�, ���ǰ� �� ��쿣 �Ʒ��� ������ �ʿ����� ����.
	/*
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (Categories = "Projectile"))
	FGameplayTag ProjectileTag;
	*/
};
