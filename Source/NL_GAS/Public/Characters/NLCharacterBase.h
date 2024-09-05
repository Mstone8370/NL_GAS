// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "Interface/CombatInterface.h"
#include "NLCharacterBase.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayAbility;
class UNLCharacterComponent;
class UGameplayEffect;
class UDamageTextWidgetComponent;
class UNLWidgetComponent;

UCLASS(Abstract)
class NL_GAS_API ANLCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	ANLCharacterBase(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnCharacterNameWidgetInitialized();

public:
	//~Begin AbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End AbilitySystemInterface

	//~Begin CombatInterface
	virtual void OnWeaponAdded(AWeaponActor* Weapon) override;
	virtual void ShowDamageText_Implementation(float Value, bool bIsCriticalHit) override;
	virtual void OnDeath(const FDeathInfo& Info) override;
	virtual bool IsDead() const { return DeathInfo.bIsDead; }
	//~End CombatInterface

	virtual void Destroyed() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DamageText")
	TSubclassOf<UDamageTextWidgetComponent> DamageTextWidgetComponentClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DamageText")
	FVector DamageTextOffset;

	UPROPERTY()
	TObjectPtr<UDamageTextWidgetComponent> LastDamageText;

	FORCEINLINE UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	virtual void EnableRagdoll();
	virtual void DisableRagdoll();

	virtual void OnRespawned();

	virtual void OnResetted();

protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNLCharacterComponent> NLCharacterComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNLWidgetComponent> CharacterNameWidgetComponent;

	virtual void InitAbilityActorInfo();

	void AddStartupAbilities();

	void InitDefaultAttribute();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DefaultAttribute;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DeathInfo)
	FDeathInfo DeathInfo;

	UFUNCTION()
	void OnRep_DeathInfo(FDeathInfo OldDeathInfo);

	virtual void HandleDeath(bool bSimulated = false);

	FTimerHandle DeathRagdollTimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DeathRagdollTime = 3.f;

	UFUNCTION()
	virtual void OnDeathRagdollTimeEnded();

	virtual void HandleRespawn(bool bSimulated = false);

	virtual void HandleReset(bool bSimulated = false);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TMap<FGameplayTag, TSubclassOf<UGameplayAbility>> StartupAbilities;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual void GetAimPoint(FVector& OutViewLocation, FRotator& OutViewRotation) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsWeaponSlotFull() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetAsEnemy();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetAsFriendly(bool bSelf = false);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetAsNeutral();
};
