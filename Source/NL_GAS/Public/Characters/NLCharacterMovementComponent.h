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
	* Crouch Interpolation이 끝났을 때 캡슐의 크기를 줄이는 함수.
	* @param bClientSimulation - 레플리케이트되어 소유하지 않은 캐릭터인 경우 true.
	*/
	UFUNCTION()
	void ShrinkCapsuleHeight(bool bClientSimulation = false);

	virtual float GetMaxSpeed() const override;

	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;

	// 커스텀 PredictionData를 리턴하도록 수정함
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

	// 클라이언트에서 받은 Flag를 통해 상태 업데이트
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	// 이번 프레임에 땅에 닿았는지를 확인하기 위한 변수. 슬라이드할때 사용됨.
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
* 새로운 상태를 추가한 커스텀 SavedMove 클래스
* 이걸 기반으로 한 정보가 서버로 전달됨.
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
* 클라이언트에서 사용되는 Prediction 데이터
* 여기에서 SavedMove를 관리함
*/
class FNetworkPredictionData_Client_NLCharacter : public FNetworkPredictionData_Client_Character
{
public:
	typedef FNetworkPredictionData_Client_Character Super;

	FNetworkPredictionData_Client_NLCharacter(const UCharacterMovementComponent& ClientMovement);

	virtual FSavedMovePtr AllocateNewMove() override;
};