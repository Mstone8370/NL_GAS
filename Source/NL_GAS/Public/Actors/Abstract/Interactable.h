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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Interaction"))
	FGameplayTag InteractionType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bShouldHoldKeyPress;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bHighlight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0, ClampMax = 256, UIMin = 0, UIMax = 256))
	float HighlightStencilValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = 0, ClampMax = 256, UIMin = 0, UIMax = 256))
	float HighlightStencilValue_Focused;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsInteracting)
	bool bIsInteracting;

	UFUNCTION()
	virtual void OnInteractorEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnInteractorExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	virtual void OnRep_IsInteracting();

	UFUNCTION(BlueprintNativeEvent)
	void OnStartInteraction(APawn* Interactor);

	UFUNCTION(BlueprintNativeEvent)
	void OnEndInteraction();

	UFUNCTION(BlueprintNativeEvent)
	void OnFocused();

	UFUNCTION(BlueprintNativeEvent)
	void OnUnfocused();

public:
	UFUNCTION(BlueprintCallable)
	virtual void StartInteraction(APawn* Interactor);

	UFUNCTION(BlueprintCallable)
	virtual void EndInteraction();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual bool CanInteract() const { return !bIsInteracting; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	const FGameplayTag& GetInteractionType() const { return InteractionType; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual bool ShouldHoldKeyPress() const { return bShouldHoldKeyPress; }

	UFUNCTION(BlueprintCallable)
	void EnableHighlight();
	
	UFUNCTION(BlueprintCallable)
	void DisableHighlight();

	UFUNCTION(BlueprintCallable)
	void Focused();

	UFUNCTION(BlueprintCallable)
	void Unfocused();
};
