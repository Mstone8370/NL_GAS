// Fill out your copyright notice in the Description page of Project Settings.


#include "Util/ActorActionUtility_HitboxInfo.h"

#include "NLFunctionLibrary.h"
#include "Data/NLDataTableRows.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/HitboxComponent.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Kismet/DataTableFunctionLibrary.h"
#include "Util/HitboxUtilActor.h"

#include "UnrealEd.h"
#include "Editor/EditorEngine.h"

#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "AssetRegistry/AssetRegistryModule.h"

void UActorActionUtility_HitboxInfo::SaveHitbox()
{
    TArray<AActor*> SelectedActors = UEditorUtilityLibrary::GetSelectionSet();
    for (AActor* SelectedActor : SelectedActors)
    {
        ASkeletalMeshActor* SkeletalMeshActor = Cast<ASkeletalMeshActor>(SelectedActor);
        if (!SkeletalMeshActor)
        {
            continue;
        }

        const FString DataTablePath = UNLFunctionLibrary::MakeHitboxInfoDataTablePath(SkeletalMeshActor->GetSkeletalMeshComponent());

        // Load or Create DataTable
        UDataTable* DataTable = nullptr;
        if (UEditorAssetLibrary::DoesAssetExist(DataTablePath))
        {
            DataTable = Cast<UDataTable>(UEditorAssetLibrary::LoadAsset(DataTablePath));
        }
        else
        {
            DataTable = CreateDataTableAsset(DataTablePath);
            if (!DataTable)
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to create new DataTable asset."));
                continue;
            }
        }
        DataTable->RowStruct = FHitboxInfoRow::StaticStruct();

        // Get Hitbox Info
        TMap<FName, TArray<FHitboxInfoRow>> HitboxInfos;
        GetAttachedHitboxInfo(SelectedActor, HitboxInfos);

        // Convert to JSON format
        TArray<TSharedPtr<FJsonValue>> JsonArray;
        for (const TTuple<FName, TArray<FHitboxInfoRow>>& Item : HitboxInfos)
        {
            FName BoneName = Item.Key;
            const TArray<FHitboxInfoRow>& InfoArray = Item.Value;
            for (int i = 0; i < InfoArray.Num(); i++)
            {
                const FHitboxInfoRow& Info = InfoArray[i];

                TSharedPtr<FJsonObject> Json_Row = MakeShareable(new FJsonObject);

                Json_Row->SetStringField("Name", FString("Hitbox_") + BoneName.ToString() + FString("_") + FString::FromInt(i));

                Json_Row->SetStringField("BoneName", Info.BoneName.ToString());

                TSharedPtr<FJsonObject> Json_Location = MakeShared<FJsonObject>();
                Json_Location->SetNumberField("X", Info.Location.X);
                Json_Location->SetNumberField("Y", Info.Location.Y);
                Json_Location->SetNumberField("Z", Info.Location.Z);
                Json_Row->SetObjectField("Location", Json_Location);

                TSharedPtr<FJsonObject> Json_Rotation = MakeShared<FJsonObject>();
                Json_Rotation->SetNumberField("Pitch", Info.Rotation.Pitch);
                Json_Rotation->SetNumberField("Yaw", Info.Rotation.Yaw);
                Json_Rotation->SetNumberField("Roll", Info.Rotation.Roll);
                Json_Row->SetObjectField("Rotation", Json_Rotation);

                TSharedPtr<FJsonObject> Json_Extend = MakeShared<FJsonObject>();
                Json_Extend->SetNumberField("X", Info.Extend.X);
                Json_Extend->SetNumberField("Y", Info.Extend.Y);
                Json_Extend->SetNumberField("Z", Info.Extend.Z);
                Json_Row->SetObjectField("Extend", Json_Extend);

                Json_Row->SetBoolField("bIsWeakHitbox", Info.bIsWeakHitbox);

                TSharedPtr<FJsonValueObject> JsonValue = MakeShareable(new FJsonValueObject(Json_Row));
                JsonArray.Add(JsonValue);
            }
        }
            
        // To JSON string
        FString JsonString;
        const TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
        FJsonSerializer::Serialize(JsonArray, JsonWriter);

        // Fill DataTable
        UDataTableFunctionLibrary::FillDataTableFromJSONString(DataTable, JsonString);

        // Save DataTable Asset
        UEditorAssetLibrary::SaveAsset(DataTablePath);
    }
}

