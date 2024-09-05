// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

class AWeaponActor;
struct FTaggedAimPunch;
struct FGameplayTag;

USTRUCT(BlueprintType)
struct FDeathInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> SourceActor;

	TSharedPtr<FGameplayTag> DamageType;

	FDeathInfo()
	{
		bIsDead = false;
		SourceActor = nullptr;
		DamageType = nullptr;
	}

	FDeathInfo(bool bInIsDead) : FDeathInfo()
	{
		bIsDead = bInIsDead;
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
};

template<>
struct TStructOpsTypeTraits< FDeathInfo > : public TStructOpsTypeTraitsBase2< FDeathInfo >
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class NL_GAS_API ICombatInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void OnWeaponAdded(AWeaponActor* Weapon) = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanAttack();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ShowDamageText(float Value, bool bIsCriticalHit);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddAimPunch(const FTaggedAimPunch& AimPunchData, FVector HitDirection, bool bIsCriticalHit);

	virtual void OnDeath(const FDeathInfo& Info) = 0;

	virtual bool IsDead() const = 0;
};
