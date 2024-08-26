// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Abstract/Interactable.h"

#include "Net/UnrealNetwork.h"
#include "Components/SphereComponent.h"
#include "Interface/PlayerInterface.h"

AInteractable::AInteractable()
    : InteractionType(FGameplayTag::EmptyTag)
    , bShouldHoldKeyPress(false)
    , bHighlight(true)
    , HighlightStencilValue(256)
    , bIsInteracting(false)
{
 	PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("Root Mesh"));
    RootMesh->SetCustomDepthStencilValue(HighlightStencilValue);
    SetRootComponent(RootMesh);

    SphereCollision = CreateDefaultSubobject<USphereComponent>(FName("SphereCollision"));
    SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereCollision->SetGenerateOverlapEvents(true);
    SphereCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    SphereCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    SphereCollision->SetSphereRadius(200.f, false);
    SphereCollision->SetupAttachment(RootMesh);
}

void AInteractable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(AInteractable, bIsInteracting, COND_None, REPNOTIFY_OnChanged);
}

void AInteractable::BeginPlay()
{
    Super::BeginPlay();

    SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AInteractable::OnInteractorEnter);
    SphereCollision->OnComponentEndOverlap.AddDynamic(this, &AInteractable::OnInteractorExit);
}

void AInteractable::OnInteractorEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!CanInteract())
    {
        return;
    }

    IPlayerInterface::Execute_OnInteractableRangeEnter(OtherActor);
}

void AInteractable::OnInteractorExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (!CanInteract())
    {
        return;
    }

    IPlayerInterface::Execute_OnInteractableRangeExit(OtherActor);
}

void AInteractable::OnRep_IsInteracting()
{
    if (bIsInteracting)
    {
        OnStartInteraction(nullptr);
    }
    else
    {
        OnEndInteraction();
    }
}

void AInteractable::OnStartInteraction_Implementation(APawn* Interactor) {}

void AInteractable::OnEndInteraction_Implementation() {}

void AInteractable::StartInteraction(APawn* Interactor)
{
    if (!CanInteract())
    {
        return;
    }

    bIsInteracting = true;

    OnStartInteraction(Interactor);
}

void AInteractable::EndInteraction()
{
    if (!bIsInteracting)
    {
        return;
    }

    bIsInteracting = false;

    OnEndInteraction();
}

void AInteractable::StartHighlight()
{
    if (bHighlight && RootMesh)
    {
        RootMesh->SetRenderCustomDepth(true);
    }
}

void AInteractable::EndHighlight()
{
    if (bHighlight && RootMesh)
    {
        RootMesh->SetRenderCustomDepth(false);
    }
}
