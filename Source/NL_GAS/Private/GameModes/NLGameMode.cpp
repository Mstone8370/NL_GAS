// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/NLGameMode.h"

#include "Player/NLPlayerController.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Actors/Volumes/RespawnArea.h"
#include "Actors/Singletons/ParticleReplicationManager.h"
#include "Actors/Singletons/ProjectileReplicationManager.h"

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

/**
* TODO: 이 함수는 플레이어가 처음 접속했을때 사용되지만, 이 게임의 리스폰을 구현하는데에는 적절하지 않음.
* RestartPlayer 같은 함수들은 컨트롤러가 이미 폰을 가지고있는 경우에는 PlayerStart의 회전만 적용하기 때문임.
* (그마저도 액터의 로테이션을 변경하므로, control rotation을 변경해야하는 이 게임의 캐릭터에는 적용이 안됨.)
* RestartPlayer 함수들을 오버라이드해서 수정해도 되지만, 이미 잘 작동하는 구현된 리스폰 함수가 있으니
* 게임 진행중의 리스폰은 그 함수를 사용하고, 처음 접속할때에는 아래의 함수를 적절하게 사용해서 처리해야함.
*/
AActor* ANLGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    UE_LOG(LogTemp, Warning, TEXT("ChoosePlayerStart override"));
    return Super::ChoosePlayerStart_Implementation(Player);
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

    FindAllRespawnAreas();
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
    float HalfHeight = 0.f;
    float Radius = 0.f;
    if (APawn* PCPawn = PC->GetPawn())
    {
        if (ACharacter* DefaultCharacter = Cast<ACharacter>(PCPawn->GetClass()->GetDefaultObject()))
        {
            DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleSize(Radius, HalfHeight);
        }
    }

    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsOfClass(this, ARespawnArea::StaticClass(), Actors);

    // Shuffle Array from UKismetArrayLibrary::GenericArray_Shuffle
    int32 LastIndex = Actors.Num() - 1;
    for (int32 i = 0; i <= LastIndex; ++i)
    {
        int32 Index = FMath::RandRange(i, LastIndex);
        if (i != Index)
        {
            Swap(Actors[i], Actors[Index]);
        }
    }

    FVector RespawnLocation = FVector::ZeroVector;
    FVector RespawnDirection = FVector::XAxisVector;
    for (AActor* Actor : Actors)
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

void ANLGameMode::FindAllRespawnAreas()
{
    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsOfClass(this, ARespawnArea::StaticClass(), Actors);

    RespawnAreas.Empty();
    for (int32 i = 0; i < RespawnAreas.Num(); i++)
    {
        if (ARespawnArea* RA = Cast<ARespawnArea>(Actors[i]))
        {
            RespawnAreas.Add(RA);
        }
    }
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
