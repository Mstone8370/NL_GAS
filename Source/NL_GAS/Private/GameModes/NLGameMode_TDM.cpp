// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/NLGameMode_TDM.h"

#include "GameStates/NLGameState_Team.h"
#include "GameStates/NLGameState_TDM.h"

void ANLGameMode_TDM::OnPlayerDied(AActor* SourceActor, AActor* TargetActor, FGameplayTag DamageType)
{
    Super::OnPlayerDied(SourceActor, TargetActor, DamageType);

    if (HasRoundStarted())
    {
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
                
                // TODO: ���� ���� Ȯ���ϴ� �Լ� ������ �����
                if (GetNLGS_Team()->GetScoreTeam(SourceTeam) == TargetScore)
                {
                    EndRound(SourceTeam, TargetScore);
                }
            }
        }
    }
}