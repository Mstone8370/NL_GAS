// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/HitboxComponent.h"

#include "Net/UnrealNetwork.h"

UHitboxComponent::UHitboxComponent()
    : bIsWeakHitbox(false)
    , CriticalHitDamageMultiplier(2.f)
    , HitboxExtent(FVector::OneVector)
{
    SetIsReplicatedByDefault(true);

    SetCollisionProfileName(FName("Hitbox"));
}

void UHitboxComponent::OnRep_HitboxExtent()
{
    SeHitboxExtent(HitboxExtent);
}

void UHitboxComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(UHitboxComponent, bIsWeakHitbox, COND_InitialOnly);
    DOREPLIFETIME_CONDITION_NOTIFY(UHitboxComponent, HitboxExtent, COND_InitialOnly, REPNOTIFY_OnChanged);
}

void UHitboxComponent::SeHitboxExtent(FVector NewHitboxExtent)
{
    HitboxExtent = NewHitboxExtent;
    SetBoxExtent(HitboxExtent);
}
