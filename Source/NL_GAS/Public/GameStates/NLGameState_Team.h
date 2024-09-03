// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameStates/NLGameState.h"
#include "NLGameState_Team.generated.h"

USTRUCT(Blueprintable)
struct FTeamInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<APlayerState*> Team_1;

	UPROPERTY(BlueprintReadOnly)
	TArray<APlayerState*> Team_2;
};

/**
 * 
 */
UCLASS()
class NL_GAS_API ANLGameState_Team : public ANLGameState
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void AssignTeamToPlayer(APlayerState* PlayerState);

protected:
	int32 ChooseTeam(APlayerState* Player);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_TeamInfo)
	FTeamInfo TeamInfo;

	UFUNCTION()
	void OnRep_TeamInfo(FTeamInfo& OldTeamInfo);
};
