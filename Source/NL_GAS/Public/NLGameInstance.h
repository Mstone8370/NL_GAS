// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "NLGameInstance.generated.h"

class UTaggedWeaponInfoList;
class UParticleData;
class UProjectileData;

/**
 * 
 */
UCLASS()
class NL_GAS_API UNLGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UTaggedWeaponInfoList> TaggedWeaponInfoList;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UParticleData> ParticleData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UProjectileData> ProjectileData;
	
};