void UActorActionUtility_HitboxInfo::LoadHitbox(TSubclassOf<AActor> HitboxActorClassOverride)
{
    UClass* HitboxActorClass = AHitboxUtilActor::StaticClass();
    if (HitboxActorClassOverride)
    {
        HitboxActorClass = HitboxActorClassOverride;
    }

    ClearAllChild();

    TArray<AActor*> SelectedActors = UEditorUtilityLibrary::GetSelectionSet();
    for (AActor* SelectedActor : SelectedActors)
    {
        ASkeletalMeshActor* SkeletalMeshActor = Cast<ASkeletalMeshActor>(SelectedActor);
        if (!SkeletalMeshActor)
        {
            continue;
        }

        const FString DataTablePath = UNLFunctionLibrary::MakeHitboxInfoDataTablePath(SkeletalMeshActor->GetSkeletalMeshComponent());

        UDataTable* DataTable = nullptr;
        if (UEditorAssetLibrary::DoesAssetExist(DataTablePath))
        {
            DataTable = Cast<UDataTable>(UEditorAssetLibrary::LoadAsset(DataTablePath));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Hitbox Info [%s] is not exist."), *DataTablePath);
            continue;
        }

        TArray<FName> RowNames = DataTable->GetRowNames();
        for (const FName& RowName : RowNames)
        {
            FHitboxInfoRow* Info = DataTable->FindRow<FHitboxInfoRow>(RowName, "");
            if (!Info)
            {
                continue;
            }

            AActor* NewActor = SpawnHitboxActor(
                SelectedActor,
                HitboxActorClass,
                *Info,
                RowName.ToString(),
                SkeletalMeshActor->GetSkeletalMeshComponent(),
                Info->BoneName
            );
        }
    }
}

void UActorActionUtility_HitboxInfo::ClearAllChild()
{
    TArray<AActor*> SelectedActors = UEditorUtilityLibrary::GetSelectionSet();
    for (AActor* SelectedActor : SelectedActors)
    {
        TArray<AActor*> AttachedActors;
        SelectedActor->GetAttachedActors(AttachedActors, true, true);
        for (AActor* Child : AttachedActors)
        {
            Child->Destroy();
        }
    }
}

void UActorActionUtility_HitboxInfo::SymmetrizeHitbox(bool bMirror)
{
    TArray<AActor*> SelectedActors = UEditorUtilityLibrary::GetSelectionSet();
    for (AActor* SelectedActor : SelectedActors)
    {
        ASkeletalMeshActor* SkeletalMeshActor = Cast<ASkeletalMeshActor>(SelectedActor);
        if (!SkeletalMeshActor)
        {
            continue;
        }

        // Get Hitbox Info
        TMap<FName, TArray<FHitboxInfoRow>> HitboxInfos;
        GetAttachedHitboxInfo(SelectedActor, HitboxInfos);

        for (const TTuple<FName, TArray<FHitboxInfoRow>>& Item : HitboxInfos)
        {
            FString BoneNameString = Item.Key.ToString();
            int32 BoneNameLength = BoneNameString.Len();
            if (BoneNameString[BoneNameLength - 1] != 'l' && BoneNameString[BoneNameLength - 1] != 'r')
            {
                continue;
            }

            FString TargetBoneNameString = FString(BoneNameString);
            TargetBoneNameString[BoneNameLength - 1] = TargetBoneNameString[BoneNameLength - 1] == 'l' ? 'r' : 'l';
            FName TargetBoneName(TargetBoneNameString);
            if (HitboxInfos.Contains(TargetBoneName))
            {
                continue;
            }

            const TArray<FHitboxInfoRow>& InfoArray = Item.Value;
            for (int i = 0; i < InfoArray.Num(); i++)
            {
                const FHitboxInfoRow& Info = InfoArray[i];
                const FString ActorName = FString("Hitbox_") + TargetBoneNameString + FString("_") + FString::FromInt(i);
                FHitboxInfoRow NewInfo(Info);
                NewInfo.Location *= bMirror ? 1 : -1;

                AActor* NewActor = SpawnHitboxActor(
                    SelectedActor,
                    AHitboxUtilActor::StaticClass(),
                    NewInfo,
                    ActorName,
                    SkeletalMeshActor->GetSkeletalMeshComponent(),
                    TargetBoneName
                );
            }
        }
    }
}

