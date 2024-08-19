// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ProjectileReplicationManager.generated.h"

UCLASS()
class NL_GAS_API AProjectileReplicationManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectileReplicationManager();

	/*
protected:
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastParticles(const FGameplayTag& ParticleTag, const TArray<FParticleSpawnInfo>& SpawnInfos);

public:
	UFUNCTION(BlueprintCallable)
	void ReplicateParticles(const AController* ParticleInstigator, const FGameplayTag& ParticleTag, const TArray<FParticleSpawnInfo>& SpawnInfos, bool bExcludeInstigator = true);

	*/
};
