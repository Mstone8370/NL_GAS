// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "NLGameMode.generated.h"

/**
 * 
 */
UCLASS()
class NL_GAS_API ANLGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float PlayerRespawnTime = 3.f;

protected:
	UFUNCTION()
	virtual void OnPlayerDead(AActor* SourceActor, AActor* TargetActor, FGameplayTag DamageType);

	virtual void SetRespawnTime(AActor* TargetActor);

	virtual void MulticastKillLog(AActor* SourceActor, AActor* TargetActor, FGameplayTag DamageType);

	virtual void RespawnPlayer(APlayerController* PC);
};
