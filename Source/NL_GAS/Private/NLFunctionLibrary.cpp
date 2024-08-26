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
#include "NLGameplayTags.h"
#include "Data/NLDataTableRows.h"
#include "Components/HitboxComponent.h"
#include "Engine/ObjectLibrary.h"
#include "GameFramework/PlayerState.h"
#include "Data/ParticleData.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/DecalComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Data/ProjectileData.h"
#include "Actors/NLProjectile.h"

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
            FString AssetPath = "/Game/Blueprints/Data/Weapon/DA_WeaponInfoList";
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
        NLContext->DamageType = TSharedPtr<FGameplayTag>(new FGameplayTag(Params.DamageType));
        NLContext->bCanCriticalHit = Params.bCanCriticalHit;
        NLContext->bIsRadialDamage = Params.bIsRadialDamage;
        if (NLContext->bIsRadialDamage)
        {
            NLContext->RadialDamageOrigin = Params.RadialDamageOrigin;
            NLContext->RadialDamageInnerRadius = Params.RadialDamageInnerRadius;
            NLContext->RadialDamageOuterRadius = Params.RadialDamageOuterRadius;
        }
        NLContext->KnockbackMagnitude = Params.KnockbackMagnitude;
        if (Params.bHasDamageOrigin)
        {
            NLContext->AddOrigin(Params.DamageOrigin);
        }
    }

    FGameplayEffectSpecHandle SpecHandle = Params.SourceASC->MakeOutgoingSpec(Params.DamageGameplayEffectClass, 1.f, ContextHandle);
    
    float Damage = Params.DamageScalableFloat.GetValueAtLevel(Params.TravelDistance);
    UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Attribute_Meta_IncomingDamage, Damage);

    Params.SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), Params.TargetASC);
}

FString UNLFunctionLibrary::MakeHitboxInfoDataTablePath(const USkeletalMeshComponent* SkeletalMeshComponent)
{
    FString FullPath = "";
    const FString HitboxInfoFolderPath = "/Game/Blueprints/Data/Hitbox";

    if (SkeletalMeshComponent && SkeletalMeshComponent->GetSkinnedAsset())
    {
        const FString MeshAssetName = GetNameSafe(SkeletalMeshComponent->GetSkinnedAsset());
        const FString AssetName = "DT_HitboxInfo_" + MeshAssetName;
        FullPath = FPaths::ConvertRelativePathToFull(HitboxInfoFolderPath, AssetName);
    }

    return FullPath;
}

void UNLFunctionLibrary::LoadHitboxComponents(USkeletalMeshComponent* SkeletalMeshComponent)
{
    const FString DataTablePath = UNLFunctionLibrary::MakeHitboxInfoDataTablePath(SkeletalMeshComponent);
    if (!UNLFunctionLibrary::AssetExists(DataTablePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find HitboxInfo DataTable Asset: %s"), *DataTablePath);
        return;
    }

    UDataTable* DataTable = LoadObject<UDataTable>(NULL, *DataTablePath);
    if (!DataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("HitboxInfo DataTable Asset load failed: %s"), *DataTablePath);
        return;
    }

    TArray<FName> RowNames = DataTable->GetRowNames();
    for (const FName& RowName : RowNames)
    {
        FHitboxInfoRow* Info = DataTable->FindRow<FHitboxInfoRow>(RowName, "");
        if (!Info)
        {
            continue;
        }

        UHitboxComponent* HitboxComp = NewObject<UHitboxComponent>(SkeletalMeshComponent);
        HitboxComp->RegisterComponent();
        HitboxComp->SetRelativeLocationAndRotation(Info->Location,Info->Rotation);
        HitboxComp->SeHitboxExtent(Info->Extend);
        HitboxComp->SetIsWeakHitbox(Info->bIsWeakHitbox);
        HitboxComp->AttachToComponent(
            SkeletalMeshComponent,
            FAttachmentTransformRules::KeepRelativeTransform,
            Info->BoneName
        );
    }
}

