// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/NLGameplayAbility.h"
#include "NLGameplayAbility_Damage.generated.h"

struct FDamageEffectParams;

/**
 * 
 */
UCLASS()
class NL_GAS_API UNLGameplayAbility_Damage : public UNLGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	FScalableFloat DamageScalableFloat;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	FGameplayTag DamageType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	bool bCanCriticalHit = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float KnockbackForceMagnitude = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float AimPunchMagnitude = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	bool bIsRadialDamage = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage|RadialDamage")
	float RadialDamageInnerRadius = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage|RadialDamage")
	float RadialDamageOuterRadius = 0.f;

	UFUNCTION(BlueprintCallable)
	void CauseDamage(FHitResult InHitResult);

	/**
	* 1. 어빌리티의 변수는 변경되지 않아야하고, 데미지를 가할 때에는 목표에 따라 값을 설정해야함.
	*    따라서 경우에 따라 다른 값을 설정해야할 필요가 있음.
	* 2. 이 어빌리티로 직접 데미지를 주는게 아니라, 발사체를 스폰해서 발사체가 데미지를 주어야하는 경우
	*    발사체에 이 어빌리티의 특성값을 저장하게 한 뒤에 발사체가 목표에 닿으면 그 값을 이용해서
	*    게임플레이 이펙트 Context를 설정해야 함.
	* 따라서 게임플레이 이펙트 Context를 설정할 때 필요한 값들을 구조체로 묶어서 관리.
	* Context를 설정할 때 경우에 따라서 이 구조체에 없는 정보를 상황에 맞게 설정해야할 필요가 있음.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FDamageEffectParams MakeDamageEffectParams(AActor* OtherActor = nullptr, FHitResult InHitResult = FHitResult());
};
