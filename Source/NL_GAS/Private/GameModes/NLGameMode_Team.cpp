// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/NLGameMode_Team.h"

#include "Player/NLPlayerController.h"
#include "GameStates/NLGameState_Team.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Actors/NLPlayerStart.h"
#include "Interface/CombatInterface.h"
#include "NLGameplayTags.h"
#include "EngineUtils.h"

ANLGameMode_Team::ANLGameMode_Team(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    RoundState = RoundState_WaitingToStart;
}

void ANLGameMode_Team::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (ANLGameState_Team* NLGameStateTeam = GetGameState<ANLGameState_Team>())
    {
        if (APlayerState* PS = NewPlayer->GetPlayerState<APlayerState>())
        {
            NLGameStateTeam->AssignTeamToPlayer(PS);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("PostLogin NumPlayers: %d, NumSpectators: %d"), NumPlayers, NumSpectators);

    if (RoundState == RoundState_WaitingToStart)
    {
        FTimerDelegate Dele;
        //GetWorldTimerManager().SetTimer(RoundStartTimer, Dele, LoginWaitTime, false);
    }
}

void ANLGameMode_Team::BeginPlay()
{
    Super::BeginPlay();
}

bool ANLGameMode_Team::CheckPlayerStartCondition(APlayerStart* PlayerStart, APlayerController* Player, bool bInitial)
{
    if (!IsValid(PlayerStart))
    {
        return false;
    }

    if (ANLPlayerStart* NLPlayerStart = Cast<ANLPlayerStart>(PlayerStart))
    {
        if (bInitial && !NLPlayerStart->bInitial)
        {
            return false;
        }
        if (NLPlayerStart->Team != 0)
        {
            if (ANLGameState_Team* GameStateTeam = GetGameState<ANLGameState_Team>())
            {
                if (GameStateTeam->FindTeam(Player) != NLPlayerStart->Team)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

ANLGameState_Team* ANLGameMode_Team::GetNLGS_Team()
{
    if (!NLGameState_Team)
    {
        NLGameState_Team = GetGameState<ANLGameState_Team>();
    }
    return NLGameState_Team;
}

void ANLGameMode_Team::SetRoundState(FGameplayTag NewState)
{
    if (RoundState == NewState)
    {
        return;
    }

    RoundState = NewState;

    OnRoundStateSet();

    if (GetNLGS_Team())
    {
        GetNLGS_Team()->SetRoundState(NewState);
    }
}

void ANLGameMode_Team::OnRoundStateSet()
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

void ANLGameMode_Team::HandleRoundIsWaitingToStart()
{
}

void ANLGameMode_Team::HandleRoundHasStarted()
{
    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        if (APlayerController* PlayerController = Iterator->Get())
        {
            RespawnPlayer(PlayerController, true);
        }
    }
}

void ANLGameMode_Team::HandleRoundHasEnded()
{
}

void ANLGameMode_Team::StartRound()
{
    SetRoundState(RoundState_InProgress);
}
