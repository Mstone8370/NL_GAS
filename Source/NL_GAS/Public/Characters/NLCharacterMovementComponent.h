// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NLCharacterMovementComponent.generated.h"

DECLARE_DELEGATE(FFallingStartedSignature);

/**
 * 
 */
UCLASS()
class NL_GAS_API UNLCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	virtual bool CanAttemptJump() const override;

	virtual void Crouch(bool bClientSimulation = false) override;

	FFallingStartedSignature FallingStarted;

	/**
	* Crouch Interpolation이 끝났을 때 캡슐의 크기를 줄이는 함수.
	* @param bClientSimulation - 레플리케이트되어 소유하지 않은 캐릭터인 경우 true.
	*/
	UFUNCTION()
	void ShrinkCapsuleHeight(bool bClientSimulation = false);

protected:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
};
