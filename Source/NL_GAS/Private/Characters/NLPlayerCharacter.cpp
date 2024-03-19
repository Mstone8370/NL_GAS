// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NLPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Characters/NLCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

ANLPlayerCharacter::ANLPlayerCharacter()
    : CrouchInterpSpeed(10.f)
    , CrouchInterpErrorTolerance(0.1f)
    , bIsInterpolatingCrouch(false)
{
    PrimaryActorTick.bCanEverTick = false;
    
    GetMesh()->bOwnerNoSee = true;

    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(FName("SpringArm"));
    SpringArmComponent->TargetArmLength = 0.f;
    SpringArmComponent->bUsePawnControlRotation = true;
    SpringArmComponent->bInheritPitch = true;
    SpringArmComponent->bInheritYaw = true;
    SpringArmComponent->bInheritRoll = false;
    SpringArmComponent->SetupAttachment(GetRootComponent());

    ArmMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("ArmMesh"));
    ArmMesh->bOnlyOwnerSee = true;
    ArmMesh->SetupAttachment(SpringArmComponent);

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(FName("Camera"));
    CameraComponent->SetupAttachment(ArmMesh, FName("camera"));
    CameraComponent->FieldOfView = 90.f;
}

void ANLPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (GetCharacterMovement()->IsA<UNLCharacterMovementComponent>())
    {
        NLCharacterMovementComponent = Cast<UNLCharacterMovementComponent>(GetCharacterMovement());
    }

    check(NLCharacterMovementComponent);

    // Crouch Interpolation
    BaseSpringArmOffset = SpringArmComponent->TargetOffset.Z;
    TargetSpringArmOffset = BaseSpringArmOffset;
}

bool ANLPlayerCharacter::CanJumpInternal_Implementation() const
{
    return /*!bIsCrouched &&*/ JumpIsAllowedInternal();
}

void ANLPlayerCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    InterpolateCrouch(DeltaSeconds);
}

void ANLPlayerCharacter::InterpolateCrouch(float DeltaSeconds)
{
    if (bIsInterpolatingCrouch)
    {
        SpringArmComponent->TargetOffset.Z = FMath::FInterpTo(
            SpringArmComponent->TargetOffset.Z,
            TargetSpringArmOffset,
            DeltaSeconds,
            CrouchInterpSpeed
        );

        if (FMath::IsNearlyEqual(SpringArmComponent->TargetOffset.Z, TargetSpringArmOffset, CrouchInterpErrorTolerance))
        {
            SpringArmComponent->TargetOffset.Z = TargetSpringArmOffset;
            bIsInterpolatingCrouch = false;
        }
    }
}

void ANLPlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

    TargetSpringArmOffset = BaseSpringArmOffset;
    if (NLCharacterMovementComponent->IsFalling() && NLCharacterMovementComponent->bFallingCrouchMaintainSightLocation)
    {
        SpringArmComponent->TargetOffset.Z = TargetSpringArmOffset;

        GetCapsuleComponent()->MoveComponent(
            FVector(0.f, 0.f, -HalfHeightAdjust),
            GetCapsuleComponent()->GetComponentQuat(),
            false, nullptr,
            EMoveComponentFlags::MOVECOMP_NoFlags,
            ETeleportType::TeleportPhysics
        );
    }
    else
    {
        bIsInterpolatingCrouch = true;

        // ĸ�� ��ġ�� Z�������� ���������Ƿ�, �þߴ� �ݴ�� ���ҽ��Ѽ� ������ �þ� ���̸� �����ϵ��� ��.
        SpringArmComponent->TargetOffset.Z -= HalfHeightAdjust;
    }
}

void ANLPlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

    TargetSpringArmOffset = BaseSpringArmOffset - HalfHeightAdjust;
    if (NLCharacterMovementComponent->IsFalling() && NLCharacterMovementComponent->bFallingCrouchMaintainSightLocation)
    {
        SpringArmComponent->TargetOffset.Z = TargetSpringArmOffset;

        GetCapsuleComponent()->MoveComponent(
            FVector(0.f, 0.f, HalfHeightAdjust),
            GetCapsuleComponent()->GetComponentQuat(),
            false, nullptr,
            EMoveComponentFlags::MOVECOMP_NoFlags,
            ETeleportType::TeleportPhysics
        );
    }
    else
    {
        bIsInterpolatingCrouch = true;

        // ĸ�� ��ġ�� Z�������� ���������Ƿ�, �þߴ� �ݴ�� �������Ѽ� ������ �þ� ���̸� �����ϵ��� ��.
        SpringArmComponent->TargetOffset.Z += HalfHeightAdjust;
    }
}
