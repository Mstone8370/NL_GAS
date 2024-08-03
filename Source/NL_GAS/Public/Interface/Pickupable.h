// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Pickupable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPickupable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class NL_GAS_API IPickupable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanPickedUp();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnPickedUp();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnDropped();
};
