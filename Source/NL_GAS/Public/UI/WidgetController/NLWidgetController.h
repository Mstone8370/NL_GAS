// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NLWidgetController.generated.h"

class APlayerController;
class APlayerState;
class UAbilitySystemComponent;
class UAttributeSet;
class ANLPlayerController;
class ANLPlayerState;
class UNLAbilitySystemComponent;
class UNLAttributeSet;

/**
 * 
 */
UCLASS()
class NL_GAS_API UNLWidgetController : public UObject
{
	GENERATED_BODY()
	
public:
	void Initialize(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

	virtual void BindEvents();

	UFUNCTION(BlueprintCallable)
	virtual void BroadcastInitialValues();

	ANLPlayerController* GetNLPC();
	ANLPlayerState* GetNLPS();
	UNLAbilitySystemComponent* GetNLASC();
	UNLAttributeSet* GetNLAS();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<APlayerController> PlayerController;
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<APlayerState> PlayerState;
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<ANLPlayerController> NLPlayerController;
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<ANLPlayerState> NLPlayerState;
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UNLAbilitySystemComponent> NLAbilitySystemComponent;
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UNLAttributeSet> NLAttributeSet;
};
