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
    // �������� ������ �߻�ü�� ���ø�����Ʈ �ϴ°Ͱ� �����ϹǷ� instigator�� ���õȰ� ����� �ʿ�� ���µ�.
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
                // TODO: �߻�ü ��ġ �����Ϸ��� ���⿡�� �߻�ü�� �����Ȱ� �˷��ټ�������.
            }
        }
    }
    else
    {
        MulticastProjectiles(ProjectileTag, SpawnInfos);
    }
}
