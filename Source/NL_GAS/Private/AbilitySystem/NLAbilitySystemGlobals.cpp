// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/NLAbilitySystemGlobals.h"

#include "AbilitySystem/NLAbilitySystemTypes.h"

FGameplayEffectContext* UNLAbilitySystemGlobals::AllocGameplayEffectContext() const
{
    return new FNLGameplayEffectContext();
}
