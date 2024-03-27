// Fill out your copyright notice in the Description page of Project Settings.


#include "NLFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameModes/NLGameMode.h"
#include "Data/WeaponInfo.h"

const FTaggedWeaponInfo* UNLFunctionLibrary::GetWeaponInfoByTag(const UObject* WorldContextObject, const FGameplayTag& WeaponTag)
{
    if (WeaponTag.IsValid())
    {
        if (ANLGameMode* NLGameMode = Cast<ANLGameMode>(UGameplayStatics::GetGameMode(WorldContextObject)))
        {
            if (UWeaponInfo* WeaponInfoDataAsset = NLGameMode->WeaponInfo)
            {
                return WeaponInfoDataAsset->FindWeaponInfoByTag(WeaponTag);
            }
        }
    }
    return nullptr;
}
