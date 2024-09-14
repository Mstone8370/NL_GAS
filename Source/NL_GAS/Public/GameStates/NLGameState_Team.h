// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameStates/NLGameState.h"
#include "GameplayTagContainer.h"
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

USTRUCT(Blueprintable)
struct FTeamScoreInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 Team_1 = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Team_2 = 0;
};

DECLARE_DELEGATE_OneParam(FTeamScoreUpdatedSignature, FTeamScoreInfo);
DECLARE_DELEGATE_OneParam(FTargetScoreUpdatedSignature, int32 /*TargetScore*/);
DECLARE_DELEGATE_OneParam(FRoundWinTeamDecidedSignature, int32 /*WinTeam*/);
DECLARE_DELEGATE_OneParam(FMatchWinTeamDecidedSignature, int32 /*WinTeam*/);
DECLARE_DELEGATE_OneParam(FRoundIntroBeginSignature, int32 /*RoundIntroTime*/);
DECLARE_DELEGATE_OneParam(FRoundInProgressSignature, int32 /*RoundTimeLimit*/);
DECLARE_DELEGATE_OneParam(FRoundTimeLimitUpdatedSignature, int32 /*RoundTimeLimit*/);

DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerJoinedToTeamSignature, const APlayerState* /*Player*/, int32 /*Team*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerLeavedFromTeamSignature, const APlayerState* /*Player*/, int32 /*Team*/);

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

	virtual void OnPlayerDied(APlayerState* SourcePlayer, APlayerState* TargetPlayer) override;

	bool IsSameTeam(const APlayerState* A, const APlayerState* B) const;

	int32 FindTeam(const APlayerState* PlayerState);
	int32 FindTeam(const APlayerController* PlayerController);

	FTeamScoreUpdatedSignature TeamScoreUpdated;
	FTargetScoreUpdatedSignature TargetScoreUpdated;
	FRoundWinTeamDecidedSignature RoundWinTeamDecided;
	FMatchWinTeamDecidedSignature MatchWinTeamDecided;
	FRoundIntroBeginSignature RoundIntroBegin;
	FRoundInProgressSignature RoundInProgress;
	FRoundTimeLimitUpdatedSignature RoundTimeLimitUpdated;

	FPlayerJoinedToTeamSignature PlayerJoinedToTeam;
	FPlayerLeavedFromTeamSignature PlayerLeavedFromTeam;

	UPROPERTY(BlueprintReadOnly, Replicated)
	float RoundIntroTime;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_TargetScore)
	int32 TargetScore;

	UPROPERTY(BlueprintReadOnly, Replicated)
	int32 RoundTimeLimit;

protected:
	int32 ChooseTeam(APlayerState* Player);

	UPROPERTY(BlueprintReadOnly)
	FTeamInfo TeamInfo;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_TeamScoreInfo)
	FTeamScoreInfo TeamScoreInfo;

	UFUNCTION()
	virtual void OnRep_TeamScoreInfo();

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_RoundState)
	FGameplayTag RoundState;

	UFUNCTION()
	virtual void OnRep_RoundState();

	UFUNCTION()
	void OnRep_TargetScore();

	UPROPERTY()
	TMap<APlayerState*, bool> PlayerSurvivalStatus;

	virtual void HandleRoundIsWaitingToStart();

	virtual void HandleRoundIntro();

	virtual void HandleRoundHasStarted();

	virtual void HandleRoundHasEnded();

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE FTeamScoreInfo GetScore() const { return TeamScoreInfo; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE int32 GetTargetScore() const { return TargetScore; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetScoreTeam(int32 Team) const;

	UFUNCTION(BlueprintCallable)
	void SetScore(int32 Team, int32 Value);

	UFUNCTION(BlueprintCallable)
	void AddScore(int32 Team, int32 Value = 1);

	UFUNCTION(BlueprintCallable)
	void ResetScore();

	virtual void SetRoundState(FGameplayTag NewState);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnStartMatchTimerSet(float Time);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnRoundWinTeamDecided(int32 WinTeam);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnMatchWinTeamDecided(int32 WinTeam);

	virtual void AddPlayerState(APlayerState* PlayerState) override;

	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	virtual void AddPlayerStateToTeam(APlayerState* PlayerState, int32 Team);

	virtual void RemovePlayerStateToTeam(APlayerState* PlayerState, int32 Team);
};
