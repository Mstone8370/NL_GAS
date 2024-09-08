// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/NLWidgetController.h"
#include "PlayersStatWidgetController.generated.h"

class APlayerState;

/**
 * 
 */
UCLASS()
class NL_GAS_API UPlayersStatWidgetController : public UNLWidgetController
{
	GENERATED_BODY()

public:
	virtual void BindEvents() override;

	virtual void BroadcastInitialValues() override;

protected:
	UPROPERTY()
	TSet<APlayerState*> BindedPlayers;
};
