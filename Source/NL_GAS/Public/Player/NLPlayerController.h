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
class AInteractable;
struct FInputActionValue;
struct FParticleSpawnInfo;
struct FProjectileSpawnInfo;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTakenDamageSignature, FVector);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnReceivedKillLogSignature, AActor* /*SourceActor*/, AActor* /*TargetActor*/, FGameplayTag /*DamageType*/);
DECLARE_MULTICAST_DELEGATE(FOnRespawnableSignature);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnKillSignature, AActor* /*TargetActor*/);
DECLARE_DELEGATE_OneParam(FOnRequestRespawn, APlayerController* /*PC*/);
DECLARE_EVENT_ThreeParams(ANLPlayerController, FOnPlayerDeathSignature, AActor* /*SourceActor*/, AActor* /*TargetActor*/, FGameplayTag /*DamageType*/);
DECLARE_EVENT(ANLPlayerController, FOnPlayerRespawnSignature);
DECLARE_EVENT_TwoParams(ANLPlayerController, FOnInteractionEnabledSignature, AActor* /*Interactable*/, FString /*Message*/);
DECLARE_EVENT(ANLPlayerController, FOnInteractionDisabledSignature);
DECLARE_MULTICAST_DELEGATE(FOnInteractionBeginSignature);
DECLARE_MULTICAST_DELEGATE(FOnInteractionEndSignature);

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
	void Interaction();
	void OnInteractionHoldTriggered();
	void BeginInteraction();
	void EndInteraction();

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);

	void Respawn();

public:
	UNLAbilitySystemComponent* GetNLASC();
	ANLPlayerState* GetNLPS();
	ANLPlayerCharacter* GetNLPlayerCharacter();
	ANLHUD* GetNLHUD();

	void InitHUD(APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS, UNLCharacterComponent* NLC);

	FORCEINLINE bool IsListenServerController() const { return bIsListenServerController; }

	FOnTakenDamageSignature OnTakenDamageDelegate;

	FOnReceivedKillLogSignature OnReceivedKillLog;

	FOnRespawnableSignature OnRespawnable;

	FOnKillSignature OnKill;

	FOnRequestRespawn OnRequestRespawn;

	FOnPlayerDeathSignature OnPlayerDeathDelegate;

	FOnPlayerRespawnSignature OnPlayerRespawn;

	FOnInteractionEnabledSignature OnInteractionEnabled;

	FOnInteractionDisabledSignature OnInteractionDisabled;

	FOnInteractionBeginSignature OnInteractionBegin;

	FOnInteractionEndSignature OnInteractionEnd;

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

	UFUNCTION(Server, Reliable)
	void Server_Interaction(AInteractable* Interactable);

	UFUNCTION(Client, Reliable)
	void Client_ShowDamageCauseIndicator(float InDamage, bool bIsCriticalHit, AActor* DamagedActor);

	UFUNCTION(Client, Reliable)
	void Client_TakenDamage(FVector DamageOrigin, FVector HitDirection, bool bIsCriticalHit, FGameplayTag DamageType);

	FTimerHandle RespawnTimerHandle;

	virtual void OnRespawnableState();

	UFUNCTION(Client, Reliable)
	void Client_OnRespawnableState();

	UFUNCTION(Client, Reliable)
	void Client_OnKilled(AActor* TargetActor);

	UFUNCTION(Server, Reliable)
	void Server_RespawnRequested(APlayerController* PC);

	UFUNCTION(Client, Reliable)
	void Client_OnRespawned();

	UPROPERTY()
	TObjectPtr<AInteractable> InteractableActor;

	bool bIsInteracting = false;

	void SetupDeathCam(AActor* TargetActor);

	void ClearDeathCam();

	UFUNCTION(Client, Unreliable)
	void Client_SpawnParticles(const FGameplayTag& ParticleTag, const TArray<FParticleSpawnInfo>& SpawnInfos);

	UFUNCTION(Client, Unreliable)
	void Client_SpawnProjectiles(const FGameplayTag& ProjectileTag, const TArray<FProjectileSpawnInfo>& SpawnInfos);

public:
	float GetBaseLookSensitivity() const { return LookSensitivity; }

	void SetLookSensitivity(float InLookSensitivity);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	void GetPlayerAimPoint(FVector& OutViewLocation, FRotator& OutViewRotation) const;

	UFUNCTION(BlueprintCallable)
	void OnCausedDamage(float InDamage, bool bInIsCriticalHit, AActor* DamagedActor);

	void OnTakenDamage(const FHitResult* InHitResult, FVector DamageOrigin, bool bIsCriticalHit, const FGameplayTag& DamageType);

	void OnKilled(AActor* TargetActor);

	void OnPlayerDeath(AActor* SourceActor, FGameplayTag DamageType);

	UFUNCTION(Client, Unreliable)
	void AddKillLog(AActor* SourceActor, AActor* TargetActor, FGameplayTag DamageType);

	void SetRespawnTime(float RespawnTime);

	void OnRespawned(FVector Direction);

	void OnResetted(FVector Direction);

	void EnableInteraction(AInteractable* Interactable, FString Message);

	void DisableInteraction();

	void ReplicateParticlesToClient(const FGameplayTag& ParticleTag, const TArray<FParticleSpawnInfo>& SpawnInfos);

	void ReplicateProjectilesToClient(const FGameplayTag& ProjectileTag, const TArray<FProjectileSpawnInfo>& SpawnInfos);
};
