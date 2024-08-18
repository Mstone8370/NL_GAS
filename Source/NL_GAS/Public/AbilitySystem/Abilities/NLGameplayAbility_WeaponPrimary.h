// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/NLGameplayAbility_Damage.h"
#include "NLGameplayAbility_WeaponPrimary.generated.h"

UENUM(BlueprintType)
enum EFireType : uint8
{
	Single,
	Burst,
	Automatic
};

UCLASS()
class NL_GAS_API UNLGameplayAbility_WeaponPrimary : public UNLGameplayAbility_Damage
{
	GENERATED_BODY()

protected:
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (Categories = "Particle"))
	FGameplayTag ParticleTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float FirePerSecond = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TEnumAsByte<EFireType> FireType;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Burst", meta = (EditCondition = "FireType == EFireType::Burst", ClampMin = "1"))
	int32 BurstFireNum = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Burst", meta = (EditCondition = "FireType == EFireType::Burst"))
	float BurstFirePerSecond = 10;
};
