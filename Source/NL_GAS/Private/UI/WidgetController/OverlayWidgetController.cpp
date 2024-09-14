// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/OverlayWidgetController.h"

#include "Components/NLCharacterComponent.h"
#include "AbilitySystem/NLAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/NLAttributeSet.h"
#include "Player/NLPlayerController.h"
#include "Player/NLPlayerState.h"
#include "NLFunctionLibrary.h"
#include "GameStates/NLGameState_Team.h"

void UOverlayWidgetController::BindEvents()
{
    NLCharacterComponent->WeaponSlotChanged.AddDynamic(this, &UOverlayWidgetController::OnWeaponSlotChanged);
    NLCharacterComponent->WeaponSwapped.AddDynamic(this, &UOverlayWidgetController::OnWeaponSwapped);
    NLCharacterComponent->CurrentWeaponBulletNumChanged.AddDynamic(this, &UOverlayWidgetController::OnCurrentWeaponBulletNumChanged);

    GetNLASC()->GetGameplayAttributeValueChangeDelegate(GetNLAS()->GetMaxHealthAttribute()).AddLambda(
        [this](const FOnAttributeChangeData& Data)
        {
            MaxHealthUpdated.Broadcast(Data.NewValue);
        }
    );
    GetNLASC()->GetGameplayAttributeValueChangeDelegate(GetNLAS()->GetHealthAttribute()).AddLambda(
        [this](const FOnAttributeChangeData& Data)
        {
            HealthUpdated.Broadcast(Data.NewValue);
        }
    );

    GetNLPC()->OnTakenDamageDelegate.AddLambda(
        [this](FVector HitDirection)
        {
            OnTakenDamage(HitDirection);
        }
    );
    GetNLPC()->OnPlayerDeathDelegate.AddLambda(
        [this](AActor* SourceActor, AActor* TargetActor, FGameplayTag DamageType)
        {
            PlayerDeath.Broadcast();
        }
    );
    GetNLPC()->OnReceivedKillLog.AddLambda(
        [this](APlayerState* SourcePS, APlayerState* TargetPS, FGameplayTag DamageType)
        {
            ReceivedKillLog.Broadcast(SourcePS, TargetPS, DamageType);
        }
    );
    GetNLPC()->OnRespawnable.AddLambda(
        [this]()
        {
            Respawnable.Broadcast();
        }
    );
    GetNLPC()->OnKill.AddLambda(
        [this](AActor* TargetActor)
        {
            Killed.Broadcast(TargetActor);
        }
    );
    GetNLPC()->OnPlayerRespawn.AddLambda(
        [this]()
        {
            PlayerRespawn.Broadcast();
        }
    );
    GetNLPC()->OnInteractionEnabled.AddLambda(
        [this](AActor* Interactable, FString Message)
        {
            InteractionEnabled.Broadcast(Interactable, Message);
        }
    );
    GetNLPC()->OnInteractionDisabled.AddLambda(
        [this]()
        {
            InteractionDisabled.Broadcast();
        }
    );
    GetNLPC()->OnInteractionBegin.AddLambda(
        [this]()
        {
            InteractionBegin.Broadcast();
        }
    );
    GetNLPC()->OnInteractionEnd.AddLambda(
        [this]()
        {
            InteractionEnd.Broadcast();
        }
    );

    GetNLPS()->GetPlayerStatUpdatedDelegate().AddLambda(
        [this](const APlayerState* PS, const FGameplayTag& Tag, int32 Value)
        {
            OnPlayerStatUpdated.Broadcast(PS, Tag, Value);
        }
    );

    ANLGameState_Team* NLGS_Team = Cast<ANLGameState_Team>(GetWorld()->GetGameState());
    if (NLGS_Team)
    {
        NLGS_Team->RoundWinTeamDecided.BindLambda(
            [this](int32 WinTeam)
            {
                int32 LocalPlayerTeam = UNLFunctionLibrary::GetLocalPlayerTeam(this);
                OnRoundWinTeamDecided.Broadcast(WinTeam == LocalPlayerTeam);
            }
        );
        NLGS_Team->MatchWinTeamDecided.BindLambda(
            [this](int32 WinTeam)
            {
                int32 LocalPlayerTeam = UNLFunctionLibrary::GetLocalPlayerTeam(this);
                OnMatchWinTeamDecided.Broadcast(WinTeam == LocalPlayerTeam);
            }
        );
        NLGS_Team->RoundIntroBegin.BindLambda(
            [this](int32 RoundIntroTime)
            {
                OnRoundIntroBegin.Broadcast(RoundIntroTime);
            }
        );
        NLGS_Team->RoundInProgress.BindLambda(
            [this](int32 RoundTimeLimit)
            {
                OnRoundInProgress.Broadcast(RoundTimeLimit);
            }
        );
        NLGS_Team->TeamScoreUpdated.BindLambda(
            [this](FTeamScoreInfo TeamScoreInfo)
            {
                int32 LocalPlayerTeam = UNLFunctionLibrary::GetLocalPlayerTeam(this);
                int32 FriendlyTeamScore = 0;
                int32 EnemyTeamScore = 0;
                if (LocalPlayerTeam == 1)
                {
                    FriendlyTeamScore = TeamScoreInfo.Team_1;
                    EnemyTeamScore = TeamScoreInfo.Team_2;
                }
                else if (LocalPlayerTeam == 2)
                {
                    FriendlyTeamScore = TeamScoreInfo.Team_2;
                    EnemyTeamScore = TeamScoreInfo.Team_1;
                }
                OnTeamScoreUpdated.Broadcast(FriendlyTeamScore, EnemyTeamScore);
            }
        );
        NLGS_Team->TargetScoreUpdated.BindLambda(
            [this](int32 TargetScore)
            {
                OnTargetScoreUpdated.Broadcast(TargetScore);
            }
        );
    }
}

