// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NLCharacterMovementComponent.h"

#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Characters/NLPlayerCharacter.h"

void UNLCharacterMovementComponent::BeginPlay()
{
    Super::BeginPlay();

    DefaultGroundFriction = GroundFriction;
    DefaultBrakingDecelerationWalking = BrakingDecelerationWalking;
    DefaultMaxAcceleration = MaxAcceleration;
}

bool UNLCharacterMovementComponent::CanAttemptJump() const
{
    return IsJumpAllowed() &&
        // !bWantsToCrouch &&
        (IsMovingOnGround() || IsFalling()); // Falling included for double-jump and non-zero jump hold time, but validated by character.
}

void UNLCharacterMovementComponent::Crouch(bool bClientSimulation)
{
    // 기존의 Crouch 함수에서 캡슐의 크기를 변경하는 코드를 분리했음.
    if (!HasValidData())
    {
        return;
    }

    if (!bClientSimulation && !CanCrouchInCurrentState())
    {
        return;
    }

    // 캐릭터의 bIsCrouched는 캐릭터 무브먼트의 MaxSpeed를 결정지음.
    // 앉기를 시작할때부터 속도를 낮추길 원하므로 여기에서 bIsCrouched를 설정.
    CharacterOwner->bIsCrouched = true;

    if (CanSlideInCurrentState())
    {
        Slide(bClientSimulation);
        ShrinkCapsuleHeight(bClientSimulation);
    }
}

void UNLCharacterMovementComponent::UnCrouch(bool bClientSimulation)
{
    Super::UnCrouch(bClientSimulation);

    if (IsSliding())
    {
        StopSlide(bClientSimulation);
    }
}

bool UNLCharacterMovementComponent::DoJump(bool bReplayingMoves)
{
    if (CharacterOwner && CharacterOwner->CanJump())
    {
        // Don't jump if we can't move up/down.
        if (!bConstrainToPlane || FMath::Abs(PlaneConstraintNormal.Z) != 1.f)
        {
            if (HasCustomGravity())
            {
                FVector GravityRelativeVelocity = RotateWorldToGravity(Velocity);
                GravityRelativeVelocity.Z = FMath::Max<FVector::FReal>(GravityRelativeVelocity.Z, JumpZVelocity);
                Velocity = RotateGravityToWorld(GravityRelativeVelocity);
            }
            else
            {
                Velocity.Z = FMath::Max<FVector::FReal>(Velocity.Z, JumpZVelocity);
            }

            if (IsSliding())
            {
                StopSlide(false);
            }

            SetMovementMode(MOVE_Falling);
            return true;
        }
    }

    return false;
}

void UNLCharacterMovementComponent::ShrinkCapsuleHeight(bool bClientSimulation)
{
    // Codes from UCharacterMovementComponent::Crouch(bool bClientSimulation)

    if (!HasValidData())
    {
        return;
    }

    if (!bClientSimulation && !CanCrouchInCurrentState())
    {
        return;
    }

    // See if collision is already at desired size.
    if (CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == GetCrouchedHalfHeight())
    {
        if (!bClientSimulation)
        {
            CharacterOwner->bIsCrouched = true;
        }
        return;
    }

    if (bClientSimulation && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
    {
        // restore collision size before crouching
        ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
        CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());
        bShrinkProxyCapsule = true;
    }

    // Change collision size to crouching dimensions
    const float ComponentScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
    const float OldUnscaledHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
    const float OldUnscaledRadius = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius();
    // Height is not allowed to be smaller than radius.
    const float ClampedCrouchedHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, GetCrouchedHalfHeight());
    CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, ClampedCrouchedHalfHeight);
    float HalfHeightAdjust = (OldUnscaledHalfHeight - ClampedCrouchedHalfHeight);
    float ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

    if (!bClientSimulation)
    {
        // Crouching to a larger height? (this is rare)
        if (ClampedCrouchedHalfHeight > OldUnscaledHalfHeight)
        {
            FCollisionQueryParams CapsuleParams(SCENE_QUERY_STAT(CrouchTrace), false, CharacterOwner);
            FCollisionResponseParams ResponseParam;
            InitCollisionParams(CapsuleParams, ResponseParam);
            const bool bEncroached = GetWorld()->OverlapBlockingTestByChannel(UpdatedComponent->GetComponentLocation() - FVector(0.f, 0.f, ScaledHalfHeightAdjust), FQuat::Identity,
                UpdatedComponent->GetCollisionObjectType(), GetPawnCapsuleCollisionShape(SHRINK_None), CapsuleParams, ResponseParam);

            // If encroached, cancel
            if (bEncroached)
            {
                CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(OldUnscaledRadius, OldUnscaledHalfHeight);
                return;
            }
        }

        if (bCrouchMaintainsBaseLocation)
        {
            // Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
            UpdatedComponent->MoveComponent(FVector(0.f, 0.f, -ScaledHalfHeightAdjust), UpdatedComponent->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);
        }
    }

    bForceNextFloorCheck = true;

    // OnStartCrouch takes the change from the Default size, not the current one (though they are usually the same).
    const float MeshAdjust = ScaledHalfHeightAdjust;
    ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
    HalfHeightAdjust = (DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - ClampedCrouchedHalfHeight);
    ScaledHalfHeightAdjust = HalfHeightAdjust * ComponentScale;

    AdjustProxyCapsuleSize();
    CharacterOwner->OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

    // Don't smooth this change in mesh position
    if ((bClientSimulation && CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy) || (IsNetMode(NM_ListenServer) && CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy))
    {
        FNetworkPredictionData_Client_Character* ClientData = GetPredictionData_Client_Character();
        if (ClientData)
        {
            ClientData->MeshTranslationOffset -= FVector(0.f, 0.f, MeshAdjust);
            ClientData->OriginalMeshTranslationOffset = ClientData->MeshTranslationOffset;
        }
    }
}

