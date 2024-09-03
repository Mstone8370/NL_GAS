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
	
public:public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void AssignTeamToPlayers();

protected:
	virtual void HandleMatchHasStarted() override;
};
