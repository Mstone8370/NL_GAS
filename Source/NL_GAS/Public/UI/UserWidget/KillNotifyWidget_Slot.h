// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidget/NLUserWidget.h"
#include "KillNotifyWidget_Slot.generated.h"

class UKillNotifyWidget_Item;

/**
 * 
 */
UCLASS()
class NL_GAS_API UKillNotifyWidget_Slot : public UNLUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnInitializedItems();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UKillNotifyWidget_Item> ItemWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 SlotNum = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ItemDisplayTime = 1.75f;

	UPROPERTY(BlueprintReadOnly)
	TArray<UKillNotifyWidget_Item*> Items;

public:
	UFUNCTION(BlueprintCallable)
	void DisplayItem(const FString& Name);
};
