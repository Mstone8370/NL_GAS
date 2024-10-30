// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/ControlShakeData.h"

const FTaggedControlShake* UControlShakeData::GetControlShakeData(const FGameplayTag& ShakeTag) const
{
    if (ShakeTag.IsValid() && Data.Contains(ShakeTag))
    {
        return Data.Find(ShakeTag);
    }
    return nullptr;
}
