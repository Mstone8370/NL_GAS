// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "NLHUD.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UNLUserWidget;
class UOverlayWidgetController;
class UNLCharacterComponent;

/**
 * 
 */
UCLASS()
class NL_GAS_API ANLHUD : public AHUD
{
	GENERATED_BODY()

public:
	void Initialize(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS, UNLCharacterComponent* NLC);

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UNLUserWidget> OverlayWidgetClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNLUserWidget> OverlayWidget;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UOverlayWidgetController> OverlayWidgetController;

public:
	UOverlayWidgetController* GetOverlayWidgetController(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS, UNLCharacterComponent* NLC);
};
