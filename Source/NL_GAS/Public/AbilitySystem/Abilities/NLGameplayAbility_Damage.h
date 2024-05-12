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
	* 1. �����Ƽ�� ������ ������� �ʾƾ��ϰ�, �������� ���� ������ ��ǥ�� ���� ���� �����ؾ���.
	*    ���� ��쿡 ���� �ٸ� ���� �����ؾ��� �ʿ䰡 ����.
	* 2. �� �����Ƽ�� ���� �������� �ִ°� �ƴ϶�, �߻�ü�� �����ؼ� �߻�ü�� �������� �־���ϴ� ���
	*    �߻�ü�� �� �����Ƽ�� Ư������ �����ϰ� �� �ڿ� �߻�ü�� ��ǥ�� ������ �� ���� �̿��ؼ�
	*    �����÷��� ����Ʈ Context�� �����ؾ� ��.
	* ���� �����÷��� ����Ʈ Context�� ������ �� �ʿ��� ������ ����ü�� ��� ����.
	* Context�� ������ �� ��쿡 ���� �� ����ü�� ���� ������ ��Ȳ�� �°� �����ؾ��� �ʿ䰡 ����.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FDamageEffectParams MakeDamageEffectParams(AActor* OtherActor = nullptr, FHitResult InHitResult = FHitResult());
};
