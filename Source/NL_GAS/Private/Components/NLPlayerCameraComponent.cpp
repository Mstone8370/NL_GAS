// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/NLPlayerCameraComponent.h"

UNLPlayerCameraComponent::UNLPlayerCameraComponent()
    : HealthPPMatThresholdMax(0.5f)
    , HealthPPMatThresholdMin(0.2f)
    , InterpSpeed(10.f)
    , BaseFOV(90.f)
    , TargetFOV(90.f)
    , CurrentFOV(90.f)
    , bDoInterp(false)
{
}

void UNLPlayerCameraComponent::BeginPlay()
{
    Super::BeginPlay();

    if (HealthPPMaterial)
    {
        if (HealthPPMatInst = UMaterialInstanceDynamic::Create(HealthPPMaterial, this))
        {
            PostProcessSettings.AddBlendable(HealthPPMatInst, 1.f);
            HealthPPMatInst->SetScalarParameterValue(FName("Percent"), 1.f);
        }
    }
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

    CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, CurrentInterpSpeed);
    if (FMath::IsNearlyEqual(CurrentFOV, TargetFOV))
    {
        CurrentFOV = TargetFOV;
        bDoInterp = false;
        CurrentInterpSpeed = InterpSpeed;
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

void UNLPlayerCameraComponent::SetTargetFOV(float InTargetFOV, float TransientInterpSpeed)
{
    if (!FMath::IsNearlyEqual(TargetFOV, InTargetFOV))
    {
        TargetFOV = InTargetFOV;
        bDoInterp = true;
        CurrentInterpSpeed = TransientInterpSpeed > 0.f ? TransientInterpSpeed : InterpSpeed;
    }
}

void UNLPlayerCameraComponent::OnPlayerHealthChanged(float HealthPercent)
{
    if (HealthPPMatInst)
    {
        float Ratio = 1.f;
        if (HealthPPMatThresholdMax > HealthPPMatThresholdMin)
        {
            Ratio = (HealthPercent - HealthPPMatThresholdMin) / (HealthPPMatThresholdMax - HealthPPMatThresholdMin);
        }
        const float Value = FMath::Clamp(Ratio, 0.f, 1.f);
        HealthPPMatInst->SetScalarParameterValue(FName("Percent"), Value);
    }
}
