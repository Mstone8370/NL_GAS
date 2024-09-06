// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameStates/NLGameState_Team.h"
#include "NLGameState_TDM.generated.h"

/**
 * 
 */
UCLASS()
class NL_GAS_API ANLGameState_TDM : public ANLGameState_Team
{
	GENERATED_BODY()
	
public:
	virtual void OnPlayerDied(APlayerState* SourcePlayer, APlayerState* TargetPlayer) override;
};
