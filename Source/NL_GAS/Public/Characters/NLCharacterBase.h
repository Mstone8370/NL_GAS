// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "NLCharacterBase.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class UGameplayAbility;

UCLASS(Abstract)
class NL_GAS_API ANLCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ANLCharacterBase();

protected:
	virtual void BeginPlay() override;

public:
	//~ Begin AbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End AbilitySystemInterface

	FORCEINLINE UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	void SetWeaponMesh(UStaticMesh* NewMesh);

protected:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	virtual void InitAbilityActorInfo();

	void AddStartupAbilities();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TMap<FGameplayTag, TSubclassOf<UGameplayAbility>> StartupAbilities;

};
