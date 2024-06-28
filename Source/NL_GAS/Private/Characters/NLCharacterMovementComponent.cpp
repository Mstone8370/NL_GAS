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
    // ������ Crouch �Լ����� ĸ���� ũ�⸦ �����ϴ� �ڵ带 �и�����.
    if (!HasValidData())
    {
        return;
    }

    if (!bClientSimulation && !CanCrouchInCurrentState())
    {
        return;
    }

    // ĳ������ bIsCrouched�� ĳ���� �����Ʈ�� MaxSpeed�� ��������.
    // �ɱ⸦ �����Ҷ����� �ӵ��� ���߱� ���ϹǷ� ���⿡�� bIsCrouched�� ����.
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

            SetMovementMode(MOVE_Falling);

            if (IsSliding())
            {
                StopSlide(false);
            }

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

            // Check Slide boost cooltime timer
            /**
            * If Slide boost feature is true and
            * currently not sliding and
            * Slide boost is not ready and
            * Slide boost cooltime timer is not setted
            * Check StopSlide() function for additional condition
            */
            if (bSlideBoost && !IsSliding() && !bSlideBoostReady)
            {
                if (GetWorld())
                {
                    FTimerManager& TimerManager = GetWorld()->GetTimerManager();
                    if (!TimerManager.IsTimerActive(SlideBoostCooltimeTimer))
                    {
                        GetWorld()->GetTimerManager().SetTimer(
                            SlideBoostCooltimeTimer,
                            this,
                            &UNLCharacterMovementComponent::OnSlideBoostCooltimeFinished,
                            SlideBoostCooltime,
                            false
                        );
                    }
                }
            }
        }
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
    if (Velocity.SizeSquared2D() < 60000.f)  // ���� �ӵ��� ����� ������ ������ �޸��� ����
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
            const double Speed2DSquared = GetLastUpdateVelocity().SizeSquared2D();
            if (!bWasFalling && Speed2DSquared <= MaxWalkSpeed * MaxWalkSpeed + 2500.f /*padding*/)
            {
                return false;
            }
            else if (bWasFalling && Speed2DSquared <= MaxWalkSpeed * MaxWalkSpeed - 2500.f /*padding*/)
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

    if (CanApplySlideBoost())
    {
        ApplySlideBoost();
    }
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

    // Slide boost ��Ÿ���� ���� ����ְ�, �����̵��� ���� �������� ����
    if (bSlideBoost && MovementMode != EMovementMode::MOVE_Falling && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            SlideBoostCooltimeTimer,
            this,
            &UNLCharacterMovementComponent::OnSlideBoostCooltimeFinished,
            SlideBoostCooltime,
            false
        );
    }
}

bool UNLCharacterMovementComponent::CanApplySlideBoost() const
{
    return bSlideBoost && bSlideBoostReady;
}

void UNLCharacterMovementComponent::ApplySlideBoost()
{
    /**
    * 1. Apply Accumulated Force
    * 2. Update Character State Before Movement
    * 3. Handle Pending Launch
    * 4. Clear Accumulated Force
    * ������ �۵��ϹǷ�, Before movement�� ���޽��� �߰��ϸ� ���޽� ���� �ٷ� ������
    */
    /**
    * ������ AddImpulse�� �ӵ��� �߰��Ϸ��� ������, ���� ���� ������ �ӵ��� ���� ƽ�� ����ǰ�,
    * ���������� ���� ƽ������ �ӵ��� �߰��Ǿ� Ŭ���̾�Ʈ���� ����ȭ�� ������ ���̰� �Ǿ
    * ��ġ ������ �ǰ�, �÷��̿� �������� ����.
    * ���� ���޽��� �̹� ƽ�� ����Ǿ���ϴµ�, �ɱ� üũ�� ���޽��� ����� ���Ŀ� ó���ϹǷ�
    * ���޽��� ����� �� ����.
    * �׷��Ƿ� ���޽��� �����ϰ� �۵��ϴ� ������� Velocity�� ���� �����ϴ� ����� ����߰�,
    * ����� ����������.
    */
    Velocity += Velocity.GetSafeNormal2D() * SlideBoostForce;

    bSlideBoostReady = false;
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

void UNLCharacterMovementComponent::OnSlideBoostCooltimeFinished()
{
    bSlideBoostReady = true;
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

bool FSavedMove_NLCharacter::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
    if (FSavedMove_NLCharacter* NewNLMove = static_cast<FSavedMove_NLCharacter*>(NewMove.Get()))
    {
        if (NewNLMove->bWantsToSprint != bWantsToSprint)
        {
            return false;
        }
        if (NewNLMove->bWantsToCrouch != bWantsToCrouch)
        {
            return false;
        }
    }

    return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void FSavedMove_NLCharacter::PrepMoveFor(ACharacter* C)
{
    Super::PrepMoveFor(C);

    if (UNLCharacterMovementComponent* NLCMC = Cast<UNLCharacterMovementComponent>(C->GetCharacterMovement()))
    {
        NLCMC->bWantsToSprint = bWantsToSprint;
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
