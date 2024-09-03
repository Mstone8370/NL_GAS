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
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameState.h"

ANLPlayerState::ANLPlayerState()
{
    AbilitySystemComponent = CreateDefaultSubobject<UNLAbilitySystemComponent>("AbilitySystemComponent");
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->SetIsReplicated(true);
        AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
    }

    AttributeSet = CreateDefaultSubobject<UNLAttributeSet>("AttributeSet");

    bUseCustomPlayerNames = true;

    // �ʴ� 10��
    NetUpdateFrequency = 10.f;
}

void ANLPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(ANLPlayerState, Team, COND_None, REPNOTIFY_OnChanged);
}

void ANLPlayerState::OnRep_Team()
{
    TeamAssigned(Team, true);
}

UAbilitySystemComponent* ANLPlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

FString ANLPlayerState::GetPlayerNameCustom() const
{
    return Super::GetPlayerNameCustom();
}

void ANLPlayerState::TeamAssigned(int32 NewTeam, bool bOnRepTeam)
{
    
    Team = NewTeam;
    
    if (!IsValid(GetPawn()))
    {
        return;
    }

    if (GetPawn()->GetLocalRole() == ROLE_SimulatedProxy)
    {
        // ���� �÷��̾��� �� ������ ������� �� ���� ����
        ANLCharacterBase* NLCharacter = Cast<ANLCharacterBase>(GetPawn());

        int32 LocalPlayerTeam = UNLFunctionLibrary::GetLocalPlayerTeam(this);
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
    else if (GetOwningController()->IsLocalPlayerController())
    {
        // Autonomous proxy or Authority

        // �ٸ� �ùķ���Ƽ�� ���Ͻ� �� ���� ������Ʈ
        if (AGameStateBase* GameState = GetWorld()->GetGameState())
        {
            for (APlayerState* PS : GameState->PlayerArray)
            {
                if (ANLPlayerState* NLPS = Cast<ANLPlayerState>(PS))
                {
                    if (NLPS != this && NLPS->GetTeam() != 0)
                    {
                        if (ANLCharacterBase* NLCharacter = Cast<ANLCharacterBase>(NLPS->GetPawn()))
                        {
                            if (NLPS->GetTeam() == Team)
                            {
                                NLCharacter->SetAsFriendly();
                            }
                            else
                            {
                                NLCharacter->SetAsEnemy();
                            }
                        }
                    }
                }
            }
        }
    }
}
