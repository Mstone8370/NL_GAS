// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "NLPlayerController.generated.h"

class UInputConfig;
class UInputMappingContext;
class UNLAbilitySystemComponent;
class ANLPlayerState;
struct FInputActionValue;

/**
 * 
 */
UCLASS()
class NL_GAS_API ANLPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void SetupInputComponent() override;

	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Jump();
	void Crouch();
	void UnCrouch();
	void CrouchToggle();

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);

public:
	UNLAbilitySystemComponent* GetNLAbilitySystemComponent();
	ANLPlayerState* GetNLPlayerState();
	FORCEINLINE bool IsListenServerController() const { return bIsListenServerController; }

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TArray<TSoftObjectPtr<UInputMappingContext>> StartupIMC;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TSoftObjectPtr<UInputConfig> InputConfig;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	float LookSensitivity = 1.f;

	TObjectPtr<UNLAbilitySystemComponent> LNAbilitySystemComponent;
	TObjectPtr<ANLPlayerState> NLPlayerState;

	bool bIsListenServerController = false;

	float CurrentLookSensitivity;

	UFUNCTION(Client, Reliable)
	void Client_ShowDamageCauseIndicator(float InDamage, bool bIsCriticalHit, AActor* DamagedActor);

	UFUNCTION(Client, Reliable)
	void Client_TakenDamage(FVector HitDirection, float AimpunchMagnitude);

public:
	float GetBaseLookSensitivity() const { return LookSensitivity; }

	void SetLookSensitivity(float InLookSensitivity);

	void OnCausedDamage(float InDamage, bool bInIsCriticalHit, AActor* DamagedActor);

	void OnTakenDamage(const FHitResult* InHitResult, float AimpunchMagnitude);
};
