// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/NLWidgetController.h"
#include "GameplayTagContainer.h"
#include "OverlayWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponSlotUpdatedSignature, const TArray<FGameplayTag>&, WeaponSlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCurrentWeaponUpdatedSignature, const FGameplayTag&, WeaponTag, const int32, SlotNum);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBulletNumUpdatedSignature, const int32, BulletNum);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttributeUpdatedSignature, float, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDirectionalDamageTakenSignature, FVector, DamageOrigin);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerDeathSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FReceivedKillLogSignature, const AActor*, SourceActor, const AActor*, TargetActor, const FGameplayTag, DamageType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRespawnableSingature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKilled, AActor*, TargetActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerRespawnSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInteractionEnabledSignature, AActor*, Interactable, FString, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInteractionDisabledSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInteractionBeginSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInteractionEndSignature);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeamScoreUpdatedSignature, int32, FriendlyTeam, int32, EnemyTeam);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetScoreUpdatedSignature, int32, TargetScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundWinTeamDecidedSignature, int32, WinTeam);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchWinTeamDecidedSignature, int32, WinTeam);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundIntroBeginSignature, int32, RoundIntroTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoundInProgressSignature, int32, RoundTimeLimit);

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class NL_GAS_API UOverlayWidgetController : public UNLWidgetController
{
	GENERATED_BODY()

public:
	virtual void BindEvents() override;

	virtual void BroadcastInitialValues() override;

	UPROPERTY(BlueprintAssignable)
	FWeaponSlotUpdatedSignature WeaponSlotUpdated;

	UPROPERTY(BlueprintAssignable)
	FCurrentWeaponUpdatedSignature CurrentWeaponUpdated;

	UPROPERTY(BlueprintAssignable)
	FBulletNumUpdatedSignature BulletNumUpdated;

	UPROPERTY(BlueprintAssignable)
	FAttributeUpdatedSignature HealthUpdated;

	UPROPERTY(BlueprintAssignable)
	FAttributeUpdatedSignature MaxHealthUpdated;

	UPROPERTY(BlueprintAssignable)
	FDirectionalDamageTakenSignature DamageTaken;

	UPROPERTY(BlueprintAssignable)
	FPlayerDeathSignature PlayerDeath;

	UPROPERTY(BlueprintAssignable)
	FReceivedKillLogSignature ReceivedKillLog;

	UPROPERTY(BlueprintAssignable)
	FRespawnableSingature Respawnable;

	UPROPERTY(BlueprintAssignable)
	FKilled Killed;

	UPROPERTY(BlueprintAssignable)
	FPlayerRespawnSignature PlayerRespawn;

	UPROPERTY(BlueprintAssignable)
	FInteractionEnabledSignature InteractionEnabled;
	UPROPERTY(BlueprintAssignable)
	FInteractionDisabledSignature InteractionDisabled;
	UPROPERTY(BlueprintAssignable)
	FInteractionBeginSignature InteractionBegin;
	UPROPERTY(BlueprintAssignable)
	FInteractionEndSignature InteractionEnd;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerStatUpdatedSignature OnPlayerStatUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnRoundWinTeamDecidedSignature OnRoundWinTeamDecided;

	UPROPERTY(BlueprintAssignable)
	FOnMatchWinTeamDecidedSignature OnMatchWinTeamDecided;

	UPROPERTY(BlueprintAssignable)
	FOnRoundIntroBeginSignature OnRoundIntroBegin;

	UPROPERTY(BlueprintAssignable)
	FOnRoundInProgressSignature OnRoundInProgress;

	UPROPERTY(BlueprintAssignable)
	FOnTeamScoreUpdatedSignature OnTeamScoreUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnTargetScoreUpdatedSignature OnTargetScoreUpdated;

protected:
	UFUNCTION()
	void OnWeaponSlotChanged(const TArray<FGameplayTag>& WeaponTagSlot);

	UFUNCTION()
	void OnWeaponSwapped(const FGameplayTag& FromWeaponTag, int32 FromSlotNum, const FGameplayTag& ToWeaponTag, int32 ToSlotNum);
	
	UFUNCTION()
	void OnCurrentWeaponBulletNumChanged(int32 NewBulletNum);

	UFUNCTION()
	void OnTakenDamage(FVector DamageOrigin);
};
