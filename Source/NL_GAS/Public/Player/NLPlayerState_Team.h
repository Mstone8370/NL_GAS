// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/NLPlayerState.h"
#include "GameplayTagContainer.h"
#include "NLPlayerState_Team.generated.h"

/**
 * 
 */
UCLASS()
class NL_GAS_API ANLPlayerState_Team : public ANLPlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
protected:
	int32 FireCount = 0;

	int32 HitCount = 0;

	int32 CriticalHitCount = 0;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_KillCount)
	int32 KillCount = 0;

	UFUNCTION()
	void OnRep_KillCount();

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DeathCount)
	int32 DeathCount = 0;

	UFUNCTION()
	void OnRep_DeathCount();

public:
	// Player Stats
	virtual void SetPlayerStat(FGameplayTag StatTag, int32 Value) override;
	virtual void AddPlayerStat(FGameplayTag StatTag, int32 ValueAdded = 1) override;
	virtual int32 GetPlayerStat(FGameplayTag StatTag) const override;
	virtual void BroadcastPlayerAllStats() const override;

	virtual void ResetPlayerStats() override;
};
