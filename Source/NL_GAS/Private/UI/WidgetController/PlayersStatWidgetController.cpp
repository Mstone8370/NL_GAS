// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/PlayersStatWidgetController.h"

#include "Player/NLPlayerState.h"
#include "GameStates/NLGameState_Team.h"
#include "Engine/World.h"
#include "EngineUtils.h"

void UPlayersStatWidgetController::BindEvents()
{
    UWorld* World = GetWorld();
    if (ANLGameState_Team* NLGS_Team = World->GetGameState<ANLGameState_Team>())
    {
        NLGS_Team->PlayerJoinedToTeam.AddUObject(this, &UPlayersStatWidgetController::HandlePlayerJoinedToTeam);
        NLGS_Team->PlayerLeavedFromTeam.AddUObject(this, &UPlayersStatWidgetController::HandlePlayerLeavedFromTeam);
    }
}

void UPlayersStatWidgetController::BroadcastInitialValues()
{
    UWorld* World = GetWorld();
    for (TActorIterator<APlayerState> It(World); It; ++It)
    {
        APlayerState* PS = *It;
        ANLPlayerState* NLPS = Cast<ANLPlayerState>(PS);

        // 위젯 초기상태엔 기존에 존재하던 플레이어도 추가하기 위함
        OnPlayerJoinedToTeam.Broadcast(NLPS, NLPS->GetTeam());

        if (NLPS)
        {
            NLPS->BroadcastPlayerAllStats();
        }

        if (!BindedPlayers.Contains(PS))
        {
            if (NLPS)
            {
                NLPS->GetPlayerStatUpdatedDelegate().AddUObject(
                    this, &UPlayersStatWidgetController::HandlePlayerStatUpdate
                );
            }

            BindedPlayers.Add(PS);
        }
    }
}

void UPlayersStatWidgetController::HandlePlayerJoinedToTeam(const APlayerState* Player, int32 Team)
{
    if (!BindedPlayers.Contains(Player))
    {
        OnPlayerJoinedToTeam.Broadcast(Player, Team);

        if (ANLPlayerState* NLPS = const_cast<ANLPlayerState*>(Cast<ANLPlayerState>(Player)))
        {
            NLPS->GetPlayerStatUpdatedDelegate().AddUObject(
                this, &UPlayersStatWidgetController::HandlePlayerStatUpdate
            );
            NLPS->BroadcastPlayerAllStats();
        }

        BindedPlayers.Add(Player);
    }
}

void UPlayersStatWidgetController::HandlePlayerLeavedFromTeam(const APlayerState* Player, int32 Team)
{
    if (BindedPlayers.Contains(Player))
    {
        OnPlayerLeavedFromTeam.Broadcast(Player, Team);

        if (ANLPlayerState* NLPS = const_cast<ANLPlayerState*>(Cast<ANLPlayerState>(Player)))
        {
            NLPS->GetPlayerStatUpdatedDelegate().RemoveAll(this);
        }

        BindedPlayers.Remove(Player);
    }
}

void UPlayersStatWidgetController::HandlePlayerStatUpdate(const APlayerState* Player, const FGameplayTag& Tag, int32 Value)
{
    OnPlayerStatUpdated.Broadcast(Player, Tag, Value);
}
