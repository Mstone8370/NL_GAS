// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectiles/NLProjectile_Bullet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NLFunctionLibrary.h"

void ANLProjectile_Bullet::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (HasAuthority())
    {
        if (GetOwner() == OtherActor)
        {
            return;
        }

        if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
        {
            DamageEffectParams.TargetASC = TargetASC;
            DamageEffectParams.TravelDistance = FVector::Dist(SweepResult.Location, StartLocation);
            DamageEffectParams.RadialDamageOrigin = GetActorLocation();

            UNLFunctionLibrary::ApplyDamageEffect(DamageEffectParams);
        }
        else if (false) // TODO: Check if OtherActor is damageable prop Actor.
        {

        }

        Destroy();
    }
}

void ANLProjectile_Bullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (HasAuthority())
    {
        Destroy();
    }
}
