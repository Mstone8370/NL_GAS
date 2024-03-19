// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NLCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class NL_GAS_API UNLCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	virtual bool CanAttemptJump() const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement (General Settings)")
	bool bFallingCrouchMaintainSightLocation = true;
};
