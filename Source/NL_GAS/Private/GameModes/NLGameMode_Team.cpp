// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/NLGameMode_Team.h"

#include "Player/NLPlayerController.h"
#include "GameStates/NLGameState_Team.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Actors/NLPlayerStart.h"
#include "Interface/CombatInterface.h"
#include "NLGameplayTags.h"
#include "GameFramework/GameSession.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CheatManager.h"

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
}

void ANLGameMode_Team::StartPlay()
{
    Super::StartPlay();

    if (GetNLGS_Team())
    {
        GetNLGS_Team()->RoundStartWaitTime = RoundStartWaitTime;
        GetNLGS_Team()->TargetScore = TargetScore;
    }
}

void ANLGameMode_Team::StartMatch()
{
    if (HasMatchStarted())
    {
        // Already started
        return;
    }

    //Let the game session override the StartMatch function, in case it wants to wait for arbitration
    if (GameSession->HandleStartMatchRequest())
    {
        return;
    }

    if (GetWorldTimerManager().IsTimerActive(MatchStartTimer))
    {
        return;
    }

    // 매치 시작 조건은 맞지만 일정 시간동안 더 기다렸다가 시작함.
    FTimerDelegate Dele;
    Dele.BindLambda(
        [this]()
        {
            SetMatchState(MatchState::InProgress);
        }
    );
    GetWorldTimerManager().SetTimer(MatchStartTimer, Dele, LoginWaitTime, false);

    if (GetNLGS_Team())
    {
        GetNLGS_Team()->Multicast_OnStartMatchTimerSet(LoginWaitTime);
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

bool ANLGameMode_Team::ReadyToStartMatch_Implementation()
{
    if (GetMatchState() == MatchState::WaitingToStart)
    {
        if (NumPlayers + NumBots > 1)
        {
            return true;
        }
    }
    return false;
}

void ANLGameMode_Team::HandleMatchHasStarted()
{
    // Codes from AGameMode::HandleMatchHasStarted

    GameSession->HandleMatchHasStarted();

    // Make sure level streaming is up to date before triggering NotifyMatchStarted
    GEngine->BlockTillLevelStreamingCompleted(GetWorld());

    // First fire BeginPlay, if we haven't already in waiting to start match
    GetWorldSettings()->NotifyBeginPlay();

    // Then fire off match started
    GetWorldSettings()->NotifyMatchStarted();

    // if passed in bug info, send player to right location
    const FString BugLocString = UGameplayStatics::ParseOption(OptionsString, TEXT("BugLoc"));
    const FString BugRotString = UGameplayStatics::ParseOption(OptionsString, TEXT("BugRot"));
    if (!BugLocString.IsEmpty() || !BugRotString.IsEmpty())
    {
        for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
        {
            APlayerController* PlayerController = Iterator->Get();
            if (PlayerController && PlayerController->CheatManager != nullptr)
            {
                PlayerController->CheatManager->BugItGoString(BugLocString, BugRotString);
            }
        }
    }

    if (IsHandlingReplays() && GetGameInstance() != nullptr)
    {
        GetGameInstance()->StartRecordingReplay(GetWorld()->GetMapName(), GetWorld()->GetMapName());
    }

    StartRound();
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
    DisableActionAllPlayer();
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

    DisableActionAllPlayer();

    // 라운드 시작되면 일정 시간동안 대기
    FTimerDelegate Dele;
    Dele.BindLambda(
        [this]()
        {
            EnableActionAllPlayer();
        }
    );
    GetWorldTimerManager().SetTimer(RoundStartTimer, Dele, RoundStartWaitTime, false);
}

void ANLGameMode_Team::HandleRoundHasEnded()
{
    DisableActionAllPlayer();
}

void ANLGameMode_Team::StartRound()
{
    SetRoundState(RoundState_InProgress);
}

void ANLGameMode_Team::EndRound(int32 WinTeam, int32 WinTeamScore)
{
    SetRoundState(RoundState_End);

    if (WinTeamScore == TargetScore)
    {
        GetNLGS_Team()->Multicast_OnMatchWinTeamDecided(WinTeam);
    }
    else
    {
        GetNLGS_Team()->Multicast_OnRoundWinTeamDecided(WinTeam);

        // TODO: 일정시간 후에 라운드 재시작
    }
}

bool ANLGameMode_Team::HasRoundStarted() const
{
    return RoundState.MatchesTagExact(RoundState_InProgress);
}

bool ANLGameMode_Team::HasRoundEnded() const
{
    return !HasRoundStarted();
}

void ANLGameMode_Team::EnableActionAllPlayer()
{
    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        if (APlayerController* PlayerController = Iterator->Get())
        {
            if (ANLPlayerController* NLPC = Cast<ANLPlayerController>(PlayerController))
            {
                NLPC->EnableAction();
            }
        }
    }
}

void ANLGameMode_Team::DisableActionAllPlayer()
{
    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        if (APlayerController* PlayerController = Iterator->Get())
        {
            if (ANLPlayerController* NLPC = Cast<ANLPlayerController>(PlayerController))
            {
                NLPC->DisableAction();
            }
        }
    }
}
