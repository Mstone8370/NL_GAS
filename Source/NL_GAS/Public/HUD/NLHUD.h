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

	UFUNCTION(BlueprintImplementableEvent)
	void OnNativeInitialized();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UNLUserWidget> OverlayWidgetClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNLUserWidget> OverlayWidget;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UOverlayWidgetController> OverlayWidgetController;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	void GetPlayerAimPoint(FVector& OutLocation, FRotator& OutRotation) const;

	UPROPERTY(BlueprintReadOnly)
	bool bNativeInitialized = false;

public:
	UOverlayWidgetController* GetOverlayWidgetController(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS, UNLCharacterComponent* NLC);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowDamageCauseIndicator(float Damage, bool bIsCriticalHit, AActor* DamagedActor);
};
