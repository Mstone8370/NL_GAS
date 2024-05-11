// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AttributeSet/NLAttributeSet.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/NLAbilitySystemTypes.h"

void UNLAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    /**
    * Default Attribute
    */
    DOREPLIFETIME_CONDITION_NOTIFY(UNLAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UNLAttributeSet, Health, COND_None, REPNOTIFY_Always);
}

void UNLAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    if (Attribute == GetHealthAttribute())
    {
        UE_LOG(LogTemp, Warning, TEXT("[PreAttributeChange] Health"));
        NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
    }
}

void UNLAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    if (Attribute == GetHealthAttribute())
    {
        UE_LOG(LogTemp, Warning, TEXT("[PostAttributeChange] Health"));
    }
}

void UNLAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        UE_LOG(LogTemp, Warning, TEXT("[PostGameplayEffectExecute] Health"));
    }
    if (FNLGameplayEffectContext* Context = static_cast<FNLGameplayEffectContext*>(Data.EffectSpec.GetContext().Get()))
    {
        UE_LOG(LogTemp, Warning, TEXT("NLGameplayEffectContext"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Default GameplayEffectContext"));
    }
}

void UNLAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldHealth) const
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNLAttributeSet, MaxHealth, OldHealth);
}

void UNLAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNLAttributeSet, Health, OldHealth);
}
