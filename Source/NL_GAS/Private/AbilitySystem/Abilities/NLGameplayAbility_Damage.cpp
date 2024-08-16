// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/NLGameplayAbility_Damage.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/NLAbilitySystemTypes.h"
#include "AbilitySystemComponent.h"
#include "NLGameplayTags.h"
#include "NLFunctionLibrary.h"
#include "Interface/Damageable.h"

void UNLGameplayAbility_Damage::CauseDamage(FHitResult InHitResult)
{
    AActor* OtherActor = InHitResult.GetActor();

    if (!IsValid(OtherActor))
    {
        return;
    }

    FDamageEffectParams DamageEffectParams = MakeDamageEffectParams(OtherActor, InHitResult);

    const float DamageMagnitude = DamageScalableFloat.GetValueAtLevel(InHitResult.Distance);
    
    if (UAbilitySystemComponent* TargetASC = DamageEffectParams.TargetASC)
    {
        FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass);
        UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Attribute_Meta_IncomingDamage, DamageMagnitude);

        UNLFunctionLibrary::ApplyDamageEffect(DamageEffectParams);
    }
    else if (OtherActor->Implements<UDamageable>())
    {
        IDamageable::Execute_OnTakenDamage(OtherActor, DamageMagnitude, DamageEffectParams);
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
    if (GetAvatarActorFromActorInfo())
    {
        Params.bHasDamageOrigin = true;
        Params.DamageOrigin = GetAvatarActorFromActorInfo()->GetActorLocation();
    }

    return Params;
}
