// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "NLAbilitySystemGlobals.generated.h"

/**
 * 
 */
UCLASS()
class NL_GAS_API UNLAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

	// AbilitySystemComponent���� MakeEffectContext �Լ��� ���ο��� �Ʒ��� �Լ��� FGameplayEffectContext ������.
	// �� �Լ��� override�ؼ� FGameplayEffectContext ��ӹ��� FAuraGameplayEffectContext�� �����ϵ��� �����ϸ� ��� ���� �����.
	// Config/DefaultGame.ini ���� �������� �� UAuraAbilitySystemGlobals�� ����ϰ� ���� ����
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
