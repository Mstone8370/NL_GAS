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

protected:
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	// 클라이언트에서 받은 Flag를 통해 상태 업데이트
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
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