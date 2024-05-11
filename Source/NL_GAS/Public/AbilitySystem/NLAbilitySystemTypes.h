// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "NLAbilitySystemTypes.generated.h"

class UGameplayEffect;

/**
* FGameplayEffectContext�� ��ӹ޾Ƽ� �߰� ������ ����.
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
* ������ �����÷��� ����Ʈ�� ���Ǵ� �پ��� �Ķ���͸� ���� ����ü.
*/
USTRUCT(BlueprintType)
struct FDamageEffectParams
{
	GENERATED_BODY()

	FDamageEffectParams() 
	{}


};
