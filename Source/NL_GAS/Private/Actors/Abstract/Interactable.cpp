// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Abstract/Interactable.h"

AInteractable::AInteractable()
    : InteractionType(FGameplayTag::EmptyTag)
    , bShouldHoldKeyPress(false)
    , bIsInteracting(false)
{
 	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

void AInteractable::StartInteraction(APawn* Interactor)
{
    if (!CanInteract() && bIsInteracting)
    {
        return;
    }

    OnStartInteraction(Interactor);
}

void AInteractable::EndInteraction()
{
    if (!bIsInteracting)
    {
        return;
    }

    OnEndInteraction();
}
