// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NLCharacterMovementComponent.generated.h"

UENUM(BlueprintType)
enum ENLMovementMode : uint8
{
	/** None (movement is disabled). */
	NLMOVE_None				UMETA(DisplayName = "None"),

	/** Walking on a surface. */
	NLMOVE_LedgeClimbing	UMETA(DisplayName = "LedgeClimbing"),

	NLMOVE_MAX				UMETA(Hidden),
};

DECLARE_DELEGATE(FFallingStartedSignature);

/**
 * 
 */
UCLASS()
class NL_GAS_API UNLCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	virtual bool CanAttemptJump() const override;

	virtual void Crouch(bool bClientSimulation = false) override;

	virtual void UnCrouch(bool bClientSimulation = false) override;

	virtual bool DoJump(bool bReplayingMoves) override;

	FFallingStartedSignature FallingStarted;

	/**
	* Crouch Interpolation�� ������ �� ĸ���� ũ�⸦ ���̴� �Լ�.
	* @param bClientSimulation - ���ø�����Ʈ�Ǿ� �������� ���� ĳ������ ��� true.
	*/
	UFUNCTION()
	void ShrinkCapsuleHeight(bool bClientSimulation = false);

	virtual float GetMaxSpeed() const override;

	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;

	// Ŀ���� PredictionData�� �����ϵ��� ������
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float MaxSprintSpeed = 600.f;

	bool bWantsToSprint = false;

	bool IsSprinting() const;

	virtual bool CanSprintInCurrentState() const;

	virtual void Sprint(bool bClientSimulation);
	virtual void StopSprint(bool bClientSimulation);


	bool IsSliding() const;

	virtual bool CanSlideInCurrentState() const;

	virtual void Slide(bool bClientSimulation);
	virtual void StopSlide(bool bClientSimulation);

	virtual bool CanApplySlideBoost() const;

	void ApplySlideBoost();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float SlideGroundFriction = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float SlideBrakingDecelerationWalking = 512.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float SlideMaxAcceleration = 256.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bSlideBoost = true;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float SlideBoostForce = 300.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float SlideBoostCooltime = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float LedgeTraceLength = 40.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float LedgeTraceBottomHalfHeight = 35.f;

	bool IsLedgeClimbing() const;

protected:
	// DefaultValues
	float DefaultGroundFriction;
	float DefaultBrakingDecelerationWalking;
	float DefaultMaxAcceleration;

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	// Ŭ���̾�Ʈ���� ���� Flag�� ���� ���� ������Ʈ
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	// �̹� �����ӿ� ���� ��Ҵ����� Ȯ���ϱ� ���� ����. �����̵��Ҷ� ����.
	bool bWasFalling;

	bool bSlideBoostReady = true;

	FTimerHandle SlideBoostCooltimeTimer;

	void OnSlideBoostCooltimeFinished();

	bool CheckLedgeDetectionCondition() const;

	bool FindBlockingLedge(FHitResult& OutHitResult, bool bDebug = false);

	bool CanStandUpOnLegde(FHitResult& OutHitResult, bool bDebug = false);

	bool GetLedgeClimbTargetLocation(const FHitResult& BlockingHitResult, const FHitResult& StandUpHitResult);

	FVector LedgeClimbTargetLocation;

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	void PhysLedgeClimbing(float deltaTime, int32 Iterations);

	void GetCapsuleScaledSize(float& OutHalfHeight, float& OutRadius) const;
};


/** 
* ���ο� ���¸� �߰��� Ŀ���� SavedMove Ŭ����
* �̰� ������� �� ������ ������ ���޵�.
*/
class FSavedMove_NLCharacter : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;

	virtual void Clear() override;

	virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;

	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;

	virtual void PrepMoveFor(ACharacter* C) override;

	uint8 GetCompressedFlags() const override;

	uint32 bWantsToSprint : 1;
};


/**
* Ŭ���̾�Ʈ���� ���Ǵ� Prediction ������
* ���⿡�� SavedMove�� ������
*/
class FNetworkPredictionData_Client_NLCharacter : public FNetworkPredictionData_Client_Character
{
public:
	typedef FNetworkPredictionData_Client_Character Super;

	FNetworkPredictionData_Client_NLCharacter(const UCharacterMovementComponent& ClientMovement);

	virtual FSavedMovePtr AllocateNewMove() override;
};