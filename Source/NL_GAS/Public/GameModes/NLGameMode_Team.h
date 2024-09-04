// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/NLGameMode.h"
#include "NLGameMode_Team.generated.h"

/**
 * 
 */
UCLASS()
class NL_GAS_API ANLGameMode_Team : public ANLGameMode
{
	GENERATED_BODY()
	
public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

protected:
	virtual void RespawnPlayer(APlayerController* PC);

	virtual bool CheckPlayerStartCondition(APlayerStart* PlayerStart, APlayerController* Player, bool bInitial, bool bCheckTeam) override;

};
