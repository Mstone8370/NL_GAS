// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
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

	UPROPERTY()
	bool bIsCriticalHit = false;

	UPROPERTY()
	bool bIsRadialDamage = false;

	TSharedPtr<FGameplayTag> DamageType;

	UPROPERTY()
	float KnockbackMagnitude = 0.f;

	UPROPERTY()
	float AimPunch = 0.f;
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

	FDamageEffectParams()
		: DamageGameplayEffectClass(nullptr)
		, SourceASC(nullptr)
		, TargetASC(nullptr)
		, BaseDamage(0.f)
		, DamageType(FGameplayTag())
		, KnockbackMagnitude(0.f)
		, bIsRadialDamage(false)
	{}

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> SourceASC;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> TargetASC;

	UPROPERTY(BlueprintReadWrite)
	FHitResult HitResult;

	UPROPERTY(BlueprintReadWrite)
	float BaseDamage;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag DamageType;

	UPROPERTY(BlueprintReadWrite)
	float KnockbackMagnitude;

	UPROPERTY(BlueprintReadWrite)
	bool bIsRadialDamage;
};
