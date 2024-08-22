// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Abstract/Interactable.h"
#include "ButtonActor.generated.h"

/**
 * 
 */
UCLASS()
class NL_GAS_API AButtonActor : public AInteractable
{
	GENERATED_BODY()
	
protected:
	virtual void OnStartInteraction(APawn* Interactor) override;

	virtual void OnEndInteraction() override;
};
