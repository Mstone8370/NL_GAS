// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActorActionUtility.h"
#include "ActorActionUtility_HitboxInfo.generated.h"

class UDataTable;

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
	void LoadHitbox(TSubclassOf<AActor> HitboxActorClass);

	UFUNCTION(CallInEditor, BlueprintCallable)
	void ClearAllChild();

protected:
	UDataTable* CreateDataTableAsset(FString Path, FString Name, bool bSyncBrowserToObject = true);
};
