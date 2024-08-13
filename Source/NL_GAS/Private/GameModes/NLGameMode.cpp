// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/NLGameMode.h"

#include "Player/NLPlayerController.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Actors/Volumes/RespawnArea.h"

void ANLGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (ANLPlayerController* NLPC = Cast<ANLPlayerController>(NewPlayer))
    {
        NLPC->OnPlayerDeath.AddUObject(this, &ANLGameMode::OnPlayerDead);
        NLPC->OnRequestRespawn.BindUObject(this, &ANLGameMode::RespawnPlayer);
    }
}

void ANLGameMode::OnPlayerDead(AActor* SourceActor, AActor* TargetActor, FGameplayTag DamageType)
{
    MulticastKillLog(SourceActor, TargetActor, DamageType);

    SetRespawnTime(TargetActor);
}

void ANLGameMode::SetRespawnTime(AActor* TargetActor)
{
    if (APawn* TargetPawn = Cast<APawn>(TargetActor))
    {
        if (ANLPlayerController* NLPC = Cast<ANLPlayerController>(TargetPawn->GetController()))
        {
            NLPC->SetRespawnTime(PlayerRespawnTime);
        }
    }
}

void ANLGameMode::MulticastKillLog(AActor* SourceActor, AActor* TargetActor, FGameplayTag DamageType)
{
    int32 PCNum = GetWorld()->GetNumPlayerControllers();
    for (int32 i = 0; i < PCNum; i++)
    {
        const FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator() + i;
        APlayerController* PC = It->Get();
        if (ANLPlayerController* NLPC = Cast<ANLPlayerController>(PC))
        {
            NLPC->AddKillLog(SourceActor, TargetActor, DamageType);
        }
    }
}

void ANLGameMode::RespawnPlayer(APlayerController* PC)
{
    float HalfHeight = 0.f;
    float Radius = 0.f;
    if (APawn* PCPawn = PC->GetPawn())
    {
        if (ACharacter* DefaultCharacter = Cast<ACharacter>(PCPawn->GetClass()->GetDefaultObject()))
        {
            DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleSize(Radius, HalfHeight);
        }
    }

    TArray<AActor*> RespawnAreas;
    UGameplayStatics::GetAllActorsOfClass(this, ARespawnArea::StaticClass(), RespawnAreas);

    // Shuffle Array from UKismetArrayLibrary::GenericArray_Shuffle
    int32 LastIndex = RespawnAreas.Num() - 1;
    for (int32 i = 0; i <= LastIndex; ++i)
    {
        int32 Index = FMath::RandRange(i, LastIndex);
        if (i != Index)
        {
            Swap(RespawnAreas[i], RespawnAreas[Index]);
        }
    }

    FVector RespawnLocation = FVector::ZeroVector;
    FVector RespawnDirection = FVector::XAxisVector;
    for (AActor* Actor : RespawnAreas)
    {
        if (ARespawnArea* RespawnArea = Cast<ARespawnArea>(Actor))
        {
            if (RespawnArea->GetRespawnableLocation(HalfHeight, Radius, RespawnLocation))
            {
                RespawnDirection = RespawnArea->GetDirection();
                break;
            }
        }
    }

    if (APawn* PCPawn = PC->GetPawn())
    {
        PCPawn->SetActorLocation(RespawnLocation);
    }
    if (ANLPlayerController* NLPC = Cast<ANLPlayerController>(PC))
    {
        NLPC->OnRespawned(RespawnDirection);
    }
}
