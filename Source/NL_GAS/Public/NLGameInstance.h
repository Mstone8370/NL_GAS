// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "NLGameInstance.generated.h"

class UTaggedWeaponInfoList;

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
};
