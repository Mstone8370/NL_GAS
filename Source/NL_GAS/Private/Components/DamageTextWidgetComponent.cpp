// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/DamageTextWidgetComponent.h"

void UDamageTextWidgetComponent::BeginPlay()
{
    Super::BeginPlay();

    Initialize();
}

void UDamageTextWidgetComponent::UpdateValue_Implementation(float InValue, bool bIsCriticalHit)
{
    Value += InValue;
    Initialize();
}

void UDamageTextWidgetComponent::Initialize()
{
    float LocalUpdateWaitTime = UpdateWaitTime > 0.f ? UpdateWaitTime : 1.f;
    GetWorld()->GetTimerManager().SetTimer(
        UpdateWaitTimerHandle,
        this,
        &UDamageTextWidgetComponent::OnStopWatingUpdate,
        LocalUpdateWaitTime,
        false
    );
}

void UDamageTextWidgetComponent::OnStopWatingUpdate_Implementation()
{
    if (LifeTime > 0.f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            LifeTimerHandle,
            this,
            &UDamageTextWidgetComponent::OnLifeTimeExpired,
            LifeTime,
            false
        );
    }
    else
    {
        OnLifeTimeExpired();
    }
}

void UDamageTextWidgetComponent::OnLifeTimeExpired()
{
    DestroyComponent();
}

bool UDamageTextWidgetComponent::IsWatingUpdate() const
{
    return GetWorld()->GetTimerManager().IsTimerActive(UpdateWaitTimerHandle);
}
