// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RespawnArea.generated.h"

class UBoxComponent;
class UArrowComponent;

/**
 * 
 */
UCLASS(Blueprintable)
class NL_GAS_API ARespawnArea : public AActor
{
	GENERATED_BODY()

public:
	ARespawnArea();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UBoxComponent> BoxComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UArrowComponent> ArrowComponent;
	
public:
	bool GetRespawnableLocation(float CapsuleHalfHeight, float CapsuleRadius, FVector& OutLocation);

	FVector GetDirection() const;
};
