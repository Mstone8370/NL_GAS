// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/OverlayWidgetController.h"

#include "Components/NLCharacterComponent.h"

void UOverlayWidgetController::BindEvents()
{
    NLCharacterComponent->WeaponSlotChanged.AddDynamic(this, &UOverlayWidgetController::OnWeaponSlotChanged);
    NLCharacterComponent->WeaponSwapped.AddDynamic(this, &UOverlayWidgetController::OnWeaponSwapped);
    NLCharacterComponent->CurrentWeaponBulletNumChanged.AddDynamic(this, &UOverlayWidgetController::OnCurrentWeaponBulletNumChanged);
}

void UOverlayWidgetController::BroadcastInitialValues()
{
    CurrentWeaponUpdated.Broadcast(
        NLCharacterComponent->GetCurrentWeaponTag(),
        NLCharacterComponent->GetCurrentWeaponSlot()
    );
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
