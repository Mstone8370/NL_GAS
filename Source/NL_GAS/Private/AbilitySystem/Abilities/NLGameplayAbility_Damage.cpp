// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/NLGameplayAbility_Damage.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/NLAbilitySystemTypes.h"
#include "AbilitySystemComponent.h"
#include "NLGameplayTags.h"

void UNLGameplayAbility_Damage::CauseDamage(FHitResult InHitResult)
{
    if (!IsValid(InHitResult.GetActor()))
    {
        return;
    }

    FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass);

    const float DamageMagnitude = DamageScalableFloat.GetValueAtLevel(InHitResult.Distance);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Attribute_Meta_IncomingDamage, DamageMagnitude);
        
    if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InHitResult.GetActor()))
    {
        GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
    }
}

FDamageEffectParams UNLGameplayAbility_Damage::MakeDamageEffectParams(AActor* OtherActor, FHitResult InHitResult) const
{
    FDamageEffectParams Params;

    Params.DamageGameplayEffectClass = DamageEffectClass;
    Params.SourceASC = GetAbilitySystemComponentFromActorInfo();
    Params.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
    Params.HitResult = InHitResult;
    Params.DamageType = DamageType;
    Params.bCanCriticalHit = bCanCriticalHit;
    Params.DamageScalableFloat = DamageScalableFloat;
    Params.TravelDistance = InHitResult.Distance;
    Params.KnockbackMagnitude = KnockbackForceMagnitude;
    Params.bIsRadialDamage = bIsRadialDamage;
    Params.RadialDamageOrigin = InHitResult.Location;
    Params.RadialDamageInnerRadius = RadialDamageInnerRadius;
    Params.RadialDamageOuterRadius = RadialDamageOuterRadius;
    Params.AimPunchMagnitude = AimPunchMagnitude;

    return Params;
}
