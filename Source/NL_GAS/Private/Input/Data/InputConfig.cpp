// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/Data/InputConfig.h"

const UInputAction* UInputConfig::FindInputActionByTag(const FGameplayTag& InputTag) const
{
    for (const FTaggedInputAction& TIA : TaggedInputActions)
    {
        if (TIA.InputTag.MatchesTagExact(InputTag))
        {
            return TIA.InputAction;
        }
    }
    return nullptr;
}
