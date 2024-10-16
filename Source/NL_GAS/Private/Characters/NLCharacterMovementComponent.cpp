// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NLCharacterMovementComponent.h"

#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Characters/NLPlayerCharacter.h"
#include "Kismet/KismetSystemLibrary.h"

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

    if (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy && UpdatedComponent)
    {
        if (!IsLedgeClimbing() && CheckLedgeDetectionCondition())
        {
            const FVector AccelDir = Acceleration.GetSafeNormal2D();
            const FVector Forward = UpdatedComponent->GetForwardVector();
            const float Dot = AccelDir.Dot(Forward);
            if (Dot >= 0.7f) // 전진 입력을 하고있는 동안만
            {
                FHitResult BlockingLedgeHitResult;
                FHitResult StandUpHitResult;
                LedgeClimbTargetLocation = FVector::ZeroVector;
                if (FindBlockingLedge(BlockingLedgeHitResult) && CanStandUpOnLegde(BlockingLedgeHitResult, StandUpHitResult))
                {
                    if (GetLedgeClimbTargetLocation(BlockingLedgeHitResult, StandUpHitResult))
                    {
                        StartLedgeClimb(LedgeClimbTargetLocation, false);
                    }
                }
            }
        }
        else if (IsLedgeClimbing())
        {
            // Check backward Input
            const FVector AccelDir = Acceleration.GetSafeNormal2D();
            const FVector Forward = UpdatedComponent->GetForwardVector();
            const float Dot = AccelDir.Dot(Forward);
            if (Dot <= -0.7f)
            {
                StopLedgeClimb(false);
            }
        }
    }
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

    // Slide boost 쿨타임은 땅에 닿아있고, 슬라이딩을 하지 않을때만 시작
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
    * 순서로 작동하므로, Before movement에 임펄스를 추가하면 임펄스 값이 바로 지워짐
    */
    /**
    * 원래는 AddImpulse로 속도를 추가하려고 했으나, 위와 같은 이유로 속도는 다음 틱에 적용되고,
    * 서버에서도 다음 틱에서야 속도가 추가되어 클라이언트간의 동기화와 예측이 꼬이게 되어서
    * 위치 보정이 되고, 플레이에 불편함이 생김.
    * 따라서 임펄스는 이번 틱에 적용되어야하는데, 앉기 체크는 임펄스가 적용된 이후에 처리하므로
    * 임펄스를 사용할 수 없음.
    * 그러므로 임펄스와 동일하게 작동하는 방식으로 Velocity를 직접 변경하는 방법을 사용했고,
    * 결과는 만족스러움.
    */
    Velocity += Velocity.GetSafeNormal2D() * SlideBoostForce;

    bSlideBoostReady = false;
}

bool UNLCharacterMovementComponent::IsLedgeClimbing() const
{
    return MovementMode == MOVE_Custom && CustomMovementMode == ENLMovementMode::NLMOVE_LedgeClimbing;
}

void UNLCharacterMovementComponent::StartLedgeClimb(FVector TargetLocation, bool bClientSimulation)
{
    if (!HasValidData())
    {
        return;
    }

    LedgeClimbTargetLocation = TargetLocation;
    SetMovementMode(MOVE_Custom, NLMOVE_LedgeClimbing);

    if (ANLPlayerCharacter* NLPlayerCharacter = Cast<ANLPlayerCharacter>(GetCharacterOwner()))
    {
        if (!bClientSimulation)
        {
            NLPlayerCharacter->LedgeClimbData.bIsLedgeClimbing = true;
            NLPlayerCharacter->LedgeClimbData.TargetLocation = LedgeClimbTargetLocation;
        }
        NLPlayerCharacter->OnStartLedgeClimb(LedgeClimbTargetLocation);
    }
}

void UNLCharacterMovementComponent::StopLedgeClimb(bool bClientSimulation)
{
    if (!HasValidData())
    {
        return;
    }

    SetMovementMode(MOVE_Falling);

    if (ANLPlayerCharacter* NLPlayerCharacter = Cast<ANLPlayerCharacter>(GetCharacterOwner()))
    {
        if (!bClientSimulation)
        {
            NLPlayerCharacter->LedgeClimbData.bIsLedgeClimbing = false;
            NLPlayerCharacter->LedgeClimbData.TargetLocation = FVector::ZeroVector;
        }
        NLPlayerCharacter->OnEndLedgeClimb();
    }
}

void UNLCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

    if (MovementMode == MOVE_Falling)
    {
        FallingStarted.ExecuteIfBound();
    }

    if (MovementMode == MOVE_Custom)
    {
        if (CustomMovementMode == ENLMovementMode::NLMOVE_LedgeClimbing)
        {

        }
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

bool UNLCharacterMovementComponent::CheckLedgeDetectionCondition() const
{
    return MovementMode == EMovementMode::MOVE_Falling && !IsCrouching() && !IsSliding() && !IsLedgeClimbing();
}

bool UNLCharacterMovementComponent::FindBlockingLedge(FHitResult& OutHitResult, bool bDebug)
{
    OutHitResult = FHitResult(1.f);
    if (!UpdatedComponent->IsA<UCapsuleComponent>())
    {
        return false;
    }

    float ScaledCapsuleHalfHeight;
    float ScaledCapsuleRadius;
    GetCapsuleScaledSize(ScaledCapsuleHalfHeight, ScaledCapsuleRadius);

    const FVector CapsuleLocation = UpdatedComponent->GetComponentLocation();
    const FVector CapsuleForward = UpdatedComponent->GetForwardVector();

    const FVector TraceStart = CapsuleLocation + CapsuleForward * ScaledCapsuleRadius - UE_KINDA_SMALL_NUMBER;
    const FVector TraceEnd = TraceStart + CapsuleForward * LedgeTraceLength;
    const FVector HalfSize(0.1f, ScaledCapsuleRadius * 0.05f, ScaledCapsuleHalfHeight);
    const FRotator Orientation = UpdatedComponent->GetComponentRotation();

    TArray<AActor*> ActorsToIgnore({ GetOwner() });

    UKismetSystemLibrary::BoxTraceSingleByProfile(
        UpdatedComponent,
        TraceStart + FVector(0.f, 0.f, LedgeTraceBottomHalfHeight),
        TraceEnd + FVector(0.f, 0.f, LedgeTraceBottomHalfHeight),
        HalfSize - FVector(0.f, 0.f, LedgeTraceBottomHalfHeight),
        Orientation,
        FName("Pawn"),
        false,
        ActorsToIgnore,
        bDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
        OutHitResult,
        true,
        FColor::Red,
        FColor::Green,
        1.f
    );

    if (OutHitResult.bBlockingHit)
    {
        const FVector Normal2D = OutHitResult.Normal.GetSafeNormal2D();
        if (Normal2D.SizeSquared() > UE_KINDA_SMALL_NUMBER && 
            FMath::Abs(Normal2D.Dot(OutHitResult.Normal)) >= FMath::Cos(FMath::DegreesToRadians(15.f)))
        {
            return true;
        }
    }
    return false;
}

bool UNLCharacterMovementComponent::CanStandUpOnLegde(const FHitResult& BlockingHitResult, FHitResult& OutHitResult, bool bDebug)
{
    OutHitResult = FHitResult(1.f);
    if (!UpdatedComponent->IsA<UCapsuleComponent>())
    {
        return false;
    }

    float ScaledCapsuleHalfHeight;
    float ScaledCapsuleRadius;
    GetCapsuleScaledSize(ScaledCapsuleHalfHeight, ScaledCapsuleRadius);
    
    const FVector CapsuleLocation = UpdatedComponent->GetComponentLocation();
    const FVector CapsuleForward = UpdatedComponent->GetForwardVector();
    const FVector CapsuleUp = UpdatedComponent->GetUpVector();

    const FVector BlockingNormal2D = BlockingHitResult.Normal.GetSafeNormal2D();
    const FVector TargetLocation2D = BlockingHitResult.ImpactPoint - (BlockingNormal2D * LedgeClimbForwardDist);
    const FVector MidLocation2D(FVector2D((CapsuleLocation + TargetLocation2D) / 2.f), 0.f);
    const float DistToMid2D = FVector::Dist2D(CapsuleLocation, MidLocation2D);

    const FVector HalfSize(ScaledCapsuleRadius + DistToMid2D, ScaledCapsuleRadius, 0.f);

    const FVector TraceStart = FVector(MidLocation2D.X, MidLocation2D.Y, CapsuleLocation.Z) + CapsuleUp * (ScaledCapsuleHalfHeight * 3.f) + UE_KINDA_SMALL_NUMBER;
    const FVector TraceEnd = TraceStart - CapsuleUp * (ScaledCapsuleHalfHeight * 4.f);
    // const FRotator Orientation = UpdatedComponent->GetComponentRotation();
    FRotator ToTargetLocRotation = (TargetLocation2D - CapsuleLocation).Rotation();
    const FRotator Orientation(0.f, ToTargetLocRotation.Yaw, 0.f);

    TArray<AActor*> ActorsToIgnore({ GetOwner() });

    UKismetSystemLibrary::BoxTraceSingleByProfile(
        UpdatedComponent,
        TraceStart,
        TraceEnd,
        HalfSize,
        Orientation,
        FName("Pawn"),
        false,
        ActorsToIgnore,
        bDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
        OutHitResult,
        true,
        FColor::Orange,
        FColor::Emerald,
        1.f
    );

    if (OutHitResult.bBlockingHit)
    {
        return OutHitResult.Distance > ScaledCapsuleHalfHeight * 2;
    }
    return false;
}

bool UNLCharacterMovementComponent::GetLedgeClimbTargetLocation(const FHitResult& BlockingHitResult, const FHitResult& StandUpHitResult)
{
    float CapsuleHalfHeight;
    float CapsuleRadius;
    GetCapsuleScaledSize(CapsuleHalfHeight, CapsuleRadius);

    const FVector BlockingNormal2D = BlockingHitResult.Normal.GetSafeNormal2D();
    const FVector TargetLocation2D = BlockingHitResult.ImpactPoint - (BlockingNormal2D * LedgeClimbForwardDist);

    const float ApproxZ = StandUpHitResult.ImpactPoint.Z;

    const FVector TraceStart = FVector(TargetLocation2D.X, TargetLocation2D.Y, ApproxZ + CapsuleHalfHeight);
    const FVector TraceEnd = TraceStart - FVector(0.f, 0.f, CapsuleHalfHeight);

    TArray<AActor*> ActorsToIgnore({ GetOwner() });

    FHitResult HitRes;
    UKismetSystemLibrary::CapsuleTraceSingleByProfile(
        UpdatedComponent,
        TraceStart,
        TraceEnd,
        CapsuleRadius,
        CapsuleHalfHeight,
        FName("Pawn"),
        false,
        ActorsToIgnore,
        EDrawDebugTrace::None,
        HitRes,
        true
    );
    if (HitRes.bBlockingHit)
    {
        LedgeClimbTargetLocation = HitRes.Location;
        LedgeClimbTargetLocation.Z += 1.f;
        return true;
    }
    return false;
}

void UNLCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
    Super::PhysCustom(deltaTime, Iterations);

    switch (CustomMovementMode)
    {
    case NLMOVE_None:
        break;
    case NLMOVE_LedgeClimbing:
        PhysLedgeClimbing(deltaTime, Iterations);
        break;
    default:
        SetMovementMode(MOVE_None);
        break;
    }
}

