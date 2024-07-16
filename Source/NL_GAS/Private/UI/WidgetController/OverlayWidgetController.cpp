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
    GetNLPC()->OnReceivedKillLog.AddLambda(
        [this](FString SourceName, FString TargetName, FGameplayTag DamageType)
        {
            OnReceivedKillLog(SourceName, TargetName, DamageType);
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

void UOverlayWidgetController::OnReceivedKillLog(const FString SourceName, const FString TargetName, const FGameplayTag DamageType)
{
    ReceivedKillLog.Broadcast(SourceName, TargetName, DamageType);
}
