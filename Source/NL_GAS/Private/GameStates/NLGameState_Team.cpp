// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/NLGameState_Team.h"

#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/NLPlayerState.h"
#include "NLGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Interface/PlayerInterface.h"

void ANLGameState_Team::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(ANLGameState_Team, TeamScoreInfo, COND_None, REPNOTIFY_OnChanged);
    DOREPLIFETIME_CONDITION_NOTIFY(ANLGameState_Team, RoundState, COND_None, REPNOTIFY_OnChanged);
    DOREPLIFETIME_CONDITION_NOTIFY(ANLGameState_Team, TargetScore, COND_None, REPNOTIFY_OnChanged);
    DOREPLIFETIME_CONDITION_NOTIFY(ANLGameState_Team, RoundTimeLimit, COND_None, REPNOTIFY_OnChanged);
    DOREPLIFETIME_CONDITION_NOTIFY(ANLGameState_Team, RoundIntroTime, COND_None, REPNOTIFY_OnChanged);
}

void ANLGameState_Team::AssignTeamToPlayer(APlayerState* PlayerState)
{
    int32 NewTeam = ChooseTeam(PlayerState);
    AddPlayerStateToTeam(PlayerState, NewTeam);

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
    else if (RoundState == RoundState_RoundIntro)
    {
        HandleRoundIntro();
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

void ANLGameState_Team::OnRep_TargetScore()
{
    TargetScoreUpdated.ExecuteIfBound(TargetScore);
}

void ANLGameState_Team::HandleRoundIsWaitingToStart()
{
    UE_LOG(LogTemp, Warning, TEXT("[NLGameState] Round is waiting to start"));
}

void ANLGameState_Team::HandleRoundIntro()
{
    UE_LOG(LogTemp, Warning, TEXT("[NLGameState] Round Intro"));

    RoundIntroBegin.ExecuteIfBound(RoundIntroTime);
}

void ANLGameState_Team::HandleRoundHasStarted()
{
    UE_LOG(LogTemp, Warning, TEXT("[NLGameState] Round has started"));

    RoundInProgress.ExecuteIfBound(RoundTimeLimit);

    // ó�� �� ������ ���⸦ ��� �޾Ƶ� ���� �ʴ� ��츦 ���ؼ� ���⿡�� �ѹ� �� Ȯ��
    if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this))
    {
        const TArray<ULocalPlayer*>& LocalPlayers = GameInstance->GetLocalPlayers();
        for (ULocalPlayer* LocalPlayer : LocalPlayers)
        {
            if (APawn* Pawn = LocalPlayer->PlayerController->GetPawn())
            {
                if (Pawn->Implements<UPlayerInterface>())
                {
                    IPlayerInterface::Execute_TrySwapWeaponSlot(Pawn, 0);
                }
            }
        }
    }
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

/**
* ���ÿ��� �÷��̾� ������Ʈ�� �����Ǿ�����, ���ŵɶ� ������ ���� ������Ʈ�� ã�Ƽ� 
* �Ʒ��� �Լ��� (AddPlayerState, RemovePlayerState)
* ȣ���ϴ� ������� ������ PlayerArray�� ����ȭ��. PlayerArray�� ���ø�����Ʈ���� ����.
* �̷� ������� �÷��̾� ���� ����� ����ȭ �ϴ� ����� �����غ��� ����.
* �÷��̾� ������Ʈ�� ���� ���ø�����Ʈ�ǰ� �ؼ� ������Ʈ �Ǹ� �ش� ����� �˸��� �Լ��� ȣ���ϴ� �������
* ���ÿ����� ���� ����Ʈ�� ����ȭ�Ҽ� ����.
* �׸��� ���ÿ� �ִ� �÷��̾� ������Ʈ�� ���� �Ҵ�Ǿ��ٸ� �׶����� UI���� ǥ���� �� �ִ� ����.
*/

void ANLGameState_Team::AddPlayerState(APlayerState* PlayerState)
{
    Super::AddPlayerState(PlayerState);
}

void ANLGameState_Team::RemovePlayerState(APlayerState* PlayerState)
{
    Super::RemovePlayerState(PlayerState);

    int32 LeavedTeam = 0;
    if (TeamInfo.Team_1.Contains(PlayerState))
    {
        TeamInfo.Team_1.Remove(PlayerState);
        LeavedTeam = 1;
    }
    if (TeamInfo.Team_2.Contains(PlayerState))
    {
        TeamInfo.Team_2.Remove(PlayerState);
        LeavedTeam = 2;
    }
    PlayerLeavedFromTeam.Broadcast(PlayerState, LeavedTeam);
}

void ANLGameState_Team::AddPlayerStateToTeam(APlayerState* PlayerState, int32 Team)
{
    int32 LeavedTeam = 0;
    if (TeamInfo.Team_1.Contains(PlayerState))
    {
        TeamInfo.Team_1.Remove(PlayerState);
        LeavedTeam = 1;
    }
    if (TeamInfo.Team_2.Contains(PlayerState))
    {
        TeamInfo.Team_2.Remove(PlayerState);
        LeavedTeam = 2;
    }
    PlayerLeavedFromTeam.Broadcast(PlayerState, LeavedTeam);

    if (Team == 1)
    {
        TeamInfo.Team_1.AddUnique(PlayerState);
    }
    else if (Team == 2)
    {
        TeamInfo.Team_2.AddUnique(PlayerState);
    }
    PlayerJoinedToTeam.Broadcast(PlayerState, Team);
}

void ANLGameState_Team::RemovePlayerStateToTeam(APlayerState* PlayerState, int32 Team)
{
}
