// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Volumes/RespawnArea.h"

#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

ARespawnArea::ARespawnArea()
{
    BoxComponent = CreateDefaultSubobject<UBoxComponent>(FName("BoxComponent"));
    BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetRootComponent(BoxComponent);

    ArrowComponent = CreateDefaultSubobject<UArrowComponent>(FName("Respawn Direction"));
    ArrowComponent->SetupAttachment(BoxComponent);
}

bool ARespawnArea::GetRespawnableLocation(float CapsuleHalfHeight, float CapsuleRadius, FVector& OutLocation)
{
    const FVector BoxExtend = BoxComponent->GetScaledBoxExtent();
    const FVector Origin = GetActorLocation();

    for (int32 i = 0; i < 999; i++)
    {
        const FVector RandomPoint = UKismetMathLibrary::RandomPointInBoundingBox(Origin, BoxExtend);

        FHitResult HitRes;
        FCollisionQueryParams Params;
        FVector Start = RandomPoint;
        Start.Z = Origin.Z + BoxExtend.Z - CapsuleRadius - UE_KINDA_SMALL_NUMBER;
        FVector End = Start;
        End.Z = Origin.Z - BoxExtend.Z + CapsuleRadius;
        TArray<AActor*> ActorsToIgnore;
        UKismetSystemLibrary::SphereTraceSingleByProfile(
            this,
            Start,
            End,
            CapsuleRadius,
            FName("Pawn"),
            false,
            ActorsToIgnore,
            EDrawDebugTrace::None,
            HitRes,
            true,
            FLinearColor::Red,
            FLinearColor::Green,
            999.f
        );

        if (HitRes.bBlockingHit)
        {
            if (Start.Z - HitRes.Location.Z > (CapsuleHalfHeight - CapsuleRadius) * 2 + UE_KINDA_SMALL_NUMBER)
            {
                OutLocation = HitRes.Location;
                OutLocation.Z += CapsuleHalfHeight - CapsuleRadius + UE_SMALL_NUMBER;

                return true;
            }
            else
            {
                continue;
            }
        }

        OutLocation = End;
        OutLocation.Z += CapsuleHalfHeight - CapsuleRadius + UE_SMALL_NUMBER;

        return true;
    }
    return false;
}

FVector ARespawnArea::GetDirection() const
{
    FVector Ret = ArrowComponent->GetForwardVector();
    Ret.Z = 0.f;
    return Ret;
}
