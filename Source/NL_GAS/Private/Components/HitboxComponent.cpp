// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/HitboxComponent.h"

UHitboxComponent::UHitboxComponent()
    : bIsWeakHitbox(false)
    , CriticalHitDamageMultiplier(2.f)
{
    SetCollisionProfileName(FName("Hitbox"));
    bHiddenInGame = 0;
}
