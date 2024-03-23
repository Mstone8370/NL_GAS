// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/NLCharacterBase.h"
#include "NLPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UNLCharacterMovementComponent;

/**
 * 
 */
UCLASS()
class NL_GAS_API ANLPlayerCharacter : public ANLCharacterBase
{
	GENERATED_BODY()
	
public:
	ANLPlayerCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	virtual bool CanJumpInternal_Implementation() const;

	virtual void InitAbilityActorInfo() override;

public:
	virtual void Tick(float DeltaSeconds) override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	// Crouch functions override;
	virtual bool CanCrouch() const override;
	virtual void Crouch(bool bClientSimulation = false) override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> ArmMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> CameraComponent;

	TObjectPtr<UNLCharacterMovementComponent> NLCharacterMovementComponent;

	//~Begin Crouch Interpolation
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CrouchInterpSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CrouchInterpErrorTolerance;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsCapsuleShrinked)
	bool bIsCapsuleShrinked = false;

	UFUNCTION()
	void OnRep_IsCapsuleShrinked();

	UFUNCTION(Server, Reliable)
	void Server_CapsuleShrinked(bool bInShrinked);

	bool bIsInterpolatingCrouch;
	float BaseSpringArmOffset;
	float TargetSpringArmOffset;

	virtual void InterpolateCrouch(float DeltaSeconds);
	//~End Crouch Interpolation

public:

};
