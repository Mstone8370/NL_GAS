// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ControlShakeManager.h"

#include "GameFramework/Character.h"
#include "Data/WeaponRecoilPattern.h"

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
    TotalShake = FRotator::ZeroRotator;

    if (ActiveShakes.Num() < 1)
    {
        return;
    }

    static TArray<int32> ExpiredShakeIndices;
    ExpiredShakeIndices.Reset();

    for (int32 i = 0; i < ActiveShakes.Num(); i++)
    {
        TObjectPtr<UControlShake> Shake = ActiveShakes[i];
        if (!Shake)
        {
            ExpiredShakeIndices.Add(i);
            continue;
        }

        FRotator CurrentDelta;
        if (!Shake->UpdateShake(DeltaTime, CurrentDelta))
        {
            ExpiredShakeIndices.Add(i);
        }
        TotalShake += CurrentDelta;
    }

    for (int32 i = ExpiredShakeIndices.Num() - 1; i >= 0; i--)
    {
        UControlShake* ExpiredShake = ActiveShakes[ExpiredShakeIndices[i]];
        if (ExpiredPool.Num() < 6)
        {
            ExpiredPool.Add(ExpiredShake);
        }
        ActiveShakes.RemoveAt(ExpiredShakeIndices[i]);
    }

    if (OwningCharacter)
    {
        OwningCharacter->AddControllerPitchInput(TotalShake.Pitch);
        OwningCharacter->AddControllerYawInput(TotalShake.Yaw);
    }
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

void UControlShakeManager::WeaponFired(const FGameplayTag& WeaponTag)
{
    if (!RecoilPatternData || !RecoilPatternData->HasRecoilPattern(WeaponTag))
    {
        return;
    }
    
    int32& WeaponRecoilOffset = RecoilOffsetsMap.FindOrAdd(WeaponTag, 0);
    FTimerHandle& WeaponRecoilOffsetResetTimer = RecoilOffsetResetTimersMap.FindOrAdd(WeaponTag);

    const FVector RecoilPattern = RecoilPatternData->GetRecoilPatternAt(WeaponTag, WeaponRecoilOffset);
    const FWeaponRecoilInfo& Info = RecoilPatternData->Data[WeaponTag];

    AddShake(
        Info.SingleRecoilDuration,
        Info.SingleRecoilCurve,
        FRotator(RecoilPattern.X, RecoilPattern.Y, RecoilPattern.Z)
    );

    WeaponRecoilOffset++;
    GetWorld()->GetTimerManager().SetTimer(
        WeaponRecoilOffsetResetTimer,
        [this, WeaponTag]()
        {
            ResetRecoilOffset(WeaponTag);
        },
        0.15f,  // TODO: make it variable
        false
    );
}

UControlShake* UControlShakeManager::ReclaimShakeFromExpiredPool()
{
    if (!ExpiredPool.IsEmpty())
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

void UControlShakeManager::ResetRecoilOffset(const FGameplayTag& WeaponTag)
{
    if (WeaponTag.IsValid() && RecoilOffsetsMap.Contains(WeaponTag))
    {
        RecoilOffsetsMap[WeaponTag] = 0;
    }
}

int UControlShakeManager::GetRecoilOffset(const FGameplayTag& WeaponTag) const
{
    if (WeaponTag.IsValid() && RecoilOffsetsMap.Contains(WeaponTag))
    {
        return RecoilOffsetsMap[WeaponTag];
    }
    return 0;
}