float UNLCharacterMovementComponent::GetMaxSpeed() const
{
    if (MovementMode == MOVE_Walking || MovementMode == MOVE_NavWalking)
    {
        if (!IsCrouching())
        {
            return IsSprinting() ? MaxSprintSpeed : MaxWalkSpeed;
        }
    }

    return Super::GetMaxSpeed();
}

void UNLCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
    Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

    bWasFalling = (MovementMode == EMovementMode::MOVE_Falling);
}

void UNLCharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
    Super::UpdateCharacterStateAfterMovement(DeltaSeconds);

    if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)
    {
        // Check Sprint state
        const bool bIsSprinting = IsSprinting();
        if (bIsSprinting && (!bWantsToSprint || !CanSprintInCurrentState()))
        {
            StopSprint(false);
        }
        else if (!bIsSprinting && bWantsToSprint && CanSprintInCurrentState())
        {
            Sprint(false);
        }
        
        // Check Slide state
        if (IsSliding())
        {
            const double SquaredSpeed = Velocity.SizeSquared2D();
            if (!IsCrouching() || SquaredSpeed <= MaxWalkSpeedCrouched * MaxWalkSpeedCrouched)
            {
                StopSlide(false);
            }
        }

        // Check laded and crouched
        if (bWasFalling && MovementMode == EMovementMode::MOVE_Walking)
        {
            if (IsCrouching() && CanSlideInCurrentState())
            {
                Slide(false);
            }
        }

        // Check Slide boost here
        // 여기에서 적용해야 다음 틱에 이동하기 전에 임펄스가 정상적으로 적용됨.
        if (bJustSlided && CanApplySlideBoost())
        {
            ApplySlideBoost();
        }
        bJustSlided = false;
    }
}

FNetworkPredictionData_Client* UNLCharacterMovementComponent::GetPredictionData_Client() const
{
    if (ClientPredictionData == nullptr)
    {
        UNLCharacterMovementComponent* MutableThis = const_cast<UNLCharacterMovementComponent*>(this);
        MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_NLCharacter(*this);
    }

    return ClientPredictionData;
}

bool UNLCharacterMovementComponent::IsSprinting() const
{
    if (GetCharacterOwner())
    {
        if (ANLPlayerCharacter* NLPlayerCharacter = Cast<ANLPlayerCharacter>(GetCharacterOwner()))
        {
            return NLPlayerCharacter->IsSprinting();
        }
    }
    return false;
}

bool UNLCharacterMovementComponent::CanSprintInCurrentState() const
{
    if (GetCharacterOwner())
    {
        if (ANLPlayerCharacter* NLPlayerCharacter = Cast<ANLPlayerCharacter>(GetCharacterOwner()))
        {
            if (NLPlayerCharacter->IsADS() || NLPlayerCharacter->bSprintBlocked)
            {
                return false;
            }
        }
    }
    if (Velocity.SizeSquared2D() < 60000.f)  // 현재 속도가 충분히 빠르지 않으면 달리지 않음
    {
        return false;
    }
    return IsMovingOnGround() && !IsCrouching() && UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics();
}

void UNLCharacterMovementComponent::Sprint(bool bClientSimulation)
{
    if (!HasValidData())
    {
        return;
    }

    if (!bClientSimulation && !CanSprintInCurrentState())
    {
        return;
    }

    if (GetCharacterOwner())
    {
        if (ANLPlayerCharacter* NLPlayerCharacter = Cast<ANLPlayerCharacter>(GetCharacterOwner()))
        {
            if (!bClientSimulation)
            {
                NLPlayerCharacter->bIsSprinting = true;
            }
            NLPlayerCharacter->OnStartSprint();
        }
    }
}

