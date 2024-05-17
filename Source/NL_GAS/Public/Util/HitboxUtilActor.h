// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HitboxUtilActor.generated.h"

class UHitboxComponent;

UCLASS()
class NL_GAS_API AHitboxUtilActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHitboxUtilActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UHitboxComponent> HitboxComp;
};
