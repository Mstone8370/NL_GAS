// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/NLHUD.h"

#include "Player/NLPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "UI/UserWidget/NLUserWidget.h"
#include "UI/WidgetController/NLWidgetController.h"
#include "UI/WidgetController/OverlayWidgetController.h"

void ANLHUD::Initialize(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS, UNLCharacterComponent* NLC)
{
    check(OverlayWidgetClass);
    check(OverlayWidgetControllerClass);

    if (bNativeInitialized)
    {
        return;
    }
    
    OverlayWidget = CreateWidget<UNLUserWidget>(GetWorld(), OverlayWidgetClass);
    UOverlayWidgetController* WidgetController = GetOverlayWidgetController(PC, PS, ASC, AS, NLC);

    OverlayWidget->SetWidgetController(WidgetController);
    WidgetController->BroadcastInitialValues();

    OverlayWidget->AddToViewport();

    OnNativeInitialized();

    bNativeInitialized = true;
}

void ANLHUD::BeginPlay()
{
    Super::BeginPlay();

    if (bNativeInitialized)
    {
        // BeginPlay 이전에 Initialize 함수가 호출될수도 있음.
        OnNativeInitialized();
    }
}

void ANLHUD::GetPlayerAimPoint(FVector& OutLocation, FRotator& OutRotation) const
{
    OutLocation = FVector::ZeroVector;
    OutRotation = FRotator::ZeroRotator;
    if (APlayerController* PC = GetOwningPlayerController())
    {
        if (ANLPlayerController* NLPC = Cast<ANLPlayerController>(PC))
        {
            NLPC->GetPlayerAimPoint(OutLocation, OutRotation);
        }
        else
        {
            FRotator ViewRotator;
            GetOwningPlayerController()->GetPlayerViewPoint(OutLocation, ViewRotator);
            OutRotation = GetOwningPlayerController()->GetControlRotation();
        }
    }
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
