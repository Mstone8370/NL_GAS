// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeathCam.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class NL_GAS_API ADeathCam : public AActor
{
	GENERATED_BODY()
	
public:
	ADeathCam();

	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY()
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY()
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

	void Tracking(float DeltaTime);

public:
	UFUNCTION(BlueprintCallable)
	void SetTargetActor(AActor* InTargetActor);

	float InterpSpeed = 10.f;
};
