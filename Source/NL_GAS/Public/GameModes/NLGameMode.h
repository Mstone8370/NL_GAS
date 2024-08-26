// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "NLGameMode.generated.h"

class AParticleReplicationManager;
class AProjectileReplicationManager;

/**
 * 
 */
UCLASS()
class NL_GAS_API ANLGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float PlayerRespawnTime = 3.f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnPlayerDead(AActor* SourceActor, AActor* TargetActor, FGameplayTag DamageType);

	virtual void SetRespawnTime(AActor* TargetActor);

	virtual void MulticastKillLog(AActor* SourceActor, AActor* TargetActor, FGameplayTag DamageType);

	virtual void RespawnPlayer(APlayerController* PC);

private:
	UPROPERTY()
	TObjectPtr<AParticleReplicationManager> ParticleReplicationManager;

	UPROPERTY()
	TObjectPtr<AProjectileReplicationManager> ProjectileReplicationManager;

	void SpawnOrGetSingleton(AActor*& OutActor, TSubclassOf<AActor> ActorClass);

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	AParticleReplicationManager* GetParticleReplicationManager() const { return ParticleReplicationManager; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AProjectileReplicationManager* GetProjectileReplicationManager() const { return ProjectileReplicationManager; }

};
