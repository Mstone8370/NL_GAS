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

protected:
	UFUNCTION()
	virtual void OnPlayerDead(AController* SourceController, AController* TargetController, FGameplayTag DamageType);
};
