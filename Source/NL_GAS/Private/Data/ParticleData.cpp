// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/ParticleData.h"

const FParticleInfo* UParticleData::FindParticleDataByTag(const FGameplayTag& Tag)
{
    if (HasParticleData(Tag))
    {
        return Data.Find(Tag);
    }
    return nullptr;
}

bool UParticleData::HasParticleData(const FGameplayTag& Tag) const
{
    return Data.Contains(Tag);
}
