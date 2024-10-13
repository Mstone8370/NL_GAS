// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ControlShakeManager.h"

#include "GameFramework/Character.h"
#include "Data/WeaponRecoilPattern.h"

UControlShakeManager::UControlShakeManager()
    : ShakeSum(FRotator::ZeroRotator)
    , ShakeSumPrev(FRotator::ZeroRotator)
    , DeltaShake(FRotator::ZeroRotator)
    , MaxPoolSize(5)
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UControlShakeManager::BeginPlay()
{
    Super::BeginPlay();

    OwningCharacter = Cast<ACharacter>(GetOwner());

    ActiveShakes.Empty();
    ExpiredPoolMap.Empty();
}

void UControlShakeManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateShakes(DeltaTime);
    ApplyShake(DeltaTime);
}

void UControlShakeManager::UpdateShakes(float DeltaTime)
{
    ShakeSum = FRotator::ZeroRotator;

    if (LoopingShake)
    {
        FRotator ShakeValue;
        LoopingShake->UpdateShake(DeltaTime, ShakeValue);
        ShakeSum += ShakeValue;
    }

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

        FRotator ShakeValue;
        if (!Shake->UpdateShake(DeltaTime, ShakeValue))
        {
            ExpiredShakeIndices.Add(i);
        }
        ShakeSum += ShakeValue;
    }

    for (int32 i = ExpiredShakeIndices.Num() - 1; i >= 0; i--)
    {
        if (UControlShake* ExpiredShake = ActiveShakes[ExpiredShakeIndices[i]])
        {
            FGameplayTag ShakeTag = ExpiredShake->GetShakeTag();
            if (ShakeTag.IsValid() && 
                ExpiredPoolMap.Contains(ShakeTag) && 
                ExpiredPoolMap[ShakeTag].Num() < MaxPoolSize)
            {
                ExpiredPoolMap[ShakeTag].Add(ExpiredShake);
            }
        }
        
        ActiveShakes.RemoveAt(ExpiredShakeIndices[i]);
    }
}

void UControlShakeManager::ApplyShake(float DeltaTime)
{
    DeltaShake = ShakeSum - ShakeSumPrev;
    ShakeSumPrev = ShakeSum;

    if (ACharacter* Character = GetOwningCharacter())
    {
        Character->AddControllerPitchInput(DeltaShake.Pitch);
        Character->AddControllerYawInput(DeltaShake.Yaw);
    }
}

void UControlShakeManager::AddShake(float InDuration, UCurveVector* InCurve, FRotator InShakeMagnitude, bool bInLoop)
{
    UControlShake* Shake = ReclaimShakeFromExpiredPoolMap(FGameplayTag::EmptyTag);
    if (!Shake)
    {
        Shake = NewObject<UControlShake>(this);
    }

    if (bInLoop || InDuration <= 0)
    {
        if (LoopingShake && InCurve == LoopingShake->ControlShakeParams.Curve)
        {
            return;
        }

        ClearLoopingShake();
        LoopingShake = Shake;
        InDuration = -1.f;
    }
    else
    {
        ActiveShakes.Add(Shake);
    }
    Shake->Activate(InDuration, InCurve, InShakeMagnitude);
}

void UControlShakeManager::AddShake(const FGameplayTag& ShakeTag, FRotator InShakeMagnitude, bool bInLoop)
{
    UControlShake* Shake = ReclaimShakeFromExpiredPoolMap(ShakeTag);
    if (!Shake)
    {
        Shake = NewObject<UControlShake>(this);
        // TODO: 쉐이크 인스턴스 기본 설정
    }

    ActiveShakes.Add(Shake);
    Shake->Reactivate(InShakeMagnitude);
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
        WeaponTag,
        FRotator(RecoilPattern.X, RecoilPattern.Y, RecoilPattern.Z)
    );

    WeaponRecoilOffset++;

    float ResetTime = Info.RecoilOffsetResetTime;
    GetWorld()->GetTimerManager().SetTimer(
        WeaponRecoilOffsetResetTimer,
        [this, WeaponTag]()
        {
            ResetRecoilOffset(WeaponTag);
        },
        ResetTime,
        false
    );
}

void UControlShakeManager::ClearLoopingShake()
{
    if (LoopingShake)
    {
        LoopingShake->Clear();
        LoopingShake = nullptr;
    }
}

UControlShake* UControlShakeManager::ReclaimShakeFromExpiredPoolMap(const FGameplayTag& ShakeTag)
{
    if (ExpiredPoolMap.Contains(ShakeTag) && !ExpiredPoolMap[ShakeTag].PooledShakes.IsEmpty())
    {
        return ExpiredPoolMap[ShakeTag].Pop();
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

