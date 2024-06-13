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
	/* Crouch해서 캡슐의 크기가 줄었을 때 호출됨. */
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
	* 서버에서부터 bIsCapsuleShrinked 변수가 레플리케이트 된 경우.
	* 이 캐릭터가 클라이언트 소유가 아니어서 시뮬레이션인 경우에만 호출됨.
	* 이 캐릭터를 소유하고있는곳에서 캡슐의 크기가 변경되었음을 의미하므로,
	* 관찰하는 입장에서도 크기를 조정해야함.
	*/
	UFUNCTION()
	void OnRep_IsCapsuleShrinked();

	/**
	* 소유한 캐릭터의 클라이언트에서 Crouch 또는 UnCrouch를 해서 캡슐 크기를 변경했을때 호출됨.
	* 서버의 bIsCapsuleShrinked 값을 변경해서 다른 클라이언트에게도 레플리케이트 되게 함.
	* @param bInShrinked - 캡슐 크기가 줄어든 경우 true.
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
