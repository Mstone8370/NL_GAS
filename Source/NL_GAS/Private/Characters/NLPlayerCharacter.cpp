// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NLPlayerCharacter.h"

#include "Components/NLPlayerCameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Characters/NLCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Player/NLPlayerState.h"
#include "Player/NLPlayerController.h"
#include "HUD/NLHUD.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/NLAbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "NLFunctionLibrary.h"
#include "Data/WeaponInfo.h"
#include "NLGameplayTags.h"
#include "Components/NLCharacterComponent.h"
#include "Components/ControlShakeManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "UnrealClient.h"
#include "Components/NLViewSkeletalMeshComponent.h"
#include "Data/NLDataTableRows.h"

ANLPlayerCharacter::ANLPlayerCharacter()
    : LookPitchRepTime(0.02f)
    , LookPitch(0.f)
    , CrouchInterpSpeed(10.f)
    , CrouchInterpErrorTolerance(0.1f)
    , bIsCapsuleShrinked(false)
    , bIsInterpolatingCrouch(false)
    , BaseSpringArmOffset(0.f)
    , TargetSpringArmOffset(0.f)
{
    PrimaryActorTick.bCanEverTick = false;
    
    GetMesh()->bOwnerNoSee = true;

    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(FName("SpringArm"));
    SpringArmComponent->TargetArmLength = 0.f;
    SpringArmComponent->bUsePawnControlRotation = true;
    SpringArmComponent->bInheritPitch = true;
    SpringArmComponent->bInheritYaw = true;
    SpringArmComponent->bInheritRoll = false;
    SpringArmComponent->bDoCollisionTest = false;
    SpringArmComponent->SetupAttachment(GetRootComponent());

    ArmMesh = CreateDefaultSubobject<UNLViewSkeletalMeshComponent>(FName("ArmMesh"));
    ArmMesh->bOnlyOwnerSee = true;
    ArmMesh->CastShadow = 0;
    ArmMesh->SetupAttachment(SpringArmComponent);

    ViewWeaponMesh = CreateDefaultSubobject<UNLViewSkeletalMeshComponent>(FName("ViewWeaponMesh"));
    ViewWeaponMesh->bOnlyOwnerSee = true;
    ViewWeaponMesh->CastShadow = 0;
    ViewWeaponMesh->SetupAttachment(ArmMesh, FName("weapon"));

    CameraComponent = CreateDefaultSubobject<UNLPlayerCameraComponent>(FName("Camera"));
    CameraComponent->SetupAttachment(ArmMesh, FName("camera"));
    CameraComponent->FieldOfView = 110.f;

    ControlShakeManager = CreateDefaultSubobject<UControlShakeManager>(FName("ControlShakeManager"));
}

void ANLPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ANLPlayerCharacter, bIsCapsuleShrinked, COND_SimulatedOnly);
    DOREPLIFETIME_CONDITION_NOTIFY(ANLPlayerCharacter, LookPitch, COND_SimulatedOnly, REPNOTIFY_OnChanged);
}

void ANLPlayerCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    ArmMesh->Initialize();
    ViewWeaponMesh->Initialize();
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

    // Look Pitch Rep Timer
    if (HasAuthority())
    {
        GetWorldTimerManager().SetTimer(LookPitchRepTimerHandle, this, &ANLPlayerCharacter::Server_InvokeLookPitchReplication, LookPitchRepTime, true);
    }

    // FOV setup
    CameraComponent->SetBaseFOV(CameraComponent->FieldOfView);
    if (GetLocalRole() != ROLE_SimulatedProxy)
    {
        FViewport::ViewportResizedEvent.AddUObject(this, &ANLPlayerCharacter::OnViewportResized);
    }
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

    AbilitySystemComponent->RegisterGameplayTagEvent(Ability_Weapon_Secondary, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ANLPlayerCharacter::OnGameplayTagCountChanged);

    if (ANLPlayerController* PC = Cast<ANLPlayerController>(GetController()))
    {
        if (ANLHUD* HUD = Cast<ANLHUD>(PC->GetHUD()))
        {
            HUD->Initialize(PC, PS, AbilitySystemComponent, AttributeSet, NLCharacterComponent);
        }
    }
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
    
    AddStartupAbilities();

    NLCharacterComponent->AddStartupWeapons();
    NLCharacterComponent->ValidateStartupWeapons();
}

void ANLPlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    // On Client
    InitAbilityActorInfo();

    NLCharacterComponent->ValidateStartupWeapons();
}

void ANLPlayerCharacter::OnRep_Controller()
{
    Super::OnRep_Controller();

    // On Client
    NLPlayerController = Cast<ANLPlayerController>(GetController());
}

bool ANLPlayerCharacter::CanSwapWeaponSlot_Implementation(int32 NewSlot)
{
    return NLCharacterComponent->CanSwapWeaponSlot(NewSlot);
}

void ANLPlayerCharacter::TrySwapWeaponSlot_Implementation(int32 NewSlot)
{
    return NLCharacterComponent->TrySwapWeaponSlot(NewSlot);
}

void ANLPlayerCharacter::GetWeaponHandIKLocation_Implementation(FName LeftIKSocketName, FName RightIKSocketName, FVector& OutLeftIKLocation, FVector& OutRightIKLocation) const
{
    OutLeftIKLocation = FVector::ZeroVector;
    OutRightIKLocation = FVector::ZeroVector;

    if (ViewWeaponMesh->GetSkinnedAsset())
    {
        OutLeftIKLocation = ViewWeaponMesh->GetSocketLocation(LeftIKSocketName);
        OutRightIKLocation = ViewWeaponMesh->GetSocketLocation(RightIKSocketName);
    }
}

float ANLPlayerCharacter::PlayCurrentWeaponMontage_Implementation(const FGameplayTag& MontageTag)
{
    return NLCharacterComponent->PlayCurrentWeaponMontage(MontageTag);
}

void ANLPlayerCharacter::WeaponFired_Implementation(TSubclassOf<UCameraShakeBase> CameraShakeBaseClass)
{
    // Add recoil
    ControlShakeManager->WeaponFired(NLCharacterComponent->GetCurrentWeaponTag());

    // Add CamearShake
    if (CameraShakeBaseClass)
    {
        if (APlayerController* PC = GetLocalViewingPlayerController())
        {
            PC->ClientStartCameraShake(CameraShakeBaseClass);
        }
    }
}

bool ANLPlayerCharacter::StartReload_Implementation()
{
    return NLCharacterComponent->StartReload();
}

void ANLPlayerCharacter::OnWeaponReloadStateChanged_Implementation(const FGameplayTag& WeaponTag, const FGameplayTag& StateTag)
{
    NLCharacterComponent->OnWeaponReloadStateChanged(WeaponTag, StateTag);
}

float ANLPlayerCharacter::GetWeaponSpreadValue_Implementation()
{
    return NLCharacterComponent->GetCurrentWeaponSpreadValue(
        bIsADS,
        GetCharacterMovement()->IsFalling(),
        bIsCrouched,
        GetCharacterMovement()->GetLastUpdateVelocity().SquaredLength(),
        ControlShakeManager->GetRecoilOffset(NLCharacterComponent->GetCurrentWeaponTag())
    );
}

bool ANLPlayerCharacter::CommitWeaponCost_Implementation(bool& bIsLast)
{
    return NLCharacterComponent->CommitWeaponCost(bIsLast);
}

bool ANLPlayerCharacter::CanAttack_Implementation()
{
    return NLCharacterComponent->CanAttack();
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

    if (IsCrouchInterpolatableCharacter() && GetCharacterMovement()->IsWalking())
    {
        // Walking�϶� ĸ�� ũ�Ⱑ �پ���, �Ʒ����� HalfHeightAdjust��ŭ ĸ���� ��ġ�� �������Ƿ�
        // �þ� ���� ������ ���� �������� ����.
        SpringArmComponent->TargetOffset.Z += HalfHeightAdjust;
    }
}

void ANLPlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

    bIsCapsuleShrinked = false;

    if (!IsCrouchInterpolatableCharacter())
    {
        return;
    }

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

