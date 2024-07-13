// Fill out your copyright notice in the Description page of Project Settings.


#include "Interface/CombatInterface.h"

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
    }

    bOutSuccess = true;
    return true;
}
