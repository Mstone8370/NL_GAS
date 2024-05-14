// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "NLAttributeSet.generated.h"

/**
 * 
 */

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

USTRUCT()
struct FEffectContextParams
{
	GENERATED_BODY()

	FEffectContextParams() {}

	FGameplayEffectContextHandle ContextHandle;

	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;

	UPROPERTY()
	AActor* SourceAvatarActor = nullptr;

	UPROPERTY()
	AController* SourceController = nullptr;

	UPROPERTY()
	APlayerController* SourcePC = nullptr;

	UPROPERTY()
	UAbilitySystemComponent* TargetASC = nullptr;

	UPROPERTY()
	AActor* TargetAvatarActor = nullptr;

	UPROPERTY()
	AController* TargetController = nullptr;

	UPROPERTY()
	APlayerController* TargetPC = nullptr;
};

UCLASS()
class NL_GAS_API UNLAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// Default Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Default Attributes", ReplicatedUsing = OnRep_MaxHealth);
	FGameplayAttributeData MaxHealth;

	ATTRIBUTE_ACCESSORS(UNLAttributeSet, MaxHealth);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldHealth) const;

	UPROPERTY(BlueprintReadOnly, Category = "Default Attributes", ReplicatedUsing = OnRep_Health);
	FGameplayAttributeData Health;

	ATTRIBUTE_ACCESSORS(UNLAttributeSet, Health);

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;

	// Meta Attributes
	UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes");
	FGameplayAttributeData IncomingDamage;

	ATTRIBUTE_ACCESSORS(UNLAttributeSet, IncomingDamage);

protected:
	void SetEffectContextParams(const FGameplayEffectModCallbackData& Data, FEffectContextParams& OutParams) const;
};
