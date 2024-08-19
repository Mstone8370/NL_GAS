// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Singletons/ProjectileReplicationManager.h"

#include "NLFunctionLibrary.h"
#include "Player/NLPlayerController.h"

AProjectileReplicationManager::AProjectileReplicationManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    bAlwaysRelevant = true;
}

/*
void AProjectileReplicationManager::MulticastParticles_Implementation(const FGameplayTag& ParticleTag, const TArray<FParticleSpawnInfo>& SpawnInfos)
{
    UNLFunctionLibrary::SpawnMultipleParticleByTag(this, ParticleTag, SpawnInfos);
}

void AProjectileReplicationManager::ReplicateParticles(const AController* ParticleInstigator, const FGameplayTag& ParticleTag, const TArray<FParticleSpawnInfo>& SpawnInfos, bool bExcludeInstigator)
{
    if (bExcludeInstigator)
    {
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            APlayerController* PC = It->Get();
            if (PC != ParticleInstigator)
            {
                if (ANLPlayerController* NLPC = Cast<ANLPlayerController>(PC))
                {
                    NLPC->ReplicateParticlesToClient(ParticleTag, SpawnInfos);
                }
            }
        }
    }
    else
    {
        MulticastParticles(ParticleTag, SpawnInfos);
    }
}
*/
