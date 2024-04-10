// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/WeaponRecoilPattern.h"

#include "Curves/CurveVector.h"

bool UWeaponRecoilPattern::HasRecoilPattern(const FGameplayTag& WeaponTag) const
{
    return WeaponTag.IsValid() && Data.Contains(WeaponTag) && Data[WeaponTag].RecoilCurve != nullptr;
}

FVector UWeaponRecoilPattern::GetRecoilPatternAt(const FGameplayTag& WeaponTag, int32 Offset) const
{
    if (!HasRecoilPattern(WeaponTag))
    {
        return FVector::ZeroVector;
    }

    const FWeaponRecoilInfo& RecoilInfo = Data[WeaponTag];

    float MinTime = 0.f;
    float MaxTime = 0.f;
    RecoilInfo.RecoilCurve->GetTimeRange(MinTime, MaxTime);

    const int32 OffsetBegin = FMath::RoundHalfToZero(MinTime);
    const int32 OffsetEnd = FMath::RoundHalfToZero(MaxTime);
    const int32 PatternLength = OffsetEnd - OffsetBegin;

    const int32 LoopStartOffset = FMath::Max(0, RecoilInfo.LoopStartOffset);
    const int32 LoopEndOffset = FMath::Max(0, RecoilInfo.LoopEndOffset);
    const int32 LoopLength = FMath::Max(0, LoopEndOffset - LoopStartOffset);

    // Before loop
    if (Offset < PatternLength)
    {
        const int32 CurveTime = OffsetBegin + Offset;
        return RecoilInfo.RecoilCurve->GetVectorValue(CurveTime);
    }

    // In loop
    if (LoopLength < 1)
    {
        return RecoilInfo.RecoilCurve->GetVectorValue(OffsetBegin + LoopStartOffset);
    }
    const int32 ExceededOffset = Offset - OffsetEnd;
    const int32 CurveTime = OffsetBegin + LoopStartOffset + (ExceededOffset % LoopLength);
    return RecoilInfo.RecoilCurve->GetVectorValue(CurveTime);
}
