// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/NLHUD.h"

#include "Blueprint/UserWidget.h"
#include "UI/UserWidget/NLUserWidget.h"
#include "UI/WidgetController/NLWidgetController.h"
#include "UI/WidgetController/OverlayWidgetController.h"

void ANLHUD::Initialize(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS, UNLCharacterComponent* NLC)
{
    check(OverlayWidgetClass);
    check(OverlayWidgetControllerClass);

    OverlayWidget = CreateWidget<UNLUserWidget>(GetWorld(), OverlayWidgetClass);
    UOverlayWidgetController* WidgetController = GetOverlayWidgetController(PC, PS, ASC, AS, NLC);

    OverlayWidget->SetWidgetController(WidgetController);
    WidgetController->BroadcastInitialValues();

    OverlayWidget->AddToViewport();
}

UOverlayWidgetController* ANLHUD::GetOverlayWidgetController(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS, UNLCharacterComponent* NLC)
{
    if (!OverlayWidgetController)
    {
        OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
        FWidgetControllerParams Params(PC, PS, ASC, AS, NLC);
        OverlayWidgetController->Initialize(Params);
        OverlayWidgetController->BindEvents();
    }
    return OverlayWidgetController;
}
