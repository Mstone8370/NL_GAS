// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Abstract/Interactable.h"

#include "Net/UnrealNetwork.h"

AInteractable::AInteractable()
    : InteractionType(FGameplayTag::EmptyTag)
    , bShouldHoldKeyPress(false)
    , bIsInteracting(false)
{
 	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
}

void AInteractable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(AInteractable, bIsInteracting, COND_None, REPNOTIFY_OnChanged);
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
