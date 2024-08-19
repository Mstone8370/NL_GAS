// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Singletons/ProjectileReplicationManager.h"

#include "NLFunctionLibrary.h"
#include "Data/ProjectileData.h"
#include "Player/NLPlayerController.h"

AProjectileReplicationManager::AProjectileReplicationManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    bAlwaysRelevant = true;
}

void AProjectileReplicationManager::MulticastProjectiles_Implementation(const FGameplayTag& ProjectileTag, const TArray<FProjectileSpawnInfo>& SpawnInfos)
{
    // 서버에서 생성된 발사체를 레플리케이트 하는것과 유사하므로 instigator와 관련된걸 고려할 필요는 없는듯.
    TArray<ANLProjectile*> DummyArray;
    UNLFunctionLibrary::SpawnMultipleProjectileByTag(this, ProjectileTag, SpawnInfos, DummyArray);
}

void AProjectileReplicationManager::ReplicateProjectiles(const AController* ParticleInstigator, const FGameplayTag& ProjectileTag, const TArray<FProjectileSpawnInfo>& SpawnInfos, bool bExcludeInstigator)
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
                    NLPC->ReplicateProjectilesToClient(ProjectileTag, SpawnInfos);
                }
            }
            else
            {
                // TODO: 발사체 위치 보정하려면 여기에서 발사체가 스폰된걸 알려줄수있을듯.
            }
        }
    }
    else
    {
        MulticastProjectiles(ProjectileTag, SpawnInfos);
    }
}
