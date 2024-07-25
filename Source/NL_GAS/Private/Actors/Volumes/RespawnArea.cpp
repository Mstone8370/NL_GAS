// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Volumes/RespawnArea.h"

#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

ARespawnArea::ARespawnArea()
{
    BoxComponent = CreateDefaultSubobject<UBoxComponent>(FName("BoxComponent"));
    BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetRootComponent(BoxComponent);
}

bool ARespawnArea::GetRespawnableLocation(float CapsuleHalfHeight, float CapsuleRadius, FVector& OutLocation)
{
    const FVector BoxExtend = BoxComponent->GetScaledBoxExtent();
    const FVector Origin = GetActorLocation();

    // TODO: 루프로 캡슐이 들어갈 수 있는 위치 찾을때까지 or 최대 횟수만큼 위치 찾기 시도
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
            EDrawDebugTrace::ForDuration,
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

                UKismetSystemLibrary::DrawDebugCapsule(
                    this,
                    OutLocation,
                    CapsuleHalfHeight,
                    CapsuleRadius,
                    FRotator::ZeroRotator,
                    FLinearColor::White,
                    999.f
                );

                return true;
            }
        }

        OutLocation = End;
        OutLocation.Z += CapsuleHalfHeight - CapsuleRadius + UE_SMALL_NUMBER;

        UKismetSystemLibrary::DrawDebugCapsule(
            this,
            OutLocation,
            CapsuleHalfHeight,
            CapsuleRadius,
            FRotator::ZeroRotator,
            FLinearColor::White,
            999.f
        );

        return true;
    }
    return false;
}