void UNLCharacterMovementComponent::StopSprint(bool bClientSimulation)
{
    if (!HasValidData())
    {
        return;
    }

    if (GetCharacterOwner())
    {
        if (ANLPlayerCharacter* NLPlayerCharacter = Cast<ANLPlayerCharacter>(GetCharacterOwner()))
        {
            if (!bClientSimulation)
            {
                NLPlayerCharacter->bIsSprinting = false;
            }
            NLPlayerCharacter->OnEndSprint();
        }
    }
}

bool UNLCharacterMovementComponent::IsSliding() const
{
    if (GetCharacterOwner())
    {
        if (ANLPlayerCharacter* NLPlayerCharacter = Cast<ANLPlayerCharacter>(GetCharacterOwner()))
        {
            return NLPlayerCharacter->IsSliding();
        }
    }
    return false;
}

bool UNLCharacterMovementComponent::CanSlideInCurrentState() const
{
    if (GetCharacterOwner())
    {
        if (ANLPlayerCharacter* NLPlayerCharacter = Cast<ANLPlayerCharacter>(GetCharacterOwner()))
        {
            if (GetLastUpdateVelocity().SquaredLength() <= MaxWalkSpeed * MaxWalkSpeed + 2500.f /*padding*/)
            {
                return false;
            }
        }
    }
    return IsMovingOnGround() && !IsSliding() && IsCrouching() && UpdatedComponent && !UpdatedComponent->IsSimulatingPhysics();
}

void UNLCharacterMovementComponent::Slide(bool bClientSimulation)
{
    if (!HasValidData())
    {
        return;
    }

    if (!bClientSimulation && !CanSlideInCurrentState())
    {
        return;
    }

    if (GetCharacterOwner())
    {
        if (ANLPlayerCharacter* NLPlayerCharacter = Cast<ANLPlayerCharacter>(GetCharacterOwner()))
        {
            if (!bClientSimulation)
            {
                NLPlayerCharacter->bIsSliding = true;
            }
            NLPlayerCharacter->OnStartSlide();
        }
    }
    
    GroundFriction = SlideGroundFriction;
    BrakingDecelerationWalking = SlideBrakingDecelerationWalking;
    MaxAcceleration = SlideMaxAcceleration;

    bJustSlided = true;
}

void UNLCharacterMovementComponent::StopSlide(bool bClientSimulation)
{
    if (!HasValidData())
    {
        return;
    }

    if (GetCharacterOwner())
    {
        if (ANLPlayerCharacter* NLPlayerCharacter = Cast<ANLPlayerCharacter>(GetCharacterOwner()))
        {
            if (!bClientSimulation)
            {
                NLPlayerCharacter->bIsSliding = false;
            }
            NLPlayerCharacter->OnEndSlide();
        }
    }

    GroundFriction = DefaultGroundFriction;
    BrakingDecelerationWalking = DefaultBrakingDecelerationWalking;
    MaxAcceleration = DefaultMaxAcceleration;
}

bool UNLCharacterMovementComponent::CanApplySlideBoost() const
{
    return bSlideBoost;
}

void UNLCharacterMovementComponent::ApplySlideBoost()
{
    /**
    * Apply Accumulated Force
    * Update Character State Before Movement
    * Handle Pending Launch
    * Clear Accumulated Force
    * 순서로 작동하므로, Before movement에 슬라이드 부스트를 주면 임펄스 값이 바로 지워짐
    */
    AddImpulse(Velocity.GetSafeNormal2D() * SlideBoostForce, true);
}

void UNLCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

    if (MovementMode == MOVE_Falling)
    {
        FallingStarted.ExecuteIfBound();
    }
}

void UNLCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
    Super::UpdateFromCompressedFlags(Flags);

    bWantsToSprint = ((Flags & FSavedMove_Character::FLAG_Custom_0) != 0);
}

void FSavedMove_NLCharacter::Clear()
{
    Super::Clear();

    bWantsToSprint = false;
}

void FSavedMove_NLCharacter::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
    Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

    if (UNLCharacterMovementComponent* NLCMC = Cast<UNLCharacterMovementComponent>(C->GetCharacterMovement()))
    {
        bWantsToSprint = NLCMC->bWantsToSprint;
    }
}

uint8 FSavedMove_NLCharacter::GetCompressedFlags() const
{
    uint8 Result = Super::GetCompressedFlags();

    if (bWantsToSprint)
    {
        Result |= FLAG_Custom_0;
    }

    return Result;
}

FNetworkPredictionData_Client_NLCharacter::FNetworkPredictionData_Client_NLCharacter(const UCharacterMovementComponent& ClientMovement)
    : Super(ClientMovement)
{}

FSavedMovePtr FNetworkPredictionData_Client_NLCharacter::AllocateNewMove()
{
    return FSavedMovePtr(new FSavedMove_NLCharacter());
}
