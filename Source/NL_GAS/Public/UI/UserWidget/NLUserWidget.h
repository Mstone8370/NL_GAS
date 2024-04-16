// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NLUserWidget.generated.h"

class UNLWidgetController;

/**
 * 
 */
UCLASS()
class NL_GAS_API UNLUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNLWidgetController> WidgetController;

	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UNLWidgetController* InWidgetController);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnWidgetControllerSet();
};
