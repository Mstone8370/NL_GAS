// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/NLGameMode_Team.h"

#include "Player/NLPlayerController.h"
#include "GameStates/NLGameState_Team.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerStart.h"
#include "Actors/NLPlayerStart.h"

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

void ANLGameMode_Team::RespawnPlayer(APlayerController* PC)
{
    AActor* Start = ChoosePlayerStartByCondition(PC, false, true);
    if (IsValid(Start))
    {
        if (APawn* PCPawn = PC->GetPawn())
        {
            PCPawn->SetActorLocation(Start->GetActorLocation());
        }
        if (ANLPlayerController* NLPC = Cast<ANLPlayerController>(PC))
        {
            NLPC->OnRespawned(Start->GetActorForwardVector());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to find PlayerStart"));
    }
}

bool ANLGameMode_Team::CheckPlayerStartCondition(APlayerStart* PlayerStart, APlayerController* Player, bool bInitial, bool bCheckTeam)
{
    if (!IsValid(PlayerStart))
    {
        return false;
    }

    if (bInitial || bCheckTeam)
    {
        if (ANLPlayerStart* NLPlayerStart = Cast<ANLPlayerStart>(PlayerStart))
        {
            if (bInitial && !NLPlayerStart->bInitial)
            {
                return false;
            }
            if (bCheckTeam && NLPlayerStart->Team != 0)
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
    }
    return true;
}
