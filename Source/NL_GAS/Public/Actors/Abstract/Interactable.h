// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Interactable.generated.h"

class USphereComponent;

UCLASS(Abstract)
class NL_GAS_API AInteractable : public AActor
{
	GENERATED_BODY()
	
public:	
	AInteractable();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> RootMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> SphereCollision;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Category = "Interactable"))
	FGameplayTag InteractionType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bShouldHoldKeyPress;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsInteracting)
	bool bIsInteracting;

	UFUNCTION()
	virtual void OnInteractorEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnInteractorExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	virtual void OnRep_IsInteracting() {};

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
