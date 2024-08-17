// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectiles/NLProjectile_Bullet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NLFunctionLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DecalComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Interface/Damageable.h"

void ANLProjectile_Bullet::BeginPlay()
{
    Super::BeginPlay();
}

void ANLProjectile_Bullet::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (GetOwner() == OtherActor)
    {
        return;
    }

    if (HasAuthority())
    {
        DamageEffectParams.TravelDistance = FVector::Dist(SweepResult.Location, StartLocation);
        DamageEffectParams.RadialDamageOrigin = GetActorLocation();
        DamageEffectParams.HitResult = SweepResult;

        if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
        {
            DamageEffectParams.TargetASC = TargetASC;

            UNLFunctionLibrary::ApplyDamageEffect(DamageEffectParams);
        }
        else if (OtherActor->Implements<UDamageable>())
        {
            float Damage = DamageEffectParams.DamageScalableFloat.GetValueAtLevel(SweepResult.Distance);
            IDamageable::Execute_OnTakenDamage(OtherActor, Damage, DamageEffectParams);
        }

        Destroy();
    }

    HandleHitFX(SweepResult);
}

void ANLProjectile_Bullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (GetOwner() == OtherActor)
    {
        return;
    }

    if (HasAuthority())
    {
        DamageEffectParams.TravelDistance = FVector::Dist(Hit.Location, StartLocation);
        DamageEffectParams.RadialDamageOrigin = GetActorLocation();
        DamageEffectParams.HitResult = Hit;

        if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
        {
            DamageEffectParams.TargetASC = TargetASC;

            UNLFunctionLibrary::ApplyDamageEffect(DamageEffectParams);
        }
        else if (OtherActor->Implements<UDamageable>())
        {
            float Damage = DamageEffectParams.DamageScalableFloat.GetValueAtLevel(Hit.Distance);
            IDamageable::Execute_OnTakenDamage(OtherActor, Damage, DamageEffectParams);
        }

        Destroy();
    }

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

void ANLProjectile_Bullet::HandleHitFX(const FHitResult& HitResult)
{
    bHit = true;

    if (HitImpactDecalMaterial)
    {
        UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(
            GetWorld(),
            HitImpactDecalMaterial,
            DecalSize,
            HitResult.ImpactPoint,
            HitResult.ImpactNormal.Rotation(),
            DecalLifeSpan
        );
        if (Decal)
        {
            Decal->SetFadeScreenSize(0.0001f);
        }
    }

    if (HitImpactFX)
    {
        FFXSystemSpawnParameters SpawnParams;
        SpawnParams.WorldContextObject = GetWorld();
        SpawnParams.SystemTemplate = HitImpactFX;
        SpawnParams.Location = HitResult.ImpactPoint;
        SpawnParams.Rotation = HitResult.ImpactNormal.Rotation();
        if (UNiagaraComponent* Niagara = UNiagaraFunctionLibrary::SpawnSystemAtLocationWithParams(SpawnParams))
        {
            Niagara->SetVectorParameter(FName("Direction"), HitResult.ImpactNormal);
        }
    }
}
