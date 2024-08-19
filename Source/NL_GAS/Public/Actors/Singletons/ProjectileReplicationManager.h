// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ProjectileReplicationManager.generated.h"

struct FProjectileSpawnInfo;
struct FProjectileInfo;

UCLASS()
class NL_GAS_API AProjectileReplicationManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectileReplicationManager();

protected:
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastProjectiles(const FGameplayTag& ProjectileTag, const TArray<FProjectileSpawnInfo>& SpawnInfos);

public:
	UFUNCTION(BlueprintCallable)
	void ReplicateProjectiles(const AController* ParticleInstigator, const FGameplayTag& ProjectileTag, const TArray<FProjectileSpawnInfo>& SpawnInfos, bool bExcludeInstigator = true);

};