UDataTable* UActorActionUtility_HitboxInfo::CreateDataTableAsset(FString FullPath, bool bSyncBrowserToObject)
{
    UPackage* Package = CreatePackage(*FullPath);
    Package->FullyLoad();

    UDataTable* DataTable = NewObject<UDataTable>(Package, *FPaths::GetCleanFilename(FullPath), RF_Public | RF_Standalone | RF_MarkAsRootSet);
    DataTable->RowStruct = FHitboxInfoRow::StaticStruct();

    Package->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(DataTable);

    FString PackageFileName = FPackageName::LongPackageNameToFilename(
        FullPath,
        FPackageName::GetAssetPackageExtension()
    );

    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = EObjectFlags::RF_Public | EObjectFlags::RF_Standalone;
    SaveArgs.Error = GError;
    SaveArgs.bForceByteSwapping = true;
    SaveArgs.bWarnOfLongFilename = true;
    SaveArgs.SaveFlags = SAVE_NoError;

    bool bSaved = UPackage::SavePackage(
        Package,
        DataTable,
        *PackageFileName,
        SaveArgs
    );

    if (bSaved)
    {
        if (bSyncBrowserToObject)
        {
            // 컨텐츠 브라우저에 에셋 표시
            TArray<UObject*> ObjectsToSync;
            ObjectsToSync.Add(DataTable);
            GEditor->SyncBrowserToObjects(ObjectsToSync);
        }

        return DataTable;
    }
    return nullptr;
}

void UActorActionUtility_HitboxInfo::GetAttachedHitboxInfo(AActor* ParentActor, TMap<FName, TArray<FHitboxInfoRow>>& OutHitboxInfos)
{
    OutHitboxInfos.Empty();

    TArray<AActor*> AttachedActors;
    ParentActor->GetAttachedActors(AttachedActors);
    for (const AActor* BoxActor : AttachedActors)
    {
        if (UHitboxComponent* HitboxComp = Cast<UHitboxComponent>(BoxActor->GetRootComponent()))
        {
            FHitboxInfoRow Info;
            Info.BoneName = BoxActor->GetAttachParentSocketName();
            Info.Location = HitboxComp->GetRelativeLocation();
            Info.Rotation = HitboxComp->GetRelativeRotation();
            Info.Extend = HitboxComp->GetScaledBoxExtent();
            Info.bIsWeakHitbox = HitboxComp->IsWeakHitbox();

            TArray<FHitboxInfoRow>& InfoArray = OutHitboxInfos.FindOrAdd(Info.BoneName, TArray<FHitboxInfoRow>());
            InfoArray.Add(Info);
        }
    }
}

AActor* UActorActionUtility_HitboxInfo::SpawnHitboxActor(UObject* WorldContextObject, UClass* Class, const FHitboxInfoRow& HitboxInfo, FString ActorName, USkeletalMeshComponent* ParentMeshComponent, FName ParentBoneName)
{
    FActorSpawnParameters ActorSpawnParam;
    ActorSpawnParam.bNoFail = true;
    ActorSpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor* NewActor = WorldContextObject->GetWorld()->SpawnActor<AActor>(
        Class,
        HitboxInfo.Location,
        HitboxInfo.Rotation,
        ActorSpawnParam
    );
    if (!NewActor)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to Spawn HitboxActor. RowName: %s"), *ActorName);
        return nullptr;
    }

    NewActor->SetActorLabel(ActorName);
    if (UHitboxComponent* HitboxComp = Cast<UHitboxComponent>(NewActor->GetRootComponent()))
    {
        HitboxComp->SetBoxExtent(HitboxInfo.Extend);
        HitboxComp->SetIsWeakHitbox(HitboxInfo.bIsWeakHitbox);
    }
    NewActor->AttachToComponent(
        ParentMeshComponent,
        FAttachmentTransformRules::KeepRelativeTransform,
        ParentBoneName
    );

    return NewActor;
}
