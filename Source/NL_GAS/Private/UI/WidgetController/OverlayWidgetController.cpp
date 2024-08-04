// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/OverlayWidgetController.h"

#include "Components/NLCharacterComponent.h"
#include "AbilitySystem/NLAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/NLAttributeSet.h"
#include "Player/NLPlayerController.h"

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
    GetNLPC()->OnPlayerDeath.AddLambda(
        [this](AActor* SourceActor, AActor* TargetActor, FGameplayTag DamageType)
        {
            PlayerDeath.Broadcast();
        }
    );
    GetNLPC()->OnReceivedKillLog.AddLambda(
        [this](AActor* SourceActor, AActor* TargetActor, FGameplayTag DamageType)
        {
            ReceivedKillLog.Broadcast(SourceActor, TargetActor, DamageType);
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
        [this](AActor* Interactable)
        {
            InteractionEnabled.Broadcast(Interactable);
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
}

void UOverlayWidgetController::BroadcastInitialValues()
{
    CurrentWeaponUpdated.Broadcast(
        NLCharacterComponent->GetCurrentWeaponTag(),
        NLCharacterComponent->GetCurrentWeaponSlot()
    );
    
    MaxHealthUpdated.Broadcast(GetNLAS()->GetMaxHealth());
    HealthUpdated.Broadcast(GetNLAS()->GetHealth());
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
