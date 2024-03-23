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
	* Crouch Interpolation�� ������ �� ĸ���� ũ�⸦ ���̴� �Լ�.
	* @param bClientSimulation - ���ø�����Ʈ�Ǿ� �������� ���� ĳ������ ��� true.
	*/
	UFUNCTION()
	void ShrinkCapsuleHeight(bool bClientSimulation = false);

protected:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
};
