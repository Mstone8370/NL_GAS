// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectiles/NLProjectile_Bullet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NLFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Interface/Damageable.h"
#include "Data/ParticleData.h"

void ANLProjectile_Bullet::BeginPlay()
{
    Super::BeginPlay();
}

void ANLProjectile_Bullet::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ApplyDamage(SweepResult);

    HandleHitFX(SweepResult);
}

void ANLProjectile_Bullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    ApplyDamage(Hit);

    HandleHitFX(Hit);
}

void ANLProjectile_Bullet::HandleDestroy(AActor* DestroyedActor)
{
    Super::HandleDestroy(DestroyedActor);

    if (!bHit)
    {
        FHitResult TempHitResult;
        TempHitResult.ImpactPoint = GetActorLocation();
        TempHitResult.ImpactNormal = -GetActorForwardVector();
        HandleHitFX(TempHitResult);
    }
}

void ANLProjectile_Bullet::ApplyDamage(const FHitResult& HitResult)
{
    if (!HasAuthority())
    {
        return;
    }

    AActor* OtherActor = HitResult.GetActor();
    if (!IsValid(OtherActor) || GetOwner() == OtherActor)
    {
        return;
    }

    if (HasAuthority())
    {
        DamageEffectParams.TravelDistance = FVector::Dist(HitResult.Location, StartLocation);
        DamageEffectParams.RadialDamageOrigin = GetActorLocation();
        DamageEffectParams.HitResult = HitResult;

        if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
        {
            DamageEffectParams.TargetASC = TargetASC;

            UNLFunctionLibrary::ApplyDamageEffect(DamageEffectParams);
        }
        else if (OtherActor->Implements<UDamageable>())
        {
            float Damage = DamageEffectParams.DamageScalableFloat.GetValueAtLevel(HitResult.Distance);
            IDamageable::Execute_OnTakenDamage(OtherActor, Damage, DamageEffectParams);
        }
    }
}

void ANLProjectile_Bullet::HandleHitFX(const FHitResult& HitResult)
{
    bHit = true;

    FParticleSpawnInfo SpawnInfo;
    SpawnInfo.Location = HitResult.ImpactPoint;
    SpawnInfo.Normal = HitResult.ImpactNormal;
    UNLFunctionLibrary::SpawnSingleParticleByTag(GetWorld(), HitParticleTag, SpawnInfo);
}
