// Fill out your copyright notice in the Description page of Project Settings.


#include "NLFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameModes/NLGameMode.h"
#include "NLGameInstance.h"
#include "Data/WeaponInfo.h"
#include "HUD/NLHUD.h"
#include "EditorAssetLibrary.h"
#include "AbilitySystemComponent.h"

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
            FString AssetPath = "/Game/Blueprints/Data/DA_TaggedWeaponInfoList";
            if (UObject* Asset = UEditorAssetLibrary::LoadAsset(AssetPath))
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

void UNLFunctionLibrary::ApplyDamageEffect(const FDamageEffectParams& Params)
{
    if (!Params.SourceASC)
    {
        return;
    }

    FGameplayEffectContextHandle ContextHandle = Params.SourceASC->MakeEffectContext();
    ContextHandle.AddSourceObject(Params.SourceASC->GetAvatarActor());
    ContextHandle.AddHitResult(Params.HitResult);

    if (FNLGameplayEffectContext* NLContext = static_cast<FNLGameplayEffectContext*>(ContextHandle.Get()))
    {
        // TODO: Set NLGameplayEffectContext
    }

    FGameplayEffectSpecHandle SpecHandle = Params.SourceASC->MakeOutgoingSpec(Params.DamageGameplayEffectClass, 1.f, ContextHandle);
    // TODO: Set IncommingDamage attribute value (set by caller magnitude)

    Params.SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), Params.TargetASC);
}
