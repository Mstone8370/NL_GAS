// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/NLGameMode.h"
#include "GameplayTagContainer.h"
#include "NLGameMode_Team.generated.h"

class ANLGameState_Team;

/**
 * 
 */
UCLASS()
class NL_GAS_API ANLGameMode_Team : public ANLGameMode
{
	GENERATED_BODY()
	
public:
	ANLGameMode_Team(const FObjectInitializer& ObjectInitializer);

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void StartMatch() override;

protected:
	virtual void BeginPlay() override;

	virtual bool CheckPlayerStartCondition(APlayerStart* PlayerStart, APlayerController* Player, bool bInitial) override;

	virtual bool ReadyToStartMatch_Implementation() override;

	virtual void HandleMatchHasStarted() override;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ANLGameState_Team> NLGameState_Team;

	ANLGameState_Team* GetNLGS_Team();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 TargetScore = 10;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag RoundState;

	virtual void SetRoundState(FGameplayTag NewState);

	virtual void OnRoundStateSet();

	virtual void HandleRoundIsWaitingToStart();

	virtual void HandleRoundHasStarted();

	virtual void HandleRoundHasEnded();

	FTimerHandle MatchStartTimer;

	float LoginWaitTime = 5.f;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE int32 GetTargetScore() const { return TargetScore; }

	UFUNCTION(BlueprintCallable)
	void StartRound();
};
