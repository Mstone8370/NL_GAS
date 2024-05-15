// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/AimPunchData.h"

const FTaggedAimPunch* UAimPunchData::GetAimPunchData(const FGameplayTag& DamageType) const
{
    if (DamageType.IsValid() && Data.Contains(DamageType))
    {
        return Data.Find(DamageType);
    }
    return nullptr;
}
