// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/ControlShake.h"

#include "Curves/CurveVector.h"
#include "Kismet/KismetMathLibrary.h"

UControlShake::UControlShake()
    : bIsActive(true)
    , TimeElapsed(0.f)
{}

bool UControlShake::UpdateShake(float DeltaTime, FRotator& OutShake)
{
    OutShake = FRotator::ZeroRotator;

    if (!bIsActive || !ControlShakeParams.Curve)
    {
        return false;
    }

    TimeElapsed += DeltaTime;
    
    float CurveTime = TimeElapsed;
    if (ControlShakeParams.bLoop)
    {
        float CurveStart;
        float CurveEnd;
        ControlShakeParams.Curve->GetTimeRange(CurveStart, CurveEnd);
        const float CurveLength = CurveEnd - CurveStart;
        TimeElapsed = CurveStart + FMath::Fmod(TimeElapsed, CurveLength);  // TimeElapsed ���� �����ǰ� ��.
        CurveTime = TimeElapsed;
    }
    else
    {
        CurveTime = UKismetMathLibrary::SafeDivide(TimeElapsed, ControlShakeParams.Duration);
        bIsActive = (CurveTime < 1.f);
    }

    const FVector CurveValue = bIsActive ? ControlShakeParams.Curve->GetVectorValue(CurveTime) : FVector::ZeroVector;
    
    OutShake = FRotator(
        ControlShakeParams.ShakeMagnitude.Pitch * CurveValue.X,
        ControlShakeParams.ShakeMagnitude.Yaw * CurveValue.Y,
        ControlShakeParams.ShakeMagnitude.Roll * CurveValue.Z  // Roll only affects weapon mesh.
    );

    if (!bIsActive)
    {
        Clear();
    }

    return bIsActive;
}

void UControlShake::Activate(float InDuration, UCurveVector* InCurve, FRotator InShakeMagnitude, bool bInLoop)
{
    bIsActive = true;
    TimeElapsed = 0.f;

    ControlShakeParams.Duration = InDuration;
    ControlShakeParams.Curve = InCurve;
    ControlShakeParams.ShakeMagnitude = InShakeMagnitude;
    ControlShakeParams.bLoop = bInLoop;
}

void UControlShake::Activate(FControlShakeParams InParams)
{
    Activate(InParams.Duration, InParams.Curve, InParams.ShakeMagnitude, InParams.bLoop);
}

void UControlShake::Clear()
{
    bIsActive = false;
    TimeElapsed = 0.f;
    ControlShakeParams.Clear();
}
