// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/WeaponInfo.h"

const FTaggedWeaponInfo* UWeaponInfo::FindWeaponInfoByTag(const FGameplayTag& InWeaponTag) const
{
    for (const FTaggedWeaponInfo& Info : WeaponInfos)
    {
        if (Info.WeaponTag.MatchesTagExact(InWeaponTag))
        {
            return &Info;
        }
    }
    return nullptr;
}
