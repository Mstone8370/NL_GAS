// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "NLWidgetComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnWidgetInitializedSignature)

/**
 * 
 */
UCLASS()
class NL_GAS_API UNLWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	virtual void InitWidget() override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool IsWidgetInitialized() const { return bIsWidgetInitialized; }

	FOnWidgetInitializedSignature OnWidgetInitialized;

protected:
	bool bIsWidgetInitialized = false;
};
