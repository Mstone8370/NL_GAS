// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "NLPlayerController.generated.h"

class UInputConfig;
class UInputMappingContext;
class UNLAbilitySystemComponent;
class UAbilitySystemComponent;
class UAttributeSet;
class ANLPlayerState;
class ANLPlayerCharacter;
class UNLCharacterComponent;
class UAimPunchData;
class ANLHUD;
class UEnhancedInputLocalPlayerSubsystem;
struct FInputActionValue;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTakenDamageSignature, FVector);
DECLARE_EVENT_ThreeParams(ANLPlayerController, FPlayerDeathSignature, AController*, AController*, FGameplayTag);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnReceivedKillLogSignature, FString, FString, FGameplayTag);

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

	void AddIMC(TArray<TSoftObjectPtr<UInputMappingContext>>& IMCs);
	void AddIMC(TSoftObjectPtr<UInputMappingContext> IMC, UEnhancedInputLocalPlayerSubsystem* InputSystem = nullptr);

	void RemoveIMC(TArray<TSoftObjectPtr<UInputMappingContext>>& IMCs);
	void RemoveIMC(TSoftObjectPtr<UInputMappingContext> IMC, UEnhancedInputLocalPlayerSubsystem* InputSystem = nullptr);

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Jump();
	void Crouch();
	void UnCrouch();
	void CrouchToggle();

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);

	void Respawn();

public:
	UNLAbilitySystemComponent* GetNLAbilitySystemComponent();
	ANLPlayerState* GetNLPlayerState();
	ANLPlayerCharacter* GetNLPlayerCharacter();
	ANLHUD* GetNLHUD();

	void InitHUD(APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS, UNLCharacterComponent* NLC);

	FORCEINLINE bool IsListenServerController() const { return bIsListenServerController; }

	FOnTakenDamageSignature OnTakenDamageDelegate;

	FPlayerDeathSignature PlayerDeathEvent;

	FOnReceivedKillLogSignature OnReceivedKillLog;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TArray<TSoftObjectPtr<UInputMappingContext>> DefaultIMC;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> DeathIMC;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TSoftObjectPtr<UInputConfig> InputConfig;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UAimPunchData> AimPunchData;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	float LookSensitivity = 1.f;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UNLAbilitySystemComponent> LNAbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ANLPlayerState> NLPlayerState;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ANLPlayerCharacter> NLPlayerCharacter;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ANLHUD> NLHUD;

	bool bIsListenServerController = false;

	float CurrentLookSensitivity;

	FVector MoveInputDirection;

	UFUNCTION(Client, Reliable)
	void Client_ShowDamageCauseIndicator(float InDamage, bool bIsCriticalHit, AActor* DamagedActor);

	UFUNCTION(Client, Reliable)
	void Client_TakenDamage(FVector DamageOrigin, FVector HitDirection, bool bIsCriticalHit, FGameplayTag DamageType);

public:
	float GetBaseLookSensitivity() const { return LookSensitivity; }

	void SetLookSensitivity(float InLookSensitivity);

	void OnCausedDamage(float InDamage, bool bInIsCriticalHit, AActor* DamagedActor);

	void OnTakenDamage(const FHitResult* InHitResult, FVector DamageOrigin, bool bIsCriticalHit, const FGameplayTag& DamageType);

	void OnDead(AController* SourceController, FGameplayTag DamageType);

	UFUNCTION(Client, Unreliable)
	void AddKillLog(APawn* SourcePawn, APawn* TargetPawn, FGameplayTag DamageType);
};
