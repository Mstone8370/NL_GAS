// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "NLPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class NL_GAS_API ANLPlayerStart : public APlayerStart
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInitial = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Team = 0;
};
