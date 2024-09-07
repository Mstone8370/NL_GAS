// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/NLGameMode_Team.h"
#include "GameplayTagContainer.h"
#include "NLGameMode_TDM.generated.h"

/**
 * 
 */
UCLASS()
class NL_GAS_API ANLGameMode_TDM : public ANLGameMode_Team
{
	GENERATED_BODY()
	
protected:
	virtual void OnPlayerDied(AActor* SourceActor, AActor* TargetActor, FGameplayTag DamageType) override;

	virtual void OnRoundTimeLimitExpired() override;
};
