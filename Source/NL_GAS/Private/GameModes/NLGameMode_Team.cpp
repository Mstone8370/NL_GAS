// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/NLGameMode_Team.h"

#include "GameStates/NLGameState_Team.h"
#include "GameFramework/PlayerState.h"

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

void ANLGameMode_Team::AssignTeamToPlayers()
{
}

void ANLGameMode_Team::HandleMatchHasStarted()
{
    Super::HandleMatchHasStarted();

}