bool UNLFunctionLibrary::AssetExists(FString FullPath)
{
    const FString FolderPath = FPaths::GetPath(FullPath);
    const FName AssetName = FName(FPaths::GetCleanFilename(FullPath));

    UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(nullptr, false, GIsEditor);
    ObjectLibrary->LoadAssetDataFromPath(FolderPath);

    TArray<FAssetData> AssetDatas;
    ObjectLibrary->GetAssetDataList(AssetDatas);

    for (const FAssetData& AssetData : AssetDatas)
    {
        if (AssetData.AssetName.IsEqual(AssetName))
        {
            return true;
        }
    }
    return false;
}

FString UNLFunctionLibrary::GetPlayerName(AActor* Actor)
{
    if (APawn* Pawn = Cast<APawn>(Actor))
    {
        if (APlayerState* PS = Pawn->GetPlayerState())
        {
            return PS->GetPlayerNameCustom();
        }
    }
    return GetNameSafe(Actor);
}

AActor* UNLFunctionLibrary::GetAbilitySystemAvatarActor(UAbilitySystemComponent* ASC)
{
    if (ASC)
    {
        return ASC->GetAvatarActor();
    }
    return nullptr;
}

AController* UNLFunctionLibrary::GetAbilitySystemController(UAbilitySystemComponent* ASC)
{
    if (AActor* AvatarActor = GetAbilitySystemAvatarActor(ASC))
    {
        if (APawn* Pawn = Cast<APawn>(AvatarActor))
        {
            return Pawn->GetController();
        }
    }
    return nullptr;
}

APlayerController* UNLFunctionLibrary::GetAbilitySystemPlayerController(UAbilitySystemComponent* ASC)
{
    if (AController* Controller = GetAbilitySystemController(ASC))
    {
        return Cast<APlayerController>(Controller);
    }
    return nullptr;
}

void UNLFunctionLibrary::SpawnSingleParticleByParticleInfo(const UObject* WorldContextObject, const FParticleInfo& ParticleInfo, const FParticleSpawnInfo& SpawnInfo)
{
    if (ParticleInfo.HitImpactDecalMaterial)
    {
        UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(
            WorldContextObject,
            ParticleInfo.HitImpactDecalMaterial,
            ParticleInfo.DecalSize,
            SpawnInfo.Location,
            SpawnInfo.Normal.Rotation(),
            ParticleInfo.DecalLifeSpan
        );
        if (Decal)
        {
            Decal->SetFadeScreenSize(ParticleInfo.FadeScreenSize);
        }
    }

    if (ParticleInfo.HitImpactFX)
    {
        FFXSystemSpawnParameters SpawnParams;
        SpawnParams.WorldContextObject = WorldContextObject;
        SpawnParams.SystemTemplate = ParticleInfo.HitImpactFX;
        SpawnParams.Location = SpawnInfo.Location;
        SpawnParams.Rotation = SpawnInfo.Normal.Rotation();
        if (UNiagaraComponent* Niagara = UNiagaraFunctionLibrary::SpawnSystemAtLocationWithParams(SpawnParams))
        {
            Niagara->SetVectorParameter(FName("Direction"), SpawnInfo.Normal);
        }
    }
}

void UNLFunctionLibrary::SpawnSingleParticleByTag(const UObject* WorldContextObject, const FGameplayTag& ParticleTag, const FParticleSpawnInfo& SpawnInfo)
{
    if (UNLGameInstance* NLGameInstance = Cast<UNLGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject)))
    {
        if (UParticleData* ParticleData = NLGameInstance->ParticleData)
        {
            if (const FParticleInfo* ParticleInfo = ParticleData->FindParticleDataByTag(ParticleTag))
            {
                UNLFunctionLibrary::SpawnSingleParticleByParticleInfo(WorldContextObject, *ParticleInfo, SpawnInfo);
            }
        }
    }
}

void UNLFunctionLibrary::SpawnMultipleParticleByParticleInfo(const UObject* WorldContextObject, const FParticleInfo& ParticleInfo, const TArray<FParticleSpawnInfo>& SpawnInfos)
{
    for (const FParticleSpawnInfo& SpawnInfo : SpawnInfos)
    {
        UNLFunctionLibrary::SpawnSingleParticleByParticleInfo(WorldContextObject, ParticleInfo, SpawnInfo);
    }
}

