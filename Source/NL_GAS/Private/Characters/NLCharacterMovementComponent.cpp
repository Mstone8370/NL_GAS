// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NLCharacterMovementComponent.h"

#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

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

void UNLCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

    if (MovementMode == MOVE_Falling)
    {
        FallingStarted.ExecuteIfBound();
    }
}
