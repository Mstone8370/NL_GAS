// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NLPlayerState.h"

#include "AbilitySystem/NLAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/NLAttributeSet.h"
#include "NLFunctionLibrary.h"
#include "Data/WeaponInfo.h"
#include "Net/UnrealNetwork.h"
#include "Characters/NLCharacterBase.h"
#include "Characters/NLPlayerCharacter.h"
#include "NLGameplayTags.h"
#include "Actors/WeaponActor.h"
#include "GameFramework/GameState.h"
#include "GameStates/NLGameState_Team.h"
#include "Engine/World.h"

ANLPlayerState::ANLPlayerState()
    : PlayerName(TEXT("Player"))
    , Team(0)
    , bIsDead(false)
{
    AbilitySystemComponent = CreateDefaultSubobject<UNLAbilitySystemComponent>("AbilitySystemComponent");
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->SetIsReplicated(true);
        AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
    }

    AttributeSet = CreateDefaultSubobject<UNLAttributeSet>("AttributeSet");

    bUseCustomPlayerNames = true;

    // 초당 10번
    NetUpdateFrequency = 10.f;
}

void ANLPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(ANLPlayerState, Team, COND_None, REPNOTIFY_OnChanged);
    DOREPLIFETIME_CONDITION_NOTIFY(ANLPlayerState, bIsDead, COND_None, REPNOTIFY_OnChanged);
}

void ANLPlayerState::OnRep_Team()
{
    TeamAssigned(Team);

    UWorld* World = GetWorld();
    if (ANLGameState_Team* NLGS_Team = World->GetGameState<ANLGameState_Team>())
    {
        NLGS_Team->AddPlayerStateToTeam(this, Team);
    }
}

void ANLPlayerState::OnRep_IsDead()
{
    
}

UAbilitySystemComponent* ANLPlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

FString ANLPlayerState::GetPlayerNameCustom() const
{
    return Super::GetPlayerNameCustom();
}

void ANLPlayerState::TeamAssigned(int32 NewTeam)
{
    Team = NewTeam;
    
    if (!IsValid(GetPawn()))
    {
        return;
    }
    
    ANLCharacterBase* NLCharacter = Cast<ANLCharacterBase>(GetPawn());

    if (GetPawn()->GetLocalRole() == ROLE_SimulatedProxy)
    {
        // 로컬 플레이어의 팀 정보를 기반으로 폰 외형 설정
        int32 LocalPlayerTeam = UNLFunctionLibrary::GetLocalPlayerTeam(this);
        if (LocalPlayerTeam >= 0)
        {
            if (NewTeam == LocalPlayerTeam)
            {
                if (NLCharacter)
                {
                    NLCharacter->SetAsFriendly();
                }
            }
            else
            {
                if (NLCharacter)
                {
                    NLCharacter->SetAsEnemy();
                }
            }
        }
    }
    else if (GetOwningController()->IsLocalPlayerController())
    {
        // Autonomous proxy or Authority

        // 다른 시뮬레이티드 프록시 폰 외형 업데이트
        UNLFunctionLibrary::ApplyTeamAppearanceToOtherPlayers(this, NewTeam);

        if (NLCharacter)
        {
            NLCharacter->SetAsFriendly(true);
        }
    }
}
