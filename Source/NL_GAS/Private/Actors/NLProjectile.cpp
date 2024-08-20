// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/NLProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ANLProjectile::ANLProjectile()
    : StartLocation(FVector::ZeroVector)
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = false;

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
    SphereCollision->OnComponentHit.AddDynamic(this, &ANLProjectile::OnHit);
    OnDestroyed.AddDynamic(this, &ANLProjectile::HandleDestroy);
}

void ANLProjectile::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    OnProjectileHit(SweepResult);
}

void ANLProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    OnProjectileHit(Hit);
}
