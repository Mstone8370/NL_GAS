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
        if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
        {
            DamageEffectParams.TargetASC = TargetASC;
            DamageEffectParams.TravelDistance = FVector::Dist(SweepResult.Location, StartLocation);
            DamageEffectParams.RadialDamageOrigin = GetActorLocation();
            DamageEffectParams.HitResult = SweepResult;

            UNLFunctionLibrary::ApplyDamageEffect(DamageEffectParams);
        }
        else if (false) // TODO: Check if OtherActor is damageable prop Actor.
        {

        }

        Destroy();

        if (GetNetMode() == ENetMode::NM_Standalone || GetNetMode() == ENetMode::NM_ListenServer)
        {
            HandleHitFX(SweepResult);
        }
    }
    else
    {
        HandleHitFX(SweepResult);
    }
}

void ANLProjectile_Bullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (GetOwner() == OtherActor)
    {
        return;
    }

    if (HasAuthority())
    {
        Destroy();

        if (GetNetMode() == ENetMode::NM_Standalone || GetNetMode() == ENetMode::NM_ListenServer)
        {
            HandleHitFX(Hit);
        }
    }
    else
    {
        HandleHitFX(Hit);
    }
}

void ANLProjectile_Bullet::HandleHitFX(const FHitResult& HitResult)
{
    bHit = true;

    if (HitImpactDecalMaterial)
    {
        UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(
            this,
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
        SpawnParams.WorldContextObject = this;
        SpawnParams.SystemTemplate = HitImpactFX;
        SpawnParams.Location = HitResult.ImpactPoint;
        SpawnParams.Rotation = HitResult.ImpactNormal.Rotation();
        if (UNiagaraComponent* Niagara = UNiagaraFunctionLibrary::SpawnSystemAtLocationWithParams(SpawnParams))
        {
            Niagara->SetVectorParameter(FName("Direction"), HitResult.ImpactNormal);
        }
    }
}

void ANLProjectile_Bullet::BeginDestroy()
{
    Super::BeginDestroy();
}