void UNLCharacterMovementComponent::PhysLedgeClimbing(float deltaTime, int32 Iterations)
{
    if (!HasValidData())
    {
        return;
    }

    if (deltaTime < MIN_TICK_TIME)
    {
        return;
    }

    float remainingTime = deltaTime;
    while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations))
    {
        Iterations++;
        float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
        remainingTime -= timeTick;

        const FVector OldLocation = UpdatedComponent->GetComponentLocation();
        const FQuat PawnRotation = UpdatedComponent->GetComponentQuat();
        bJustTeleported = false;
        
        const FVector DeltaLocation = LedgeClimbTargetLocation - UpdatedComponent->GetComponentLocation();
        const bool bIsClimbing = DeltaLocation.Z > 0.f;  // 목표 위치가 더 높은곳에 있으면 올라가고있는 상태라고 판단
        if (bIsClimbing)
        {
            Velocity = UpdatedComponent->GetUpVector() * LedgeClimbUpSpeed;
        }
        else
        {
            Velocity = DeltaLocation.GetSafeNormal2D() * LedgeClimbForwardSpeed;
        }
        FVector Adjusted = Velocity * timeTick;
        // Clamp height
        Adjusted.Z = FMath::Min(Adjusted.Z, LedgeClimbTargetLocation.Z - UpdatedComponent->GetComponentLocation().Z);

        // Move
        FHitResult Hit(1.f);
        SafeMoveUpdatedComponent(Adjusted, PawnRotation, true, Hit);

        // 지형에 걸린 경우 표면을 따라 이동
        if (Hit.IsValidBlockingHit())
        {
            SlideAlongSurface(Adjusted, 1.f - Hit.Time, Hit.Normal, Hit, true);
        }

        const FVector NewLocation = UpdatedComponent->GetComponentLocation();
        const FVector NewDeltaLocation = LedgeClimbTargetLocation - NewLocation;
        bool bStuck = (OldLocation - NewLocation).SizeSquared() <= UE_SMALL_NUMBER;
        if (bStuck)
        {
            UE_LOG(LogTemp, Warning, TEXT("Character is stuck while ledge climbing"));
        }
        // 목표 위치에 근접했거나 더 이상 움직이지 못하는 경우 중단
        if (NewDeltaLocation.SizeSquared2D() < 25.f || bStuck)
        {
            Velocity = FVector::ZeroVector;
            StopLedgeClimb(false);

            FFindFloorResult FloorResult;
            FindFloor(NewLocation, FloorResult, false);
            if (FloorResult.IsWalkableFloor())
            {
                ProcessLanded(Hit, remainingTime, Iterations);
            }
            else
            {
                StartFalling(Iterations, remainingTime, timeTick, Adjusted, OldLocation);
            }
            return;
        }
    }
}

void UNLCharacterMovementComponent::GetCapsuleScaledSize(float& OutHalfHeight, float& OutRadius) const
{
    OutHalfHeight = 0.f;
    OutRadius = 0.f;
    if (ACharacter* DefaultCharacter = GetCharacterOwner()->GetClass()->GetDefaultObject<ACharacter>())
    {
        DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleSize(OutRadius, OutHalfHeight);
    }
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
