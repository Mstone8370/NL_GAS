// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NLCharacterMovementComponent.h"

#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

bool UNLCharacterMovementComponent::CanAttemptJump() const
{
	return IsJumpAllowed() &&
		// !bWantsToCrouch &&
		(IsMovingOnGround() || IsFalling()); // Falling included for double-jump and non-zero jump hold time, but validated by character.
}
