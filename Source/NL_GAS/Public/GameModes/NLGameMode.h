// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "NLGameMode.generated.h"

class AParticleReplicationManager;
class AProjectileReplicationManager;
class APlayerStart;
class ANLGameState;

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

	virtual void Tick(float DeltaSeconds) override;

	// 폰 생성, 리스폰, 리셋을 다 처리하는 함수
	UFUNCTION(BlueprintCallable)
	virtual void RespawnPlayer(APlayerController* PC, bool bInitial = false);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnPlayerDied(AActor* SourceActor, AActor* TargetActor, FGameplayTag DamageType);

	virtual bool ShouldSetRespawnTimer() const;

	virtual void SetRespawnTime(AActor* TargetActor);

	virtual void MulticastKillLog(APlayerState* SourcePS, APlayerState* TargetPS, FGameplayTag DamageType);

	virtual AActor* ChoosePlayerStartByCondition(APlayerController* Player, bool bInitial);

	virtual bool CheckPlayerStartCondition(APlayerStart* PlayerStart, APlayerController* Player, bool bInitial);

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ANLGameState> NLGameState;

	ANLGameState* GetNLGS();

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
