// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/NLWidgetController.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "Player/NLPlayerController.h"
#include "Player/NLPlayerState.h"
#include "AbilitySystem/NLAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/NLAttributeSet.h"

void UNLWidgetController::Initialize(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
    PlayerController = PC;
    PlayerState = PS;
    AbilitySystemComponent = ASC;
    AttributeSet = AS;
}

void UNLWidgetController::BindEvents() {}

void UNLWidgetController::BroadcastInitialValues() {}

ANLPlayerController* UNLWidgetController::GetNLPC()
{
    if (!NLPlayerController)
    {
        NLPlayerController = Cast<ANLPlayerController>(PlayerController);
    }
    return NLPlayerController;
}

ANLPlayerState* UNLWidgetController::GetNLPS()
{
    if (!NLPlayerState)
    {
        NLPlayerState = Cast<ANLPlayerState>(PlayerState);
    }
    return NLPlayerState;
}

UNLAbilitySystemComponent* UNLWidgetController::GetNLASC()
{
    if (!NLAbilitySystemComponent)
    {
        NLAbilitySystemComponent = Cast<UNLAbilitySystemComponent>(AbilitySystemComponent);
    }
    return NLAbilitySystemComponent;
}

UNLAttributeSet* UNLWidgetController::GetNLAS()
{
    if (!NLAttributeSet)
    {
        NLAttributeSet = Cast<UNLAttributeSet>(AttributeSet);
    }
    return NLAttributeSet;
}
