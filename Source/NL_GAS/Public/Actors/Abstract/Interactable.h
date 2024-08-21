// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Interactable.generated.h"

UCLASS(Abstract)
class NL_GAS_API AInteractable : public AActor
{
	GENERATED_BODY()
	
public:	
	AInteractable();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Category = "Interactable"))
	FGameplayTag InteractionType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bShouldHoldKeyPress;

	UPROPERTY(BlueprintReadOnly)
	bool bIsInteracting;

	virtual void OnStartInteraction(APawn* Interactor) {};

	virtual void OnEndInteraction() {};

public:
	UFUNCTION(BlueprintCallable)
	virtual void StartInteraction(APawn* Interactor);

	UFUNCTION(BlueprintCallable)
	virtual void EndInteraction();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual bool CanInteract() const { return true; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	const FGameplayTag& GetInteractionType() const { return InteractionType; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual bool ShouldHoldKeyPress() const { return bShouldHoldKeyPress; }
};
