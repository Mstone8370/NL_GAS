// Fill out your copyright notice in the Description page of Project Settings.


#include "Util/ActorActionUtility_HitboxInfo.h"

#include "UnrealEd.h"
#include "Editor/EditorEngine.h"

#include "EditorUtilityLibrary.h"
#include "Animation/SkeletalMeshActor.h"
#include "Components/HitboxComponent.h"
#include "EditorAssetLibrary.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Data/NLDataTableRows.h"
#include "Kismet/DataTableFunctionLibrary.h"
#include "UObject/Package.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"

void UActorActionUtility_HitboxInfo::SaveHitbox()
{
    TArray<AActor*> SelectedActors = UEditorUtilityLibrary::GetSelectionSet();
    for (AActor* SelectedActor : SelectedActors)
    {
        if (ASkeletalMeshActor* SKA = Cast<ASkeletalMeshActor>(SelectedActor))
        {
            FString MeshAssetName = GetNameSafe(SKA->GetSkeletalMeshComponent()->GetSkinnedAsset());
            FString AssetPath = "/Game/Blueprints/Data/Hitbox/";
            FString AssetName = "DA_HitboxInfo_" + MeshAssetName;
            FString FullPath = AssetPath + AssetName;

            UDataTable* DT = nullptr;
            if (UEditorAssetLibrary::DoesAssetExist(FullPath))
            {
                DT = Cast<UDataTable>(UEditorAssetLibrary::LoadAsset(FullPath));
            }
            else
            {
                DT = CreateDataTableAsset(AssetPath, AssetName);
                if (!DT)
                {
                    UE_LOG(LogTemp, Error, TEXT("Failed to create new DataTable asset."));
                    continue;
                }
            }

            // Save Hitbox Info
            TArray<FHitboxInfoRow> HitboxInfos;
            TArray<AActor*> AttachedActors;
            SelectedActor->GetAttachedActors(AttachedActors);
            for (const AActor* BoxActor : AttachedActors)
            {
                if (UHitboxComponent* HitboxComp = Cast<UHitboxComponent>(BoxActor->GetRootComponent()))
                {
                    FHitboxInfoRow Info;
                    Info.BoneName = BoxActor->GetAttachParentSocketName();
                    Info.Location = HitboxComp->GetRelativeLocation();
                    Info.Rotation = HitboxComp->GetRelativeRotation();
                    Info.Extend = HitboxComp->GetScaledBoxExtent();
                    Info.IsWeakHitbox = HitboxComp->IsWeakHitbox();

                    HitboxInfos.Add(Info);
                }
            }

            // Convert to JSON format
            TArray<TSharedPtr<FJsonValue>> JsonArray;
            for (int32 i = 0; i < HitboxInfos.Num(); i++)
            {
                const FHitboxInfoRow& Info = HitboxInfos[i];

                TSharedPtr<FJsonObject> Json_Row = MakeShareable(new FJsonObject);

                Json_Row->SetStringField("Name", FString("Row_") + FString::FromInt(i));

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

                Json_Row->SetBoolField("IsWeakHitbox", Info.IsWeakHitbox);

                TSharedPtr<FJsonValueObject> JsonValue = MakeShareable(new FJsonValueObject(Json_Row));
                JsonArray.Add(JsonValue);
            }

            FString JsonString;
            const TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
            FJsonSerializer::Serialize(JsonArray, JsonWriter);

            // Fill DataTable
            UDataTableFunctionLibrary::FillDataTableFromJSONString(DT, JsonString);

            // Save DataTable Asset
            UEditorAssetLibrary::SaveAsset(FullPath);
        }
    }
}

void UActorActionUtility_HitboxInfo::LoadHitbox(TSubclassOf<AActor> HitboxActorClass)
{
    if (!HitboxActorClass)
    {
        UE_LOG(LogTemp, Error, TEXT("HitboxActorClass should not be null"));
        return;
    }

    ClearAllChild();

    FActorSpawnParameters ActorSpawnParam;
    ActorSpawnParam.bNoFail = true;
    ActorSpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    TArray<AActor*> SelectedActors = UEditorUtilityLibrary::GetSelectionSet();
    for (AActor* SelectedActor : SelectedActors)
    {
        if (ASkeletalMeshActor* SKA = Cast<ASkeletalMeshActor>(SelectedActor))
        {
            FString MeshAssetName = GetNameSafe(SKA->GetSkeletalMeshComponent()->GetSkinnedAsset());
            FString AssetPath = "/Game/Blueprints/Data/Hitbox/";
            FString AssetName = "DA_HitboxInfo_" + MeshAssetName;
            FString FullPath = AssetPath + AssetName;

            UDataTable* DT = nullptr;
            if (UEditorAssetLibrary::DoesAssetExist(FullPath))
            {
                DT = Cast<UDataTable>(UEditorAssetLibrary::LoadAsset(FullPath));
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Hitbox Info of [%s] is not exist."), *MeshAssetName);
                continue;
            }

            TArray<FName> RowNames = DT->GetRowNames();
            for (const FName& RowName : RowNames)
            {
                if (FHitboxInfoRow* Info = DT->FindRow<FHitboxInfoRow>(RowName, ""))
                {
                    AActor* NewActor = SelectedActor->GetWorld()->SpawnActor<AActor>(
                        HitboxActorClass,
                        Info->Location,
                        Info->Rotation,
                        ActorSpawnParam
                    );
                    if (!NewActor)
                    {
                        UE_LOG(LogTemp, Error, TEXT("Failed to Spawn HitboxActor. RowName: %s"), *RowName.ToString());
                        continue;
                    }

                    NewActor->AttachToComponent(
                        SKA->GetSkeletalMeshComponent(),
                        FAttachmentTransformRules::KeepRelativeTransform,
                        Info->BoneName
                    );
                    if (UHitboxComponent* HitboxComp = Cast<UHitboxComponent>(NewActor->GetRootComponent()))
                    {
                        HitboxComp->SetBoxExtent(Info->Extend);
                        HitboxComp->SetIsWeakHitbox(Info->IsWeakHitbox);
                    }
                }
            }
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

UDataTable* UActorActionUtility_HitboxInfo::CreateDataTableAsset(FString Path, FString Name, bool bSyncBrowserToObject)
{
    FString FullPath = Path + Name;

    UPackage* Package = CreatePackage(*FullPath);
    Package->FullyLoad();

    UDataTable* DataTable = NewObject<UDataTable>(Package, *Name, RF_Public | RF_Standalone | RF_MarkAsRootSet);
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