void UOverlayWidgetController::BroadcastInitialValues()
{
    CurrentWeaponUpdated.Broadcast(
        NLCharacterComponent->GetCurrentWeaponTag(),
        NLCharacterComponent->GetCurrentWeaponSlot()
    );
    
    MaxHealthUpdated.Broadcast(GetNLAS()->GetMaxHealth());
    HealthUpdated.Broadcast(GetNLAS()->GetHealth());

    if (GetNLPS())
    {
        GetNLPS()->BroadcastPlayerAllStats();
    }

    ANLGameState_Team* NLGS_Team = Cast<ANLGameState_Team>(GetWorld()->GetGameState());
    if (NLGS_Team)
    {
        FTeamScoreInfo TeamScoreInfo = NLGS_Team->GetScore();
        int32 LocalPlayerTeam = UNLFunctionLibrary::GetLocalPlayerTeam(this);
        int32 FriendlyTeamScore = 0;
        int32 EnemyTeamScore = 0;
        if (LocalPlayerTeam == 1)
        {
            FriendlyTeamScore = TeamScoreInfo.Team_1;
            EnemyTeamScore = TeamScoreInfo.Team_2;
        }
        else if (LocalPlayerTeam == 2)
        {
            FriendlyTeamScore = TeamScoreInfo.Team_2;
            EnemyTeamScore = TeamScoreInfo.Team_1;
        }
        OnTeamScoreUpdated.Broadcast(TeamScoreInfo.Team_1, TeamScoreInfo.Team_2);

        OnTargetScoreUpdated.Broadcast(NLGS_Team->GetTargetScore());
    }
}

void UOverlayWidgetController::OnWeaponSlotChanged(const TArray<FGameplayTag>& WeaponTagSlot)
{
    WeaponSlotUpdated.Broadcast(WeaponTagSlot);
}

void UOverlayWidgetController::OnWeaponSwapped(const FGameplayTag& FromWeaponTag, int32 FromSlotNum, const FGameplayTag& ToWeaponTag, int32 ToSlotNum)
{
    CurrentWeaponUpdated.Broadcast(ToWeaponTag, ToSlotNum);
}

void UOverlayWidgetController::OnCurrentWeaponBulletNumChanged(int32 NewBulletNum)
{
    BulletNumUpdated.Broadcast(NewBulletNum);
}

void UOverlayWidgetController::OnTakenDamage(FVector DamageOrigin)
{
    DamageTaken.Broadcast(DamageOrigin);
}
