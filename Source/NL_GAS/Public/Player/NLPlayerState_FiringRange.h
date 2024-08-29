// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/NLPlayerState.h"
#include "NLPlayerState_FiringRange.generated.h"

/**
 * 간단한 휘발성 플레이어 스탯을 저장하는 플레이어 스테이트
 */
UCLASS()
class NL_GAS_API ANLPlayerState_FiringRange : public ANLPlayerState
{
	GENERATED_BODY()

protected:
	int32 FireCount = 0;
	int32 HitCount = 0;
	int32 CriticalHitCount = 0;
	int32 KillCount = 0;
	int32 DeathCount = 0;
	
public:
	// Player Stats
	virtual void SetPlayerStat(FGameplayTag StatTag, int32 Value) override;
	virtual void AddPlayerStat(FGameplayTag StatTag, int32 ValueAdded = 1) override;
	virtual int32 GetPlayerStat(FGameplayTag StatTag) const override;
	virtual void BroadcastAllPlayerStats() const override;

	UFUNCTION(BlueprintCallable, Category = "PlayerStats")
	void ResetPlayerStats();
};
