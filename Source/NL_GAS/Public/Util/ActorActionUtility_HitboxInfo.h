// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorActionUtility.h"
#include "ActorActionUtility_HitboxInfo.generated.h"

class UDataTable;
struct FHitboxInfoRow;

/**
 * 
 */
UCLASS()
class NL_GAS_API UActorActionUtility_HitboxInfo : public UActorActionUtility
{
	GENERATED_BODY()
	
public:
	UFUNCTION(CallInEditor)
	void SaveHitbox();

	UFUNCTION(CallInEditor, BlueprintCallable)
	void LoadHitbox(TSubclassOf<AActor> HitboxActorClassOverride);

	UFUNCTION(CallInEditor, BlueprintCallable)
	void ClearAllChild();

	UFUNCTION(CallInEditor, BlueprintCallable)
	void SymmetrizeHitbox(bool bMirror = false);

protected:
	UDataTable* CreateDataTableAsset(FString Path, FString Name, bool bSyncBrowserToObject = true);

	void GetAttachedHitboxInfo(AActor* ParentActor, TMap<FName, TArray<FHitboxInfoRow>>& OutHitboxInfos);

	AActor* SpawnHitboxActor(UObject* WorldContextObject, UClass* Class, const FHitboxInfoRow& HitboxInfo, FString ActorName, USkeletalMeshComponent* ParentMeshComponent, FName ParentBoneName);
};
