// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/ControlShake.h"

#include "Curves/CurveVector.h"
#include "Kismet/KismetMathLibrary.h"

UControlShake::UControlShake()
    : Duration(1.f)
    , Curve(nullptr)
    , ShakeMagnitude(1.f, 1.f, 1.f)
    , bIsActive(true)
    , TimeElapsed(0.f)
{}

bool UControlShake::UpdateShake(float DeltaTime, FRotator& OutShake)
{
    OutShake = FRotator::ZeroRotator;

    if (!bIsActive || !Curve)
    {
        return false;
    }

    TimeElapsed += DeltaTime;
    
    const float CurveTime = UKismetMathLibrary::SafeDivide(TimeElapsed, Duration);
    bIsActive = (CurveTime < 1.f);

    const FVector CurveValue = bIsActive ? Curve->GetVectorValue(CurveTime) : FVector::ZeroVector;
    
    OutShake = FRotator(
        ShakeMagnitude.Pitch * CurveValue.X,
        ShakeMagnitude.Yaw * CurveValue.Y,
        ShakeMagnitude.Roll * CurveValue.Z  // Roll only affects weapon mesh.
    );

    return bIsActive;
}

void UControlShake::Activate(float InDuration, UCurveVector* InCurve, FRotator InShakeMagnitude)
{
    bIsActive = true;
    TimeElapsed = 0.f;

    Duration = InDuration;
    Curve = InCurve;
    ShakeMagnitude = InShakeMagnitude;
}

void UControlShake::Activate(FControlShakeParams Params)
{
    Activate(Params.Duration, Params.Curve, Params.ShakeMagnitude);
}
