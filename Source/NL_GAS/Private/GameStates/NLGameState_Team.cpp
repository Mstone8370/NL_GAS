// Fill out your copyright notice in the Description page of Project Settings.


#include "GameStates/NLGameState_Team.h"

#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/NLPlayerState.h"

void ANLGameState_Team::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(ANLGameState_Team, TeamInfo, COND_None, REPNOTIFY_OnChanged);
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
