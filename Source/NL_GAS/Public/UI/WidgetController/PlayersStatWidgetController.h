// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/NLWidgetController.h"
#include "PlayersStatWidgetController.generated.h"

class APlayerState;
struct FGameplayTag;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerJoinedToTeamSignature, const APlayerState*, Player, int32, Team);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerLeavedFromTeamSignature, const APlayerState*, Player, int32, Team);

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class NL_GAS_API UPlayersStatWidgetController : public UNLWidgetController
{
	GENERATED_BODY()

public:
	virtual void BindEvents() override;

	virtual void BroadcastInitialValues() override;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerJoinedToTeamSignature OnPlayerJoinedToTeam;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerLeavedFromTeamSignature OnPlayerLeavedFromTeam;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerStatUpdatedSignature OnPlayerStatUpdated;

protected:
	UPROPERTY()
	TSet<const APlayerState*> BindedPlayers;

	UFUNCTION()
	void HandlePlayerJoinedToTeam(const APlayerState* Player, int32 Team);

	UFUNCTION()
	void HandlePlayerLeavedFromTeam(const APlayerState* Player, int32 Team);

	UFUNCTION()
	void HandlePlayerStatUpdate(const APlayerState* Player, const FGameplayTag& Tag, int32 Value);
};
