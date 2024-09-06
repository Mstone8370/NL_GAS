// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/NLGameMode_TDM.h"

#include "GameStates/NLGameState_Team.h"
#include "GameStates/NLGameState_TDM.h"

void ANLGameMode_TDM::OnPlayerDied(AActor* SourceActor, AActor* TargetActor, FGameplayTag DamageType)
{
    Super::OnPlayerDied(SourceActor, TargetActor, DamageType);

    // TODO: 라운드 상태 확인해서 점수를 얻을 수 있는 상태인지 확인 먼저
    if (!GetNLGS_Team())
    {
        return;
    }

    APawn* SourcePawn = Cast<APawn>(SourceActor);
    APawn* TargetPawn = Cast<APawn>(TargetActor);
    if (SourcePawn && TargetPawn)
    {
        int32 SourceTeam = GetNLGS_Team()->FindTeam(SourcePawn->GetPlayerState());
        int32 TargetTeam = GetNLGS_Team()->FindTeam(TargetPawn->GetPlayerState());
        if (SourceTeam && TargetTeam && SourceTeam != TargetTeam)
        {
            GetNLGS_Team()->AddScore(SourceTeam);
            // TODO: TargetScore가 되면 매치 종료
        }
    }
}
