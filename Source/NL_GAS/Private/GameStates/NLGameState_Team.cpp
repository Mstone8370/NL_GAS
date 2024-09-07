// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/NLGameState_Team.h"

#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/NLPlayerState.h"
#include "NLGameplayTags.h"
#include "Kismet/GameplayStatics.h"

void ANLGameState_Team::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(ANLGameState_Team, TeamInfo, COND_None, REPNOTIFY_OnChanged);
    DOREPLIFETIME_CONDITION_NOTIFY(ANLGameState_Team, TeamScoreInfo, COND_None, REPNOTIFY_OnChanged);
    DOREPLIFETIME_CONDITION_NOTIFY(ANLGameState_Team, RoundState, COND_None, REPNOTIFY_OnChanged);
    DOREPLIFETIME_CONDITION_NOTIFY(ANLGameState_Team, TargetScore, COND_None, REPNOTIFY_OnChanged);
    DOREPLIFETIME_CONDITION_NOTIFY(ANLGameState_Team, RoundStartWaitTime, COND_None, REPNOTIFY_OnChanged);
}

void ANLGameState_Team::AssignTeamToPlayer(APlayerState* PlayerState)
{
    int32 NewTeam = ChooseTeam(PlayerState);

    if (NewTeam)
    {
        if (TeamInfo.Team_1.Contains(PlayerState))
        {
            TeamInfo.Team_1.Remove(PlayerState);
        }
        if (TeamInfo.Team_2.Contains(PlayerState))
        {
            TeamInfo.Team_2.Remove(PlayerState);
        }

        if (NewTeam == 1)
        {
            TeamInfo.Team_1.Add(PlayerState);
        }
        else if (NewTeam == 2)
        {
            TeamInfo.Team_2.Add(PlayerState);
        }
    }

    if (ANLPlayerState* NLPS = Cast<ANLPlayerState>(PlayerState))
    {
        NLPS->TeamAssigned(NewTeam);
    }
}

void ANLGameState_Team::OnPlayerDied(APlayerState* SourcePlayer, APlayerState* TargetPlayer)
{
    if (!TargetPlayer)
    {
        return;
    }

    PlayerSurvivalStatus.Add(TargetPlayer, false);
}

bool ANLGameState_Team::IsSameTeam(const APlayerState* A, const APlayerState* B) const
{
    int32 Team_A = 0;
    int32 Team_B = 0;

    if (TeamInfo.Team_1.Contains(A))
    {
        Team_A = 1;
    }
    else if (TeamInfo.Team_2.Contains(A))
    {
        Team_A = 2;
    }

    if (TeamInfo.Team_1.Contains(B))
    {
        Team_B = 1;
    }
    else if (TeamInfo.Team_2.Contains(B))
    {
        Team_B = 2;
    }

    return (Team_A != 0 && Team_A == Team_B);
}

int32 ANLGameState_Team::FindTeam(const APlayerState* PlayerState)
{
    if (PlayerState)
    {
        if (TeamInfo.Team_1.Contains(PlayerState))
        {
            return 1;
        }
        if (TeamInfo.Team_2.Contains(PlayerState))
        {
            return 2;
        }
        if (PlayerArray.Contains(PlayerState))
        {
            return 0;
        }
    }
    return -1;
}

int32 ANLGameState_Team::FindTeam(const APlayerController* PlayerController)
{
    if (PlayerController)
    {
        if (APlayerState* PS = PlayerController->GetPlayerState<APlayerState>())
        {
            return FindTeam(PS);
        }
    }
    return -1;
}

int32 ANLGameState_Team::ChooseTeam(APlayerState* Player)
{
    int32 Ret = 0;

    const int32& Team1Num = TeamInfo.Team_1.Num();
    const int32& Team2Num = TeamInfo.Team_2.Num();
    if (Team1Num == Team2Num)
    {
        Ret = UKismetMathLibrary::RandomInteger(2) + 1;
    }
    else if (Team1Num < Team2Num)
    {
        Ret = 1;
    }
    else
    {
        Ret = 2;
    }

    return Ret;
}

