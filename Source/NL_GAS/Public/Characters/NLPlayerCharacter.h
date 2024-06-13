// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/NLCharacterBase.h"
#include "Interface/PlayerInterface.h"
#include "GameplayTagContainer.h"
#include "NLPlayerCharacter.generated.h"

class USpringArmComponent;
class UNLPlayerCameraComponent;
class UNLCharacterMovementComponent;
class ANLPlayerController;
class UAnimMontage;
class UControlShakeManager;
class UMaterialInstanceDynamic;
class UNLViewSkeletalMeshComponent;

UCLASS()
class NL_GAS_API ANLPlayerCharacter : public ANLCharacterBase, public IPlayerInterface
{
	GENERATED_BODY()
	
public:
	ANLPlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents();

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
	virtual bool CanSwapWeaponSlot_Implementation(int32 NewSlot) override;
	virtual void TrySwapWeaponSlot_Implementation(int32 NewSlot) override;
	virtual void GetWeaponHandIKLocation_Implementation(FName LeftIKSocketName, FName RightIKSocketName, FVector& OutLeftIKLocation, FVector& OutRightIKLocation) const;
	virtual float PlayCurrentWeaponMontage_Implementation(const FGameplayTag& MontageTag) override;
	virtual void WeaponFired_Implementation(TSubclassOf<UCameraShakeBase> CameraShakeBaseClass) override;
	virtual bool StartReload_Implementation() override;
	virtual void OnWeaponReloadStateChanged_Implementation(const FGameplayTag& WeaponTag, const FGameplayTag& StateTag) override;
	virtual float GetWeaponSpreadValue_Implementation() override;
	virtual bool CommitWeaponCost_Implementation(bool& bIsLast) override;
	//~End PlayerInterface

	//~Begin CombatInterface
	virtual bool CanAttack_Implementation() override;
	virtual void AddAimPunch_Implementation(const FTaggedAimPunch& AimPunchData, FVector HitDirection, bool bIsCriticalHit) override;
	//~End CombatInterface

	//~Begin Crouch functions override
	virtual bool CanCrouch() const override;
	virtual void Crouch(bool bClientSimulation = false) override;
	/* Crouch�ؼ� ĸ���� ũ�Ⱑ �پ��� �� ȣ���. */
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	//~End Crouch functions override

	virtual void Landed(const FHitResult& Hit) override;
	void OnFallingStarted();

	virtual bool CanSprint();
	// Request to sprint
	virtual void Sprint();
	// Request to stop sprinting
	virtual void StopSprint();
	
	virtual void OnStartSprint();
	virtual void OnEndSprint();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNLViewSkeletalMeshComponent> ArmMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNLViewSkeletalMeshComponent> ViewWeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNLPlayerCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UControlShakeManager> ControlShakeManager;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UDataTable> FOV_Data;

protected:
	TObjectPtr<UNLCharacterMovementComponent> NLCharacterMovementComponent;

	TObjectPtr<ANLPlayerController> NLPlayerController;

	void OnViewportResized(FViewport* InViewport, uint32 arg);

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

	void OnGameplayTagCountChanged(const FGameplayTag Tag, int32 TagCount);

	bool bIsADS = false;

	void OnADS(bool bInIsADS);

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCurveVector> LoopingShakeCurve_Idle;

public:
	void GetCrouchedHalfHeightAdjust(float& OutHalfHeightAdjust, float& OutScaledHalfHeightAdjust) const;

	FORCEINLINE float GetLookPitch() const { return LookPitch; }

	bool IsListenServerControlledCharacter();

	ANLPlayerController* GetNLPC();

	void UpdateViewWeaponAndAnimLayer(USkeletalMesh* NewWeaponMesh, TSubclassOf<UAnimInstance> WeaponAnimInstanceClass, TSubclassOf<UAnimInstance> NewAnimLayerClass);

	UFUNCTION(BlueprintCallable, Category = Animation)
	virtual float PlayArmsAnimMontage(UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None);

	UFUNCTION(BlueprintCallable, Category = Animation)
	virtual void StopArmsAnimMontage(UAnimMontage* AnimMontage = nullptr);

	UFUNCTION(BlueprintCallable, Category = Animation)
	class UAnimMontage* GetCurrentArmsMontage() const;

	UFUNCTION(BlueprintCallable, Category = Animation)
	virtual float PlayWeaponAnimMontage(UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None);

	UFUNCTION(BlueprintCallable, Category = Animation)
	virtual void StopWeaponAnimMontage(UAnimMontage* AnimMontage = nullptr);

	UFUNCTION(BlueprintCallable, Category = Animation)
	class UAnimMontage* GetCurrentWeaponMontage() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool IsADS() const { return bIsADS; }

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsSprinting)
	bool bIsSprinting = false;

	UFUNCTION()
	virtual void OnRep_IsSprinting();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool IsSprinting() const { return bIsSprinting; }
};