void ANLPlayerCharacter::OnViewportResized(FViewport* InViewport, uint32 arg)
{
    /**
    * [PIE]
    * �������� ����Ʈ�� �ִ� ��� �ϳ��� ����Ʈ�� ũ�Ⱑ ����Ǵ��� ��� ����Ʈ���� ���� �� �Լ��� ȣ���.
    * ���� ũ�Ⱑ ����� ����Ʈ�� �����ؾ���.
    * ���� �ν��Ͻ��� �� ���Ӹ��� �����ǹǷ�, ����Ʈ �ϳ��� �ϳ��� ���� �ν��Ͻ��� ������.
    * ���� �ν��Ͻ��� UGameViewportClient ��ü�� �������ְ�, �̰����� �� ���� �ν��Ͻ��� ����ϴ� ����Ʈ�� �������� ���а�����.
    */
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UGameViewportClient* ViewportClient = GI->GetGameViewportClient())
        {
            FViewport* Viewport = ViewportClient->Viewport;
            if (Viewport && Viewport == InViewport)
            {
                ArmMesh->UpdateFOV();
                ViewWeaponMesh->UpdateFOV();
            }
        }
    }
}

void ANLPlayerCharacter::Server_InvokeLookPitchReplication()
{
    if (HasAuthority())
    {
        float NewPitch = GetControlRotation().Pitch;
        if (NewPitch > 180.f)
        {
            NewPitch -= 360.f;
        }
        LookPitch = NewPitch;
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

bool ANLPlayerCharacter::IsCrouchInterpolatableCharacter()
{
    if (GetNetMode() == NM_Standalone)
    {
        return true;
    }
    if (GetNetMode() == NM_Client && GetLocalRole() == ROLE_AutonomousProxy)
    {
        return true;
    }
    if (GetNetMode() == NM_ListenServer && IsListenServerControlledCharacter())
    {
        return true;
    }
    return false;
}

void ANLPlayerCharacter::InterpolateCrouch(float DeltaSeconds)
{
    if (!bIsInterpolatingCrouch)
    {
        return;
    }

    if (!IsCrouchInterpolatableCharacter())
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
            if (GetLocalRole() == ROLE_AutonomousProxy && GetNetMode() == NM_Client)
            {
                Server_CapsuleShrinked(true);
            }
        }
    }
}

void ANLPlayerCharacter::OnGameplayTagCountChanged(const FGameplayTag Tag, int32 TagCount)
{
    if (Tag.MatchesTagExact(Ability_Weapon_Secondary))
    {
        OnADS(TagCount > 0);
    }
}

