// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/NLPlayerCameraComponent.h"

UNLPlayerCameraComponent::UNLPlayerCameraComponent()
    : InterpSpeed(10.f)
    , BaseFOV(90.f)
    , TargetFOV(90.f)
    , CurrentFOV(90.f)
    , bDoInterp(false)
{
}

void UNLPlayerCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    InterpFOV(DeltaTime);
}

void UNLPlayerCameraComponent::InterpFOV(float DeltaTime)
{
    if (!bDoInterp)
    {
        return;
    }

    CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, InterpSpeed);
    if (FMath::IsNearlyEqual(CurrentFOV, TargetFOV))
    {
        CurrentFOV = TargetFOV;
        bDoInterp = false;
    }
    SetFieldOfView(CurrentFOV);
}

void UNLPlayerCameraComponent::SetBaseFOV(float InBaseFOV)
{
    const float PrevBaseFOV = BaseFOV;
    BaseFOV = InBaseFOV;

    if (FMath::IsNearlyEqual(PrevBaseFOV, TargetFOV))
    {
        TargetFOV = BaseFOV;
    }
    else
    {
        TargetFOV *= BaseFOV / PrevBaseFOV;
    }

    if (!bDoInterp)
    {
        CurrentFOV = TargetFOV;
        SetFieldOfView(CurrentFOV);
    }
}

void UNLPlayerCameraComponent::SetTargetFOV(float InTargetFOV)
{
    if (!FMath::IsNearlyEqual(TargetFOV, InTargetFOV))
    {
        TargetFOV = InTargetFOV;
        bDoInterp = true;
    }
}
