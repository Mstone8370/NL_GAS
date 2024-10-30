// Fill out your copyright notice in the Description page of Project Settings.


#include "Objects/ControlShake.h"

#include "Curves/CurveVector.h"
#include "Kismet/KismetMathLibrary.h"

UControlShake::UControlShake()
    : ShakeTag(FGameplayTag::EmptyTag)
    , ShakeCurve(nullptr)
    , Duration(1.f)
    , ShakeMagnitude(FRotator::ZeroRotator)
    , bIsActive(true)
    , TimeElapsed(0.f)
{}

void UControlShake::Initialize(const FGameplayTag& InShakeTag, UCurveVector* InShakeCurve, float InDuration, FRotator InMagnitude)
{
    ShakeTag = InShakeTag;
    ShakeCurve = InShakeCurve;
    Duration = InDuration;
    ShakeMagnitude = InMagnitude;
}

bool UControlShake::UpdateShake(float DeltaTime, FRotator& OutShake)
{
    OutShake = FRotator::ZeroRotator;

    if (!bIsActive || !ShakeCurve)
    {
        return false;
    }

    TimeElapsed += DeltaTime;
    
    float CurveTime = TimeElapsed;
    if (Duration <= 0.f)
    {
        // Looping Shake
        float CurveStart;
        float CurveEnd;
        ShakeCurve->GetTimeRange(CurveStart, CurveEnd);

        const float CurveLength = CurveEnd - CurveStart;
        TimeElapsed = CurveStart + FMath::Fmod(TimeElapsed, CurveLength);  // TimeElapsed 값을 루프되게 함.
        CurveTime = TimeElapsed;
    }
    else
    {
        CurveTime = UKismetMathLibrary::SafeDivide(TimeElapsed, Duration);
        bIsActive = (CurveTime < 1.f);
    }

    const FVector CurveValue = bIsActive ? ShakeCurve->GetVectorValue(CurveTime) : FVector::ZeroVector;
    
    OutShake = FRotator(
        ShakeMagnitude.Pitch * CurveValue.X,
        ShakeMagnitude.Yaw * CurveValue.Y,
        ShakeMagnitude.Roll * CurveValue.Z  // Roll only affects weapon mesh.
    );

    if (!bIsActive)
    {
        Deactivate();
    }

    return bIsActive;
}

void UControlShake::Activate(float InDuration, UCurveVector* InCurve, FRotator InShakeMagnitude)
{
    bIsActive = true;
    TimeElapsed = 0.f;

    Duration = InDuration;
    ShakeCurve = InCurve;
    
    ShakeMagnitude = InShakeMagnitude;
}

void UControlShake::Reactivate(FRotator InShakeMagnitude)
{
    bIsActive = true;
    TimeElapsed = 0.f;

    ShakeMagnitude = InShakeMagnitude;
}

void UControlShake::Deactivate()
{
    bIsActive = false;
    TimeElapsed = 0.f;
}
