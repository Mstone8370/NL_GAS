// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NLPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Characters/NLCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Player/NLPlayerState.h"
#include "Player/NLPlayerController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/NLAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

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
    ArmMesh->CastShadow = 0;
    ArmMesh->SetupAttachment(SpringArmComponent);

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(FName("Camera"));
    CameraComponent->SetupAttachment(ArmMesh, FName("camera"));
    CameraComponent->FieldOfView = 90.f;
}

void ANLPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ANLPlayerCharacter, bIsCapsuleShrinked, COND_SimulatedOnly);
}

void ANLPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (GetCharacterMovement()->IsA<UNLCharacterMovementComponent>())
    {
        NLCharacterMovementComponent = Cast<UNLCharacterMovementComponent>(GetCharacterMovement());
    }

    check(NLCharacterMovementComponent);

    NLCharacterMovementComponent->FallingStarted.BindUObject(this, &ANLPlayerCharacter::OnFallingStarted);

    // Set Crouch Interpolation default value
    BaseSpringArmOffset = SpringArmComponent->TargetOffset.Z;
    TargetSpringArmOffset = BaseSpringArmOffset;
}

bool ANLPlayerCharacter::CanJumpInternal_Implementation() const
{
    return /*!bIsCrouched &&*/ JumpIsAllowedInternal();
}

void ANLPlayerCharacter::InitAbilityActorInfo()
{
    ANLPlayerState* PS = GetPlayerState<ANLPlayerState>();

    check(PS);

    AbilitySystemComponent = PS->GetAbilitySystemComponent();
    AbilitySystemComponent->InitAbilityActorInfo(PS, this);

    AttributeSet = PS->GetAttributeSet();
    // TODO: Init Default Attributes
}

void ANLPlayerCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    InterpolateCrouch(DeltaSeconds);
}

void ANLPlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    // On Server
    InitAbilityActorInfo();
    // TODO: Give Start up Abilities.
    AddStartupAbilities();
}

void ANLPlayerCharacter::OnRep_PlayerState()
{
    // On Client
    InitAbilityActorInfo();
}

bool ANLPlayerCharacter::CanCrouch() const
{
    return !bIsCrouched && GetCharacterMovement() && GetCharacterMovement()->CanEverCrouch() && GetRootComponent() && !GetRootComponent()->IsSimulatingPhysics();
}

void ANLPlayerCharacter::Crouch(bool bClientSimulation)
{
    // On Client

    Super::Crouch(bClientSimulation);

    if (!CanCrouch())
    {
        return;
    }
    
    // Start interpolate
    bIsInterpolatingCrouch = true;

    float HalfHeightAdjust = 0.f;
    float ScaledHalfHeightAdjust = 0.f;
    GetCrouchedHalfHeightAdjust(HalfHeightAdjust, ScaledHalfHeightAdjust);

    TargetSpringArmOffset = BaseSpringArmOffset - HalfHeightAdjust;
    if (GetCharacterMovement()->IsWalking())
    {
        // Walking�϶� ������ �þ߰� HalfHeightAdjust�� �ι踸ŭ ����������.
        TargetSpringArmOffset -= HalfHeightAdjust;
    }
}

void ANLPlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

    bIsCapsuleShrinked = true;

    if (GetCharacterMovement()->IsWalking())
    {
        // Walking�϶� ĸ�� ũ�Ⱑ �پ���, �Ʒ����� HalfHeightAdjust��ŭ ĸ���� ��ġ�� �������Ƿ�
        // �þ� ���� ������ ���� �������� ����.
        SpringArmComponent->TargetOffset.Z += HalfHeightAdjust;
    }

    if (GetLocalRole() == ROLE_AutonomousProxy && GetNetMode() == NM_Client)
    {
        Server_CapsuleShrinked(true);
    }
}

void ANLPlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

    bIsCapsuleShrinked = false;

    TargetSpringArmOffset = BaseSpringArmOffset;
    bIsInterpolatingCrouch = true;
    
    if (GetCharacterMovement()->IsWalking())
    {
        // Walking�϶� ĸ�� ũ�Ⱑ �þ��, �Ʒ����� HalfHeightAdjust��ŭ ĸ���� ��ġ�� �ö󰡹Ƿ�
        // �þ� ���� ������ ���� �������� ����.
        SpringArmComponent->TargetOffset.Z -= HalfHeightAdjust;
    }

    if (GetLocalRole() == ROLE_AutonomousProxy && GetNetMode() == NM_Client)
    {
        Server_CapsuleShrinked(false);
    }
}

void ANLPlayerCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    if (bIsCrouched && bIsInterpolatingCrouch)
    {
        float HalfHeightAdjust = 0.f;
        float ScaledHalfHeightAdjust = 0.f;
        GetCrouchedHalfHeightAdjust(HalfHeightAdjust, ScaledHalfHeightAdjust);

        TargetSpringArmOffset = BaseSpringArmOffset - 2 * HalfHeightAdjust;
    }
}

void ANLPlayerCharacter::OnFallingStarted()
{
    /**
    * Falling�� ���������� ĸ�� �Ʒ����� HalfHeightAdjust��ŭ ���̸� ������ ����.
    * ������ Walking�϶� �ɱ⸦ �����ߴٸ� ĸ���� ���̸� ������� �����ϰ� TargetSpringArmOffset�� �ι�� ������.
    * ���� Crouch Interpolation���̶�� ��ǥ ���� ���� �����ؾ���.
    *
    * �������� �������� ��, bIsCrouched�� true�̰�, Crouch Interpolation���� ����� Walking�϶� �ɱ⸦ �����ߴٴ� ��.
    */
    if (bIsCrouched && bIsInterpolatingCrouch)
    {
        float HalfHeightAdjust = 0.f;
        float ScaledHalfHeightAdjust = 0.f;
        GetCrouchedHalfHeightAdjust(HalfHeightAdjust, ScaledHalfHeightAdjust);

        TargetSpringArmOffset = BaseSpringArmOffset - HalfHeightAdjust;
    }
}

void ANLPlayerCharacter::OnRep_IsCapsuleShrinked()
{
    // On Client, Not owned character

    if (NLCharacterMovementComponent && bIsCapsuleShrinked)
    {
        NLCharacterMovementComponent->ShrinkCapsuleHeight(true);
    }
}

void ANLPlayerCharacter::Server_CapsuleShrinked_Implementation(bool bInShrinked)
{
    // On Server

    bIsCapsuleShrinked = bInShrinked;

    if (NLCharacterMovementComponent && bIsCapsuleShrinked)
    {
        NLCharacterMovementComponent->ShrinkCapsuleHeight();
    }
}

void ANLPlayerCharacter::InterpolateCrouch(float DeltaSeconds)
{
    if (!bIsInterpolatingCrouch)
    {
        return;
    }

    const float NewTargetOffsetZ = FMath::FInterpTo(
        SpringArmComponent->TargetOffset.Z,
        TargetSpringArmOffset,
        DeltaSeconds,
        CrouchInterpSpeed
    );
    SpringArmComponent->TargetOffset.Z = NewTargetOffsetZ;

    if (FMath::IsNearlyEqual(NewTargetOffsetZ, TargetSpringArmOffset, CrouchInterpErrorTolerance))
    {
        SpringArmComponent->TargetOffset.Z = TargetSpringArmOffset;
        bIsInterpolatingCrouch = false;
        
        if (bIsCrouched && !bIsCapsuleShrinked)
        {
            // �ɱ�� interpolation�� ���۵Ǿ��ٸ� �� �������� �Ұ����� ���� �����Ƿ� �߰� Ȯ�ξ��� ����.
            NLCharacterMovementComponent->ShrinkCapsuleHeight();
        }
    }
}

void ANLPlayerCharacter::GetCrouchedHalfHeightAdjust(float& OutHalfHeightAdjust, float& OutScaledHalfHeightAdjust) const
{
    // Change collision size to crouching dimensions
    const float ComponentScale = GetCapsuleComponent()->GetShapeScale();
    const float OldUnscaledHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
    const float OldUnscaledRadius = GetCapsuleComponent()->GetUnscaledCapsuleRadius();
    // Height is not allowed to be smaller than radius.
    const float ClampedCrouchedHalfHeight = FMath::Max3(0.f, OldUnscaledRadius, GetCharacterMovement()->GetCrouchedHalfHeight());
    
    OutHalfHeightAdjust = (OldUnscaledHalfHeight - ClampedCrouchedHalfHeight);
    OutScaledHalfHeightAdjust = OutHalfHeightAdjust * ComponentScale;
}
