// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/OverlayWidgetController.h"

#include "Components/NLCharacterComponent.h"

void UOverlayWidgetController::BindEvents()
{
    NLCharacterComponent->WeaponSwapped.AddDynamic(this, &UOverlayWidgetController::OnWeaponSwapped);
    NLCharacterComponent->CurrentWeaponBulletNumChanged.AddDynamic(this, &UOverlayWidgetController::OnCurrentWeaponBulletNumChanged);
}

void UOverlayWidgetController::BroadcastInitialValues()
{
    WeaponChangedSignature.Broadcast(
        NLCharacterComponent->GetCurrentWeaponTag(),
        NLCharacterComponent->GetCurrentWeaponSlot()
    );
}

void UOverlayWidgetController::OnWeaponSwapped(const FGameplayTag& FromWeaponTag, int32 FromSlotNum, const FGameplayTag& ToWeaponTag, int32 ToSlotNum)
{
    WeaponChangedSignature.Broadcast(ToWeaponTag, ToSlotNum);
}

void UOverlayWidgetController::OnCurrentWeaponBulletNumChanged(int32 NewBulletNum)
{
    UpdateBulletNum.Broadcast(NewBulletNum);
}
