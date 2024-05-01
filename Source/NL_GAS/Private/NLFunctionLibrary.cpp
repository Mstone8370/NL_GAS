// Fill out your copyright notice in the Description page of Project Settings.


#include "NLFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameModes/NLGameMode.h"
#include "NLGameInstance.h"
#include "Data/WeaponInfo.h"
#include "HUD/NLHUD.h"
#include "EditorAssetLibrary.h"

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
#if WITH_EDITOR
        else
        {
            if (UObject* Asset = UEditorAssetLibrary::LoadAsset(FString("/Script/NL_GAS.TaggedWeaponInfoList'/Game/Blueprints/Data/DA_TaggedWeaponInfoList.DA_TaggedWeaponInfoList'")))
            {
                if (UTaggedWeaponInfoList* InfoList = Cast<UTaggedWeaponInfoList>(Asset))
                {
                    return InfoList->FindTaggedWeaponInfoByTag(WeaponTag);
                }
            }
        }
#endif
    }
    return nullptr;
}

const FTaggedAnimMontageInfo* UNLFunctionLibrary::GetAnimMontageByTag(const UObject* WorldContextObject, const FGameplayTag& WeaponTag, const FGameplayTag& MontageTag)
{
    if (const FWeaponInfo* Info = GetWeaponInfoByTag(WorldContextObject, WeaponTag))
    {
        if (const UTaggedAnimMontages* MontageDataAsset = Info->AnimMontages)
        {
            if (MontageDataAsset->Data.Contains(MontageTag))
            {
                return &MontageDataAsset->Data[MontageTag];
            }
        }
    }
    return nullptr;
}

const float UNLFunctionLibrary::GetRecoilResetTimeByTag(const UObject* WorldContextObject, const FGameplayTag& WeaponTag)
{
    if (const FWeaponInfo* Info = GetWeaponInfoByTag(WorldContextObject, WeaponTag))
    {
        return Info->RecoilOffsetResetTime;
    }
    return .2f;
}
