// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/WeaponInfo.h"

const FWeaponInfo* UTaggedWeaponInfoList::FindTaggedWeaponInfoByTag(const FGameplayTag& InWeaponTag) const
{
    if (InWeaponTag.IsValid())
    {
        for (const UTaggedWeaponInfo* Info : TaggedWeaponInfos)
        {
            if (Info->WeaponTag.MatchesTagExact(InWeaponTag))
            {
                return &Info->WeaponInfo;
            }
        }
    }
    return nullptr;
}

const FUIWeaponInfo* UUITaggedWeaponInfo::FindUIWeaponInfoByTag(const FGameplayTag& InWeaponTag) const
{
    if (InWeaponTag.IsValid())
    {
        if (Data.Contains(InWeaponTag))
        {
            return Data.Find(InWeaponTag);
        }
    }
    return nullptr;
}
