// Fill out your copyright notice in the Description page of Project Settings.


#include "NLFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameModes/NLGameMode.h"
#include "NLGameInstance.h"
#include "Data/WeaponInfo.h"

const FWeaponInfo* UNLFunctionLibrary::GetWeaponInfoByTag(const UObject* WorldContextObject, const FGameplayTag& WeaponTag)
{
    if (WeaponTag.IsValid())
    {
        if (UNLGameInstance* NLGameInstance = Cast<UNLGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject)))
        {
            if (UTaggedWeaponInfoList* InfoList = NLGameInstance->TaggedWeaponInfoList)
            {
                return InfoList->FindTaggedWeaponInfoByTag(WeaponTag);
            }
        }
    }
    return nullptr;
}

const FWeaponAnims* UNLFunctionLibrary::GetWeaponAnimInfoByTag(const UObject* WorldContextObject, const FGameplayTag& WeaponTag)
{
    if (const FWeaponInfo* Info = GetWeaponInfoByTag(WorldContextObject, WeaponTag))
    {
        return &Info->WeaponAnimInfo->WeaponAnimInfo;
    }
    return nullptr;
}

const FWeaponAnims* UNLFunctionLibrary::GetArmsAnimInfoByTag(const UObject* WorldContextObject, const FGameplayTag& WeaponTag)
{
    if (const FWeaponInfo* Info = GetWeaponInfoByTag(WorldContextObject, WeaponTag))
    {
        return &Info->ArmsAnimInfo->WeaponAnimInfo;
    }
    return nullptr;
}