void ANLPlayerCharacter::OnADS(bool bInIsADS)
{
    if (bInIsADS == bIsADS)
    {
        return;
    }

    bIsADS = bInIsADS;

    /**
    * ADS�� �ϸ� ������ �°� ī�޶��� FOV�� ����
    * ����� ������ �°� ������ ������ ����. ī�޶��� FOV���� ���� ���� ����.
    */
    float CameraTargetFOV = CameraComponent->GetBaseFOV();
    float ViewMeshTargetFOV = ArmMesh->DefaultHFOV;
    float LookSensitivityMultiplier = 1.f;

    if (bIsADS && FOV_Data)
    {
        FName FOVRowName = FName("Default");

        FGameplayTag FOVTag = NLCharacterComponent->GetCurrentWeaponADSFOVTag();
        if (FOVTag.IsValid())
        {
            FOVRowName = FOVTag.GetTagName();
        }

        if (FFOVModifyValue* DataRow = FOV_Data->FindRow<FFOVModifyValue>(FOVRowName, FString()))
        {
            CameraTargetFOV = CameraComponent->GetBaseFOV() * DataRow->CameraFOVMultiplier;
            ViewMeshTargetFOV = DataRow->ViewModelHorizontalFOV;
            LookSensitivityMultiplier = DataRow->LookSensitivityMultiplier;
        }
    }

    CameraComponent->SetTargetFOV(CameraTargetFOV);
    ArmMesh->SetTargetHFOV(ViewMeshTargetFOV);
    ViewWeaponMesh->SetTargetHFOV(ViewMeshTargetFOV);

    if (GetNLPC())
    {
        GetNLPC()->SetLookSensitivity(GetNLPC()->GetBaseLookSensitivity() * LookSensitivityMultiplier);
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

bool ANLPlayerCharacter::IsListenServerControlledCharacter()
{
    return GetNLPC() && GetNLPC()->IsListenServerController();
}

ANLPlayerController* ANLPlayerCharacter::GetNLPC()
{
    if (!NLPlayerController)
    {
        NLPlayerController = Cast<ANLPlayerController>(GetController());
    }
    return NLPlayerController;
}

void ANLPlayerCharacter::UpdateViewWeaponAndAnimLayer(USkeletalMesh* NewWeaponMesh, TSubclassOf<UAnimInstance> WeaponAnimInstanceClass, TSubclassOf<UAnimInstance> NewAnimLayerClass)
{
    if (NewWeaponMesh)
    {
        const FGameplayTag WeaponTag = NLCharacterComponent->GetCurrentWeaponTag();
        ViewWeaponMesh->SetSkeletalMesh(NewWeaponMesh, WeaponTag);

        // temp
        ViewWeaponMesh->HideBoneByName(FName("gun_sight_attach"), EPhysBodyOp::PBO_None);
        ViewWeaponMesh->HideBoneByName(FName("gun_muzzle_attach"), EPhysBodyOp::PBO_None);
    }
    if (WeaponAnimInstanceClass)
    {
        ViewWeaponMesh->SetAnimInstanceClass(WeaponAnimInstanceClass);
    }
    if (NewAnimLayerClass)
    {
        ArmMesh->LinkAnimClassLayers(NewAnimLayerClass);
    }
}

float ANLPlayerCharacter::PlayArmsAnimMontage(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
    UAnimInstance* AnimInstance = (ArmMesh) ? ArmMesh->GetAnimInstance() : nullptr;
    if (AnimMontage && AnimInstance)
    {
        float const Duration = AnimInstance->Montage_Play(AnimMontage, InPlayRate);

        if (Duration > 0.f)
        {
            // Start at a given Section.
            if (StartSectionName != NAME_None)
            {
                AnimInstance->Montage_JumpToSection(StartSectionName, AnimMontage);
            }
            return Duration;
        }
    }

    return 0.f;
}

void ANLPlayerCharacter::StopArmsAnimMontage(UAnimMontage* AnimMontage)
{
    UAnimInstance* AnimInstance = (ArmMesh) ? ArmMesh->GetAnimInstance() : nullptr;
    UAnimMontage* MontageToStop = (AnimMontage) ? AnimMontage : GetCurrentArmsMontage();
    bool bShouldStopMontage = AnimInstance && MontageToStop && !AnimInstance->Montage_GetIsStopped(MontageToStop);

    if (bShouldStopMontage)
    {
        AnimInstance->Montage_Stop(MontageToStop->BlendOut.GetBlendTime(), MontageToStop);
    }
}

UAnimMontage* ANLPlayerCharacter::GetCurrentArmsMontage() const
{
    UAnimInstance* AnimInstance = (ArmMesh) ? ArmMesh->GetAnimInstance() : nullptr;
    if (AnimInstance)
    {
        return AnimInstance->GetCurrentActiveMontage();
    }

    return nullptr;
}

float ANLPlayerCharacter::PlayWeaponAnimMontage(UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
    UAnimInstance* AnimInstance = (ViewWeaponMesh) ? ViewWeaponMesh->GetAnimInstance() : nullptr;
    if (AnimMontage && AnimInstance)
    {
        float const Duration = AnimInstance->Montage_Play(AnimMontage, InPlayRate);

        if (Duration > 0.f)
        {
            // Start at a given Section.
            if (StartSectionName != NAME_None)
            {
                AnimInstance->Montage_JumpToSection(StartSectionName, AnimMontage);
            }

            return Duration;
        }
    }

    return 0.f;
}

void ANLPlayerCharacter::StopWeaponAnimMontage(UAnimMontage* AnimMontage)
{
    UAnimInstance* AnimInstance = (ViewWeaponMesh) ? ViewWeaponMesh->GetAnimInstance() : nullptr;
    UAnimMontage* MontageToStop = (AnimMontage) ? AnimMontage : GetCurrentWeaponMontage();
    bool bShouldStopMontage = AnimInstance && MontageToStop && !AnimInstance->Montage_GetIsStopped(MontageToStop);

    if (bShouldStopMontage)
    {
        AnimInstance->Montage_Stop(MontageToStop->BlendOut.GetBlendTime(), MontageToStop);
    }
}

UAnimMontage* ANLPlayerCharacter::GetCurrentWeaponMontage() const
{
    UAnimInstance* AnimInstance = (ViewWeaponMesh) ? ViewWeaponMesh->GetAnimInstance() : nullptr;
    if (AnimInstance)
    {
        return AnimInstance->GetCurrentActiveMontage();
    }

    return nullptr;
}
