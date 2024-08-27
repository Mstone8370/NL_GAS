// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/NLGameMode_FiringRange.h"

AActor* ANLGameMode_FiringRange::ReclaimTargetActor()
{
    if (!TargetActorPool.IsEmpty())
    {
        AActor* Ret = TargetActorPool.Pop();
        Ret->SetActorHiddenInGame(false);
        PrintPoolStatus();
        return Ret;
    }
    return nullptr;
}

void ANLGameMode_FiringRange::ReturnTargetActor(AActor* TargetActor)
{
    if (TargetActorPool.Num() < PoolSize)
    {
        TargetActorPool.Add(TargetActor);
        TargetActor->SetActorHiddenInGame(true);
        PrintPoolStatus();
    }
    else
    {
        TargetActor->Destroy();
    }
}

void ANLGameMode_FiringRange::PrintPoolStatus() const
{
    const int32 CurrentPoolSize = TargetActorPool.Num();
    const int32 Empty = PoolSize - CurrentPoolSize;
    FString Log = "";
    for (int32 i = 0; i < CurrentPoolSize; i++)
    {
        Log += "X";
    }
    for (int32 i = 0; i < Empty; i++)
    {
        Log += "O";
    }
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Log);
}
