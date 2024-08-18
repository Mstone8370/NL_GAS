// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ParticleReplicationManager.generated.h"

class UParticleData;
struct FParticleInfo;
struct FParticleSpawnInfo;

UCLASS()
class NL_GAS_API AParticleReplicationManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AParticleReplicationManager();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastParticles(const FGameplayTag& ParticleTag, const TArray<FParticleSpawnInfo>& SpawnInfos);

public:
	UFUNCTION(BlueprintCallable)
	void ReplicateParticles(const AController* ParticleInstigator, const FGameplayTag& ParticleTag, const TArray<FParticleSpawnInfo>& SpawnInfos, bool bExcludeInstigator = true);
};
