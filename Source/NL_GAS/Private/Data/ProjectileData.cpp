// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/ProjectileData.h"

const FProjectileInfo* UProjectileData::FindProjectileDataByTag(const FGameplayTag& Tag)
{
    if (HasProjectileData(Tag))
    {
        return Data.Find(Tag);
    }
    return nullptr;
}

bool UProjectileData::HasProjectileData(const FGameplayTag& Tag) const
{
    return Data.Contains(Tag);
}
