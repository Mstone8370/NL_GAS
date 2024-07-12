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

UCLASS(Abstract)
class NL_GAS_API ANLCharacterBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	ANLCharacterBase(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

public:
	//~Begin AbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~End AbilitySystemInterface

	//~Begin CombatInterface
	virtual void OnWeaponAdded(AWeaponActor* Weapon) override;
	virtual void ShowDamageText_Implementation(float Value, bool bIsCriticalHit) override;
	virtual void OnDead() override;
	//~End CombatInterface

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DamageText")
	TSubclassOf<UDamageTextWidgetComponent> DamageTextWidgetComponentClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "DamageText")
	FVector DamageTextOffset;

	UPROPERTY()
	TObjectPtr<UDamageTextWidgetComponent> LastDamageText;

	FORCEINLINE UAttributeSet* GetAttributeSet() const { return AttributeSet; }

protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNLCharacterComponent> NLCharacterComponent;

	virtual void InitAbilityActorInfo();

	void AddStartupAbilities();

	void InitDefaultAttribute();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DefaultAttribute;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsDead)
	bool bIsDead = false;

	UFUNCTION()
	void OnRep_IsDead();

	virtual void OnDead_Internal(bool bSimulated = false);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TMap<FGameplayTag, TSubclassOf<UGameplayAbility>> StartupAbilities;

public:
	bool IsDead() const { return bIsDead; }
};
