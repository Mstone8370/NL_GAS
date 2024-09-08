// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "NLPlayerState.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnPlayerStatUpdatedSignature, const APlayerState* /*PlayerState*/, const FGameplayTag& /*StatTag*/, int32 /*Value*/);

/**
 * 
 */
UCLASS()
class NL_GAS_API ANLPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	ANLPlayerState();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FGameplayTag> StartupWeapons;

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	FString PlayerName;

	FOnPlayerStatUpdatedSignature OnPlayerStatUpdated;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Team)
	int32 Team;

	UFUNCTION()
	void OnRep_Team();

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsDead)
	bool bIsDead;

	UFUNCTION()
	void OnRep_IsDead();

public:
	//~ Begin AbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End AbilitySystemInterface

	FORCEINLINE UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	virtual FString GetPlayerNameCustom() const override;

	// Player Stats
	UFUNCTION(BlueprintCallable, Category = "PlayerStats")
	virtual void SetPlayerStat(FGameplayTag StatTag, int32 Value) {};

	UFUNCTION(BlueprintCallable, Category = "PlayerStats")
	virtual void AddPlayerStat(FGameplayTag StatTag, int32 ValueAdded = 1) {};

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PlayerStats")
	virtual int32 GetPlayerStat(FGameplayTag StatTag) const { return 0; }

	UFUNCTION(BlueprintCallable, Category = "PlayerStats")
	virtual void BroadcastPlayerAllStats() const {};

	FOnPlayerStatUpdatedSignature& GetPlayerStatUpdatedDelegate() { return OnPlayerStatUpdated; }
	// Player Stats

	virtual void TeamAssigned(int32 NewTeam);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetTeam() const { return Team; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsDead() const { return bIsDead; }

	void HandleDeath(AActor* SourceActor);

	UFUNCTION(BlueprintCallable, Category = "PlayerStats")
	virtual void ResetPlayerStats() {}
};