void UNLFunctionLibrary::SpawnMultipleParticleByTag(const UObject* WorldContextObject, const FGameplayTag& ParticleTag, const TArray<FParticleSpawnInfo>& SpawnInfos)
{
    if (UNLGameInstance* NLGameInstance = Cast<UNLGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject)))
    {
        if (UParticleData* ParticleData = NLGameInstance->ParticleData)
        {
            if (const FParticleInfo* ParticleInfo = ParticleData->FindParticleDataByTag(ParticleTag))
            {
                for (const FParticleSpawnInfo& SpawnInfo : SpawnInfos)
                {
                    UNLFunctionLibrary::SpawnSingleParticleByParticleInfo(WorldContextObject, *ParticleInfo, SpawnInfo);
                }
            }
        }
    }
}

ANLProjectile* UNLFunctionLibrary::SpawnSingleProjectileByProjectileInfo(const UObject* WorldContextObject, const FProjectileInfo& ProjectileInfo, const FProjectileSpawnInfo& SpawnInfo, APawn* Instigator)
{
    ANLProjectile* Ret = nullptr;

    if (WorldContextObject && WorldContextObject->GetWorld())
    {
        if (ProjectileInfo.ProjectileClass)
        {
            FActorSpawnParameters SpawnParam;
            SpawnParam.Instigator = Instigator;
            Ret = WorldContextObject->GetWorld()->SpawnActor<ANLProjectile>(
                ProjectileInfo.ProjectileClass,
                SpawnInfo.Location,
                SpawnInfo.Direction.Rotation(),
                SpawnParam
            );
            Ret->Id = SpawnInfo.Id;
        }
    }

    return Ret;
}

ANLProjectile* UNLFunctionLibrary::SpawnSingleProjectileByTag(const UObject* WorldContextObject, const FGameplayTag& ProjectileTag, const FProjectileSpawnInfo& SpawnInfo, APawn* Instigator)
{
    ANLProjectile* Ret = nullptr;

    if (UNLGameInstance* NLGameInstance = Cast<UNLGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject)))
    {
        if (UProjectileData* ProjectileData = NLGameInstance->ProjectileData)
        {
            if (const FProjectileInfo* ProjectileInfo = ProjectileData->FindProjectileDataByTag(ProjectileTag))
            {
                Ret = UNLFunctionLibrary::SpawnSingleProjectileByProjectileInfo(WorldContextObject, *ProjectileInfo, SpawnInfo, Instigator);
            }
        }
    }

    return Ret;
}

void UNLFunctionLibrary::SpawnMultipleProjectileByProjectileInfo(const UObject* WorldContextObject, const FProjectileInfo& ProjectileInfo, const TArray<FProjectileSpawnInfo>& SpawnInfos, APawn* Instigator, TArray<ANLProjectile*>& OutProjectiles)
{
    OutProjectiles.Empty();

    for (const FProjectileSpawnInfo& SpawnInfo : SpawnInfos)
    {
        OutProjectiles.Add(
            UNLFunctionLibrary::SpawnSingleProjectileByProjectileInfo(WorldContextObject, ProjectileInfo, SpawnInfo, Instigator)
        );
    }
}

void UNLFunctionLibrary::SpawnMultipleProjectileByTag(const UObject* WorldContextObject, const FGameplayTag& ProjectileTag, const TArray<FProjectileSpawnInfo>& SpawnInfos, APawn* Instigator, TArray<ANLProjectile*>& OutProjectiles)
{
    OutProjectiles.Empty();

    if (UNLGameInstance* NLGameInstance = Cast<UNLGameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject)))
    {
        if (UProjectileData* ProjectileData = NLGameInstance->ProjectileData)
        {
            if (const FProjectileInfo* ProjectileInfo = ProjectileData->FindProjectileDataByTag(ProjectileTag))
            {
                for (const FProjectileSpawnInfo& SpawnInfo : SpawnInfos)
                {
                    OutProjectiles.Add(
                        UNLFunctionLibrary::SpawnSingleProjectileByProjectileInfo(WorldContextObject, *ProjectileInfo, SpawnInfo, Instigator)
                    );
                }
            }
        }
    }
}
