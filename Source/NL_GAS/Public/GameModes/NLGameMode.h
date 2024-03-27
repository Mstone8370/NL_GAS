// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "NLGameMode.generated.h"

class UWeaponInfo;

/**
 * 
 */
UCLASS()
class NL_GAS_API ANLGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWeaponInfo> WeaponInfo;
};
