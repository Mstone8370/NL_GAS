// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "ScalableFloat.h"
#include "NLAbilitySystemTypes.generated.h"

class UGameplayEffect;

/**
* FGameplayEffectContext를 상속받아서 추가 정보를 전달.
*/
USTRUCT(BlueprintType)
struct FNLGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	virtual UScriptStruct* GetScriptStruct() const
	{
		return FNLGameplayEffectContext::StaticStruct();
	}

	virtual FNLGameplayEffectContext* Duplicate() const
	{
		FNLGameplayEffectContext* NewContext = new FNLGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	TSharedPtr<FGameplayTag> DamageType;

	UPROPERTY()
	bool bCanCriticalHit = false;

	UPROPERTY()
	bool bIsRadialDamage = false;

	UPROPERTY()
	FVector RadialDamageOrigin = FVector::ZeroVector;

	UPROPERTY()
	float RadialDamageInnerRadius = 0.f;

	UPROPERTY()
	float RadialDamageOuterRadius = 0.f;

	UPROPERTY()
	float KnockbackMagnitude = 0.f;
};

template<>
struct TStructOpsTypeTraits< FNLGameplayEffectContext > : public TStructOpsTypeTraitsBase2< FNLGameplayEffectContext >
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true		// Necessary so that TSharedPtr<FHitResult> Data is copied around
	};
};


/**
* 데미지 게임플레이 이펙트에 사용되는 다양한 파라미터를 묶은 구조체.
*/
USTRUCT(BlueprintType)
struct FDamageEffectParams
{
	GENERATED_BODY()

	FDamageEffectParams() {}

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> SourceASC = nullptr;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> TargetASC = nullptr;

	UPROPERTY(BlueprintReadWrite)
	FHitResult HitResult = FHitResult();

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag DamageType = FGameplayTag();

	UPROPERTY(BlueprintReadWrite)
	bool bCanCriticalHit = false;

	UPROPERTY(BlueprintReadWrite)
	FScalableFloat DamageScalableFloat = FScalableFloat();

	UPROPERTY(BlueprintReadWrite)
	float TravelDistance = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float KnockbackMagnitude = 0.f;

	UPROPERTY(BlueprintReadWrite)
	bool bIsRadialDamage = false;

	UPROPERTY(BlueprintReadWrite)
	FVector RadialDamageOrigin = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	float RadialDamageInnerRadius = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float RadialDamageOuterRadius = 0.f;

	UPROPERTY(BlueprintReadWrite)
	bool bHasDamageOrigin = false;

	UPROPERTY(BlueprintReadWrite)
	FVector DamageOrigin = FVector::ZeroVector;
};
