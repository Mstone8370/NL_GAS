// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ControlShakeManager.h"

#include "GameFramework/Character.h"

UControlShakeManager::UControlShakeManager()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UControlShakeManager::BeginPlay()
{
    Super::BeginPlay();

    OwningCharacter = Cast<ACharacter>(GetOwner());

    ActiveShakes.Empty();
    ExpiredPool.Empty();
}

void UControlShakeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateShakes(DeltaTime);
}

void UControlShakeManager::UpdateShakes(float DeltaTime)
{
    static TArray<int32> ExpiredShakesIdx;
    ExpiredShakesIdx.Reset();

    FRotator DeltaSum = FRotator::ZeroRotator;

    for (int32 i = 0; i < ActiveShakes.Num(); i++)
    {
        TObjectPtr<UControlShake> Shake = ActiveShakes[i];
        if (!Shake)
        {
            ExpiredShakesIdx.Add(i);
            continue;
        }

        FRotator CurrentDelta;
        bool bIsStillActive = Shake->UpdateShake(DeltaTime, CurrentDelta);
        DeltaSum += CurrentDelta;

        if (!bIsStillActive)
        {
            ExpiredShakesIdx.Add(i);
        }
    }

    for (int32 i = ExpiredShakesIdx.Num() - 1; i >= 0; i--)
    {
        UControlShake* ExpiredShake = ActiveShakes[ExpiredShakesIdx[i]];
        if (ExpiredPool.Num() < 10)
        {
            ExpiredPool.Add(ExpiredShake);
        }
        ActiveShakes.RemoveAt(ExpiredShakesIdx[i]);
    }

    if (OwningCharacter)
    {
        OwningCharacter->AddControllerPitchInput(DeltaSum.Pitch);
        OwningCharacter->AddControllerYawInput(DeltaSum.Yaw);
        //OwningCharacter->AddControllerRollInput(DeltaSum.Roll);
    }

    UE_LOG(LogTemp, Warning, TEXT("ActiveShake Num: %d, ExpiredPool Num: %d"), ActiveShakes.Num(), ExpiredPool.Num());
}

void UControlShakeManager::AddShake(float InDuration, UCurveVector* InCurve, FRotator InShakeMagnitude)
{
    UControlShake* Shake = ReclaimShakeFromExpiredPool();
    if (!Shake)
    {
        Shake = NewObject<UControlShake>(this);
    }
    Shake->Activate(InDuration, InCurve, InShakeMagnitude);
    ActiveShakes.Add(Shake);
}

void UControlShakeManager::AddShake(FControlShakeParams Params)
{
    AddShake(Params.Duration, Params.Curve, Params.ShakeMagnitude);
}

UControlShake* UControlShakeManager::ReclaimShakeFromExpiredPool()
{
    if (ExpiredPool.Num() > 0)
    {
        return ExpiredPool.Pop();
    }
    return nullptr;
}

ACharacter* UControlShakeManager::GetOwningCharacter()
{
    if (!OwningCharacter)
    {
        OwningCharacter = Cast<ACharacter>(GetOwner());
    }
    return OwningCharacter;
}

