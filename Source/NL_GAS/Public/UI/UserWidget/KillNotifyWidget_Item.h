// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidget/NLUserWidget.h"
#include "KillNotifyWidget_Item.generated.h"

/**
 * 
 */
UCLASS()
class NL_GAS_API UKillNotifyWidget_Item : public UNLUserWidget
{
	GENERATED_BODY()
	
protected:
	FTimerHandle DisplayTimerHandle;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsDisplaying() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StartDisplay(const FString& Name, float DisplayTime);
};
