// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/NLHUD.h"

#include "Blueprint/UserWidget.h"
#include "UI/UserWidget/NLUserWidget.h"
#include "UI/WidgetController/OverlayWidgetController.h"

void ANLHUD::Initialize(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
    check(OverlayWidgetClass);
    check(OverlayWidgetControllerClass);

    OverlayWidget = CreateWidget<UNLUserWidget>(GetWorld(), OverlayWidgetClass);
    UNLWidgetController* WidgetController = GetOverlayWidgetController(PC, PS, ASC, AS);
    OverlayWidget->SetWidgetController(WidgetController);
    WidgetController->BroadcastInitialValues();
}

UOverlayWidgetController* ANLHUD::GetOverlayWidgetController(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
    if (!OverlayWidgetController)
    {
        OverlayWidgetController = NewObject<UOverlayWidgetController>();
        OverlayWidgetController->Initialize(PC, PS, ASC, AS);
        OverlayWidgetController->BindEvents();
    }
    return OverlayWidgetController;
}