void ANLGameState_Team::OnRep_TeamInfo(FTeamInfo& OldTeamInfo)
{
    TSet<APlayerState*> Checked;

    for (APlayerState* PS : TeamInfo.Team_1)
    {
        if (ANLPlayerState* NLPS = Cast<ANLPlayerState>(PS))
        {
            NLPS->TeamAssigned(1);
        }
        Checked.Add(PS);
    }

    for (APlayerState* PS : TeamInfo.Team_2)
    {
        if (ANLPlayerState* NLPS = Cast<ANLPlayerState>(PS))
        {
            NLPS->TeamAssigned(2);
        }
        Checked.Add(PS);
    }

    for (APlayerState* PS : PlayerArray)
    {
        if (!Checked.Contains(PS))
        {
            if (ANLPlayerState* NLPS = Cast<ANLPlayerState>(PS))
            {
                NLPS->TeamAssigned(0);
            }
        }
    }
}

void ANLGameState_Team::OnRep_TeamScoreInfo()
{
    TeamScoreUpdated.ExecuteIfBound(TeamScoreInfo);
}

void ANLGameState_Team::OnRep_RoundState()
{
    if (RoundState == RoundState_WaitingToStart)
    {
        HandleRoundIsWaitingToStart();
    }
    else if (RoundState == RoundState_InProgress)
    {
        HandleRoundHasStarted();
    }
    else if (RoundState == RoundState_End)
    {
        HandleRoundHasEnded();
    }
}

void ANLGameState_Team::HandleRoundIsWaitingToStart()
{
    UE_LOG(LogTemp, Warning, TEXT("[NLGameState] Round is waiting to start"));
}

void ANLGameState_Team::HandleRoundHasStarted()
{
    UE_LOG(LogTemp, Warning, TEXT("[NLGameState] Round has started"));

    RoundStarted.ExecuteIfBound(RoundStartWaitTime);
}

void ANLGameState_Team::HandleRoundHasEnded()
{
    UE_LOG(LogTemp, Warning, TEXT("[NLGameState] Round has ended"));
}

int32 ANLGameState_Team::GetScoreTeam(int32 Team) const
{
    if (Team == 1)
    {
        return TeamScoreInfo.Team_1;
    }
    else if (Team == 2)
    {
        return TeamScoreInfo.Team_2;
    }
    return 0;
}

void ANLGameState_Team::SetScore(int32 Team, int32 Value)
{
    if (Team == 1)
    {
        TeamScoreInfo.Team_1 = FMath::Max(0, Value);
    }
    else if (Team == 2)
    {
        TeamScoreInfo.Team_2 = FMath::Max(0, Value);
    }
}

void ANLGameState_Team::AddScore(int32 Team, int32 Value)
{
    if (Team == 1)
    {
        SetScore(1, GetScore().Team_1 + Value);
    }
    else if (Team == 2)
    {
        SetScore(2, GetScore().Team_2 + Value);
    }
}

void ANLGameState_Team::ResetScore()
{
    SetScore(1, 0);
    SetScore(2, 0);
}

void ANLGameState_Team::SetRoundState(FGameplayTag NewState)
{
    if (GetLocalRole() == ROLE_Authority)
    {
        RoundState = NewState;

        OnRep_RoundState();
    }
}

void ANLGameState_Team::Multicast_OnStartMatchTimerSet_Implementation(float Time)
{
    UE_LOG(LogTemp, Warning, TEXT("StartMatch Timer Set: %f"), Time);
}

void ANLGameState_Team::Multicast_OnRoundWinTeamDecided_Implementation(int32 WinTeam)
{
    UE_LOG(LogTemp, Warning, TEXT("Round Win Team Decided: %d"), WinTeam);

    RoundWinTeamDecided.ExecuteIfBound(WinTeam);
}

void ANLGameState_Team::Multicast_OnMatchWinTeamDecided_Implementation(int32 WinTeam)
{
    UE_LOG(LogTemp, Warning, TEXT("Match Win Team Decided: %d"), WinTeam);

    MatchWinTeamDecided.ExecuteIfBound(WinTeam);
}
