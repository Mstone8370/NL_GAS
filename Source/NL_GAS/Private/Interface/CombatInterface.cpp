// Fill out your copyright notice in the Description page of Project Settings.


#include "Interface/CombatInterface.h"

#include "GameplayTagContainer.h"

// Add default functionality here for any ICombatInterface functions that are not pure virtual.

bool FDeathInfo::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
    uint8 Flags = 0;

    if (Ar.IsSaving())
    {
        if (bIsDead)
        {
            Flags |= 1 << 0;
        }
    }

    Ar.SerializeBits(&Flags, 2);

    if (Flags & (1 << 0))
    {
        bIsDead = true;

        Ar << SourceActor;

        if (Ar.IsLoading())
        {
            if (!DamageType.IsValid())
            {
                DamageType = TSharedPtr<FGameplayTag>(new FGameplayTag());
            }
        }
        DamageType->NetSerialize(Ar, Map, bOutSuccess);
    }

    bOutSuccess = true;
    return true;
}
