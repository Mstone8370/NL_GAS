// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/NLProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NLFunctionLibrary.h"

ANLProjectile::ANLProjectile()
{
    PrimaryActorTick.bCanEverTick = true;

    SphereCollision = CreateDefaultSubobject<USphereComponent>(FName("SphereCollision"));
    SetRootComponent(SphereCollision);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("Mesh"));
    Mesh->SetupAttachment(GetRootComponent());

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovement"));
}

void ANLProjectile::BeginPlay()
{
    Super::BeginPlay();
    
    SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &ANLProjectile::OnBeginOverlap);
}

void ANLProjectile::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (HasAuthority())
    {
        if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
        {
            DamageEffectParams.TargetASC = TargetASC;

            // TODO: Fill DamageEffectParams.

            UNLFunctionLibrary::ApplyDamageEffect(DamageEffectParams);
        }
        else if (false) // TODO: Check if OtherActor is damageable prop Actor.
        {

        }
    }
}
