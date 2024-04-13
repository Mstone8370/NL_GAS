// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/ControlShake.h"

#include "Curves/CurveVector.h"
#include "Kismet/KismetMathLibrary.h"

UControlShake::UControlShake()
    : Duration(1.f)
    , Curve(nullptr)
    , ShakeMagnitude(1.f, 1.f, 0.f)
    , bIsActive(true)
    , TimeElapsed(0.f)
    , CurveValue_Prev(FVector::ZeroVector)
{}

bool UControlShake::UpdateShake(float DeltaTime, FRotator& OutDeltaRotation)
{
    OutDeltaRotation = FRotator::ZeroRotator;

    if (!bIsActive || !Curve)
    {
        return false;
    }

    TimeElapsed += DeltaTime;
    
    const float CurveTime = UKismetMathLibrary::SafeDivide(TimeElapsed, Duration);
    bIsActive = (CurveTime < 1.f);

    const FVector CurveValue_Current = bIsActive ? Curve->GetVectorValue(CurveTime) : FVector::ZeroVector;
    const FVector CurveValue_Delta = CurveValue_Current - CurveValue_Prev;
    CurveValue_Prev = CurveValue_Current;

    OutDeltaRotation = FRotator(
        ShakeMagnitude.Pitch * CurveValue_Delta.X,
        ShakeMagnitude.Yaw * CurveValue_Delta.Y,
        0.f  // Roll is ignored.
    );

    return bIsActive;
}

void UControlShake::Activate(float InDuration, UCurveVector* InCurve, FRotator InShakeMagnitude)
{
    bIsActive = true;
    TimeElapsed = 0.f;
    CurveValue_Prev = FVector::ZeroVector;

    Duration = InDuration;
    Curve = InCurve;
    ShakeMagnitude = InShakeMagnitude;
}

void UControlShake::Activate(FControlShakeParams Params)
{
    Activate(Params.Duration, Params.Curve, Params.ShakeMagnitude);
}
