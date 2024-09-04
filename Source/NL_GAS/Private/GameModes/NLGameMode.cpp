// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/NLGameMode.h"

#include "Player/NLPlayerController.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"

#include "Actors/Singletons/ParticleReplicationManager.h"
#include "Actors/Singletons/ProjectileReplicationManager.h"
#include "Actors/NLPlayerStart.h"

void ANLGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (ANLPlayerController* NLPC = Cast<ANLPlayerController>(NewPlayer))
    {
        NLPC->OnPlayerDeath.AddUObject(this, &ANLGameMode::OnPlayerDead);
        NLPC->OnRequestRespawn.BindUObject(this, &ANLGameMode::RespawnPlayer);
    }
}

void ANLGameMode::Logout(AController* Exiting)
{
    if (ANLPlayerController* NLPC = Cast<ANLPlayerController>(Exiting))
    {
        NLPC->OnPlayerDeath.RemoveAll(this);
        NLPC->OnRequestRespawn.Unbind();
    }

    Super::Logout(Exiting);
}

void ANLGameMode::BeginPlay()
{
    Super::BeginPlay();

    AActor* ParticleRepManager = nullptr;
    SpawnOrGetSingleton(ParticleRepManager, AParticleReplicationManager::StaticClass());
    ParticleReplicationManager = Cast<AParticleReplicationManager>(ParticleRepManager);

    AActor* ProjRepManager = nullptr;
    SpawnOrGetSingleton(ProjRepManager, AProjectileReplicationManager::StaticClass());
    ProjectileReplicationManager = Cast<AProjectileReplicationManager>(ProjRepManager);
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
            NLPC->SetRespawnTime(MinRespawnDelay);
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
    AActor* Start = ChoosePlayerStartByCondition(PC, false, false);
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

void ANLGameMode::ResetPlayer(APlayerController* PC)
{
}

AActor* ANLGameMode::ChoosePlayerStartByCondition(APlayerController* Player, bool bInitial, bool bCheckTeam)
{
    // Codes from AGameModeBase::ChoosePlayerStart_Implementation
    
    APlayerStart* FoundPlayerStart = nullptr;

    UClass* PawnClass = GetDefaultPawnClassForController(Player);
    APawn* PawnToFit = PawnClass ? PawnClass->GetDefaultObject<APawn>() : nullptr;

    TArray<APlayerStart*> UnOccupiedStartPoints;
    TArray<APlayerStart*> OccupiedStartPoints;

    UWorld* World = GetWorld();
    for (TActorIterator<APlayerStart> It(World); It; ++It)
    {
        APlayerStart* PlayerStart = *It;
        if (!CheckPlayerStartCondition(PlayerStart, Player, bInitial, bCheckTeam))
        {
            continue;
        }
        
        FVector ActorLocation = PlayerStart->GetActorLocation();
        const FRotator ActorRotation = PlayerStart->GetActorRotation();
        if (!World->EncroachingBlockingGeometry(PawnToFit, ActorLocation, ActorRotation))
        {
            UnOccupiedStartPoints.Add(PlayerStart);
        }
        else if (World->FindTeleportSpot(PawnToFit, ActorLocation, ActorRotation))
        {
            OccupiedStartPoints.Add(PlayerStart);
        }
    }

    if (FoundPlayerStart == nullptr)
    {
        if (UnOccupiedStartPoints.Num() > 0)
        {
            FoundPlayerStart = UnOccupiedStartPoints[FMath::RandRange(0, UnOccupiedStartPoints.Num() - 1)];
        }
        else if (OccupiedStartPoints.Num() > 0)
        {
            FoundPlayerStart = OccupiedStartPoints[FMath::RandRange(0, OccupiedStartPoints.Num() - 1)];
        }
    }

    return FoundPlayerStart;
}

bool ANLGameMode::CheckPlayerStartCondition(APlayerStart* PlayerStart, APlayerController* Player, bool bInitial, bool bCheckTeam)
{
    if (!IsValid(PlayerStart))
    {
        return false;
    }

    if (bInitial)
    {
        if (ANLPlayerStart* NLPlayerStart = Cast<ANLPlayerStart>(PlayerStart))
        {
            if (bInitial && !NLPlayerStart->bInitial)
            {
                return false;
            }
        }
    }
    return true;
}

void ANLGameMode::SpawnOrGetSingleton(AActor*& OutActor, TSubclassOf<AActor> ActorClass)
{
    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsOfClass(this, ActorClass, Actors);
    if (!Actors.IsEmpty())
    {
        OutActor = Actors[0];
        for (int32 i = 1; i < Actors.Num(); i++)
        {
            Actors[i]->Destroy();
        }
    }
    if (!OutActor)
    {
        OutActor = GetWorld()->SpawnActor(ActorClass);
    }
}
