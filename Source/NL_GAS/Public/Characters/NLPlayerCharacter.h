// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/NLCharacterBase.h"
#include "Interface/PlayerInterface.h"
#include "GameplayTagContainer.h"
#include "NLPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UNLCharacterMovementComponent;
class ANLPlayerController;
class AWeaponActor;
class UNLPlayerComponent;

/**
 * 
 */
UCLASS()
class NL_GAS_API ANLPlayerCharacter : public ANLCharacterBase, public IPlayerInterface
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
	virtual void OnRep_Controller() override;

	//~Begin PlayerInterface
	virtual void OnWeaponAdded(AWeaponActor* Weapon) override;
	virtual bool StartChangeWeaponSlot_Implementation(int32 NewSlot) override;
	//~End PlayerInterface

	//~Begin Crouch functions override
	virtual bool CanCrouch() const override;
	virtual void Crouch(bool bClientSimulation = false) override;
	/* Crouch�ؼ� ĸ���� ũ�Ⱑ �پ��� �� ȣ���. */
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	//~End Crouch functions override

	virtual void Landed(const FHitResult& Hit) override;
	void OnFallingStarted();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> ArmMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> ViewWeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNLPlayerComponent> NLPlayerComponent;

protected:
	TObjectPtr<UNLCharacterMovementComponent> NLCharacterMovementComponent;

	TObjectPtr<ANLPlayerController> NLPlayerController;

	UPROPERTY(EditDefaultsOnly)
	float LookPitchRepTime;

	UPROPERTY(BlueprintReadOnly, Replicated);
	float LookPitch;

	void Server_InvokeLookPitchReplication();

	FTimerHandle LookPitchRepTimerHandle;

	//~Begin Crouch Interpolation
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CrouchInterpSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CrouchInterpErrorTolerance;

	/* Replication condition - SimulatedOnly */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsCapsuleShrinked)
	bool bIsCapsuleShrinked = false;

	/**
	* ������������ bIsCapsuleShrinked ������ ���ø�����Ʈ �� ���.
	* �� ĳ���Ͱ� Ŭ���̾�Ʈ ������ �ƴϾ �ùķ��̼��� ��쿡�� ȣ���.
	* �� ĳ���͸� �����ϰ��ִ°����� ĸ���� ũ�Ⱑ ����Ǿ����� �ǹ��ϹǷ�,
	* �����ϴ� ���忡���� ũ�⸦ �����ؾ���.
	*/
	UFUNCTION()
	void OnRep_IsCapsuleShrinked();

	/**
	* ������ ĳ������ Ŭ���̾�Ʈ���� Crouch �Ǵ� UnCrouch�� �ؼ� ĸ�� ũ�⸦ ���������� ȣ���.
	* ������ bIsCapsuleShrinked ���� �����ؼ� �ٸ� Ŭ���̾�Ʈ���Ե� ���ø�����Ʈ �ǰ� ��.
	* @param bInShrinked - ĸ�� ũ�Ⱑ �پ�� ��� true.
	*/
	UFUNCTION(Server, Reliable)
	void Server_CapsuleShrinked(bool bInShrinked);

	bool bIsInterpolatingCrouch;
	float BaseSpringArmOffset;
	float TargetSpringArmOffset;

	bool IsCrouchInterpolatableCharacter();
	virtual void InterpolateCrouch(float DeltaSeconds);
	//~End Crouch Interpolation

public:
	void GetCrouchedHalfHeightAdjust(float& OutHalfHeightAdjust, float& OutScaledHalfHeightAdjust) const;

	FORCEINLINE float GetLookPitch() const { return LookPitch; }

	bool IsListenServerControlledCharacter();

	ANLPlayerController* GetNLPC();

	void OnCurrentWeaponChanged(const FGameplayTag& InWeaponTag);

	// ���� ����ִ� ���� ������ ������ Hidden ���¿� �޽��� �ִϸ��̼� ������Ʈ
	void UpdateCharacterMesh(AWeaponActor* OldWeaponActor = nullptr);
};
