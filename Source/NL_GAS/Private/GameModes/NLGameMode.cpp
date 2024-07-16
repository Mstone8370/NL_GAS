// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/NLGameMode.h"

#include "Player/NLPlayerController.h"

void ANLGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (ANLPlayerController* NLPC = Cast<ANLPlayerController>(NewPlayer))
    {
        NLPC->PlayerDeathEvent.AddUObject(this, &ANLGameMode::OnPlayerDead);
    }
}

void ANLGameMode::OnPlayerDead(AController* SourceController, AController* TargetController, FGameplayTag DamageType)
{
    APawn* SourcePawn = nullptr;
    if (SourceController)
    {
        SourcePawn = SourceController->GetPawn();
    }
    APawn* TargetPawn = nullptr;
    if (SourceController)
    {
        TargetPawn = TargetController->GetPawn();
    }
    int32 PCNum = GetWorld()->GetNumPlayerControllers();
    for (int32 i = 0; i < PCNum; i++)
    {
        const FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator() + i;
        APlayerController* PC = It->Get();
        if (ANLPlayerController* NLPC = Cast<ANLPlayerController>(PC))
        {
            NLPC->AddKillLog(SourcePawn, TargetPawn, DamageType);
        }
    }
}
