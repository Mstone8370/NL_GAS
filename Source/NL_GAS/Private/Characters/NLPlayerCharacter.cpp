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
#include "Data/AimPunchData.h"

ANLPlayerCharacter::ANLPlayerCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UNLCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))  // CharacterMovementComponent의 클래스를 NLCharacterMovementComponent로 변경
    , LookPitchRepTime(0.02f)
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
    GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

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
    DOREPLIFETIME_CONDITION_NOTIFY(ANLPlayerCharacter, bIsSprinting, COND_SimulatedOnly, REPNOTIFY_OnChanged);
    DOREPLIFETIME_CONDITION_NOTIFY(ANLPlayerCharacter, LedgeClimbData, COND_SimulatedOnly, REPNOTIFY_OnChanged);
}

void ANLPlayerCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    ArmMesh->Initialize();
    ViewWeaponMesh->Initialize();

    NLCharacterMovementComponent = Cast<UNLCharacterMovementComponent>(GetCharacterMovement());
    check(NLCharacterMovementComponent);
}

void ANLPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        // 서버에서는 캐릭터 메시의 본을 항상 업데이트
        GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    }

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

    if (ControlShakeManager && LoopingShakeCurve_Idle)
    {
        ControlShakeManager->AddShake(-1.f, LoopingShakeCurve_Idle, FRotator(1.f, 1.f, 0.f), true);
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
    check(GetNLASC());
    AbilitySystemComponent->InitAbilityActorInfo(PS, this);

    AttributeSet = PS->GetAttributeSet();
    if (HasAuthority())
    {
        InitDefaultAttribute();
    }

    AbilitySystemComponent->RegisterGameplayTagEvent(Ability_Weapon_Secondary, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ANLPlayerCharacter::OnGameplayTagCountChanged);
    AbilitySystemComponent->RegisterGameplayTagEvent(Ability_Block_Sprint, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ANLPlayerCharacter::OnGameplayTagCountChanged);
}

void ANLPlayerCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    InterpolateCrouch(DeltaSeconds);
    TiltCamera(DeltaSeconds);
}

void ANLPlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    // On Server
    InitAbilityActorInfo();
    
    TryInitializeHUD();
    
    AddStartupAbilities();
    
    if (GetNetMode() == NM_ListenServer || GetNetMode() == NM_Standalone)
    {
        NLCharacterComponent->AddStartupWeapons();
        NLCharacterComponent->ValidateStartupWeapons();
    }
}

void ANLPlayerCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    // On Client

    InitAbilityActorInfo();

    NLCharacterComponent->ValidateStartupWeapons();

    TryInitializeHUD();
    TryRequestStartupWeapons();
}

void ANLPlayerCharacter::OnRep_Controller()
{
    Super::OnRep_Controller();

    // On Client

    TryInitializeHUD();
    TryRequestStartupWeapons();
}

void ANLPlayerCharacter::TryInitializeHUD()
{
    if (GetNLPC() && GetPlayerState())
    {
        if (ANLHUD* HUD = Cast<ANLHUD>(GetNLPC()->GetHUD()))
        {
            HUD->Initialize(
                GetNLPC(),
                GetPlayerState(),
                AbilitySystemComponent,
                AttributeSet,
                NLCharacterComponent
            );
        }
    }
}

void ANLPlayerCharacter::TryRequestStartupWeapons()
{
    if (GetNLPC() && GetPlayerState() && !bRequestedStartupWeapons)
    {
        Server_RequestStartupWeapons();
        bRequestedStartupWeapons = true;
    }
}

void ANLPlayerCharacter::Server_RequestStartupWeapons_Implementation()
{
    NLCharacterComponent->AddStartupWeapons();
    NLCharacterComponent->ValidateStartupWeapons();
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
    return !bIsWeaponLowered && NLCharacterComponent->CanAttack();
}

void ANLPlayerCharacter::AddAimPunch_Implementation(const FTaggedAimPunch& AimPunchData, FVector HitDirection, bool bIsCriticalHit)
{
    FRotator ShakeMagnitude(-1.f, 0.f, 0.f);
    const FVector ForwardDirection = GetActorForwardVector();
    const FVector RightDirection = GetActorRightVector();

    double Pitch = FVector::DotProduct(ForwardDirection, HitDirection);
    double Yaw = FVector::DotProduct(RightDirection, HitDirection);

    float Magnitude = bIsCriticalHit ? AimPunchData.CriticalMagnitude : AimPunchData.BaseMagnitude;
    FVector2D Normalized = FVector2D(Pitch, Yaw).GetSafeNormal();
    ShakeMagnitude = FRotator(Normalized.X * Magnitude, Normalized.Y * Magnitude, 0.f);

    ControlShakeManager->AddShake(AimPunchData.Duration, AimPunchData.ControlShakeCurve, ShakeMagnitude);
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

    if (IsCrouchInterpolatableCharacter())
    {
        if (GetCharacterMovement()->IsWalking())
        {
            /**
            * 캡슐이 줄어들 때에는 캡슐의 상단부와 하단부 모두 Delta만큼 크기가 줄어듬.
            * 그런데 Walking일때 앉기를 하면 캡슐이 줄어들때 바닥에 닿아있는 상태를 유지하기위해
            * 캡슐의 중심 위치를 Delta만큼 아래로 옮기게 됨.
            * 따라서 Delta의 두배 차이나게 설정해야 최종 Target Offset의 위치로 Interpolation 함.
            */
            TargetSpringArmOffset = BaseSpringArmOffset - 2 * GetCrouchedHalfHeightDelta();
        }
        else
        {
            /**
            * 하지만 공중에 있을 때에는 캡슐의 중심 위치가 옮겨지지 않으므로 Delta만큼만 차이나게 함.
            */
            TargetSpringArmOffset = BaseSpringArmOffset - GetCrouchedHalfHeightDelta();
        }
    }
    
    // Start interpolate
    bIsInterpolatingCrouch = true;
}

void ANLPlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

    // Same with On Capsule shrinked

    bIsCapsuleShrinked = true;

    if (!IsCrouchInterpolatableCharacter())
    {
        return;
    }

    if (GetCharacterMovement()->IsWalking())
    {
        /**
        * Walking일때 캡슐이 줄어들어야 하는 상황이라면, Crouch Interpolation의 Target Spring Arm Offset이
        * Base Spring Arm Offset과 Delta의 두배 차이나게 설정되어있는 상태임.
        * 이는 캡슐이 줄어들면 캡슐의 상단부와 하단부 양쪽에서 모두 Delta만큼 캡슐이 줄어들며,
        * 공중에 있을때에는 그저 줄어들기만 하고 중심은 유지하면 되지만, 바닥에 닿아있는 Walking일땐
        * 바닥에 닿은 상태를 유지하기 위해 중심을 Delta만큼 아래로 이동하기 때문에 발생하는 차이임.
        * 따라서 Crouch Interpolation의 부드러운 연결을 위해 Target Spring Arm Offset을
        * Delta의 두배만큼 차이나게 설정했다가 캡슐이 줄어들어 중심이 Delta만큼 아래로 이동했다면
        * 그 Delta만큼 Offset을 올려줘야 중심이 이동하기 전 위치를 그대로 유지하게 됨.
        */
        SpringArmComponent->TargetOffset.Z += HalfHeightAdjust;

        if (bIsInterpolatingCrouch)
        {
            /**
            * 만약 아직 Target Spring Arm Offset만큼 Interpolation이 되지 않았는데 캡슐이 줄어들었다면
            * 캡슐이 줄어든 상황에 맞는 Target Spring Arm Offset 값으로 설정해줌.
            * 이런 상황이 발생하는 예시로는 슬라이딩이 있음.
            */
            TargetSpringArmOffset = BaseSpringArmOffset - GetCrouchedHalfHeightDelta();
        }
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

    if (!IsCrouchInterpolatableCharacter())
    {
        return;
    }

    TargetSpringArmOffset = BaseSpringArmOffset;
    
    if (GetCharacterMovement()->IsWalking())
    {
        /**
        * Walking일때 앉기를 끝내서 캡슐의 크기가 늘어났다면, 캡슐 하단부의 늘어난 Delta만큼 중심의 위치로 위로 올라가게 됨.
        * 그렇다는건 월드 관점에서는 Spring Arm의 Target Offset의 Teleportation이 발생하게되는 상황이므로
        * Target Offset을 Delta만큼 낮추어서 월드 위치를 유지하게 하고, 자연스러운 Interpolation이 가능하게 됨.
        */
        SpringArmComponent->TargetOffset.Z -= HalfHeightAdjust;
    }

    bIsInterpolatingCrouch = true;

    if (GetLocalRole() == ROLE_AutonomousProxy && GetNetMode() == NM_Client)
    {
        Server_CapsuleShrinked(false);
    }
}

void ANLPlayerCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    if (bIsCrouched && !bIsCapsuleShrinked)
    {
        /**
        * 캡슐이 줄어들지 않은 상태면 Crouch Interpolating 상태며, 슬라이딩 상태가 아니고,
        * Falling일때 앉기 시작했다는 의미일 수 있으니
        * Target Spring Arm Offset이 Base Spring Arm Offset과 Delta만큼의 차이만 설정되어 있을 수 있음.
        * Landed 상태와 엮이면 Delta의 두배만큼 차이가 있어야하므로
        * Target Spring Arm Offset을 Delta의 두배로 다시 지정.
        */
        TargetSpringArmOffset = BaseSpringArmOffset - 2 * GetCrouchedHalfHeightDelta();
    }
}

void ANLPlayerCharacter::OnFallingStarted()
{
    if (bIsCrouched && bIsInterpolatingCrouch)
    {
        /**
        * Landed()에서와 다르게 여기에서는 Crouch Interpolating 상태인지 확인함.
        * 슬라이딩 상태에서는 캡슐은 줄었지만 Crouch Interpolating 상태고,
        * 그때의 Target Spring Arm Offset은 Base Spring Arm Offset과 Delta만큼의 차이만 있어야 함.
        * 물론 OnStartCrouch() 함수에서도 이런 경우를 고려해서 Target Spring Arm Offset을 설정하지만,
        * 혹시 모를 그 외의 경우를 위해서 여기에서 한번 더 진행하고, 논리적으로도 이해하는데 도움이 될듯 함.
        */
        TargetSpringArmOffset = BaseSpringArmOffset - GetCrouchedHalfHeightDelta();
    }
}

bool ANLPlayerCharacter::CanSprint()
{
    return !bSprintBlocked && !bIsCrouched && !bIsADS && NLCharacterMovementComponent && NLCharacterMovementComponent->IsMovingOnGround() && GetRootComponent() && !GetRootComponent()->IsSimulatingPhysics();
}

void ANLPlayerCharacter::Sprint()
{
    if (NLCharacterMovementComponent)
    {
        NLCharacterMovementComponent->bWantsToSprint = CanSprint();
    }
}

void ANLPlayerCharacter::StopSprint()
{
    if (NLCharacterMovementComponent)
    {
        NLCharacterMovementComponent->bWantsToSprint = false;
    }
}

void ANLPlayerCharacter::OnStartSprint()
{
    if (GetWorldTimerManager().IsTimerActive(SprintStopTimer))
    {
        GetWorldTimerManager().ClearTimer(SprintStopTimer);
        GetNLASC()->RemoveLooseGameplayTag(Ability_Sprint);
    }
    GetNLASC()->AddLooseGameplayTag(Ability_Sprint);
}

void ANLPlayerCharacter::OnEndSprint()
{
    FTimerDelegate Dele;
    Dele.BindLambda(
        [this]()
        {
            GetNLASC()->RemoveLooseGameplayTag(Ability_Sprint);
        }
    );
    GetWorldTimerManager().SetTimer(SprintStopTimer, Dele, .2f, false);
}

void ANLPlayerCharacter::OnStartSlide()
{
    GetWorldTimerManager().SetTimer(SlideTiltTimer, SlidingTiltInterpTime, false);

    SetTargetFOVByTag(FOV_Movement_Slide);
}

void ANLPlayerCharacter::OnEndSlide()
{
    GetWorldTimerManager().SetTimer(SlideTiltTimer, SlidingTiltInterpTime, false);

    SetTargetFOVByTag(FOV_Default, 5.f);
}

void ANLPlayerCharacter::OnRep_LedgeClimbData()
{
    if (NLCharacterMovementComponent)
    {
        if (LedgeClimbData.bIsLedgeClimbing)
        {
            NLCharacterMovementComponent->StartLedgeClimb(LedgeClimbData.TargetLocation, true);
        }
        else
        {
            NLCharacterMovementComponent->StopLedgeClimb(true);
        }
        NLCharacterMovementComponent->bNetworkUpdateReceived = true;
    }
}

void ANLPlayerCharacter::OnStartLedgeClimb(FVector TargetLocation)
{
    // On Server and Client

    const float HeightDiff = FMath::Abs(TargetLocation.Z - GetActorLocation().Z);
    if (HeightDiff >= LedgeClimbHeightThreshold)
    {
        LowerWeapon();
    }
}

void ANLPlayerCharacter::OnEndLedgeClimb()
{
    // On Server and Client

    if (bIsWeaponLowered)
    {
        RaiseWeapon();
    }
}

void ANLPlayerCharacter::OnViewportResized(FViewport* InViewport, uint32 arg)
{
    /**
    * [PIE]
    * 여러개의 뷰포트가 있는 경우 하나의 뷰포트의 크기가 변경되더라도 모든 뷰포트에서 각각 이 함수가 호출됨.
    * 따라서 크기가 변경된 뷰포트를 구분해야함.
    * 게임 인스턴스는 각 게임마다 생성되므로, 뷰포트 하나당 하나의 게임 인스턴스가 생성됨.
    * 게임 인스턴스는 UGameViewportClient 객체를 가지고있고, 이걸통해 이 게임 인스턴스가 담당하는 뷰포트가 무엇인지 구분가능함.
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

void ANLPlayerCharacter::SetTargetFOVByTag(FGameplayTag FOVTag, float TransientInterpSpeed)
{
    /**
    * GameplayTag에 따라 카메라의 FOV, 뷰모델의 HFOV, 마우스 감도를 조정
    * 뷰모델은 배율에 맞게 정해진 값으로 설정. 카메라의 FOV값에 영향 받지 않음.
    */
    float CameraTargetFOV = CameraComponent->GetBaseFOV();
    float ViewMeshTargetFOV = ArmMesh->DefaultHFOV;
    float LookSensitivityMultiplier = 1.f;
    UCurveVector* LoopingControlShakeCurve = LoopingShakeCurve_Idle;

    check(FOV_Data);

    if (!FOVTag.IsValid())
    {
        FOVTag = FOV_Default;
    }
    FName FOVRowName = FOVTag.GetTagName();

    if (FFOVModifyValue* DataRow = FOV_Data->FindRow<FFOVModifyValue>(FOVRowName, FString()))
    {
        CameraTargetFOV = CameraComponent->GetBaseFOV() * DataRow->CameraFOVMultiplier + DataRow->CameraFOVAdditive;
        ViewMeshTargetFOV = DataRow->ViewModelHorizontalFOV;
        LookSensitivityMultiplier = DataRow->LookSensitivityMultiplier;
        LoopingControlShakeCurve = DataRow->LoopingControlShakeCurve;
    }

    CameraComponent->SetTargetFOV(CameraTargetFOV, TransientInterpSpeed);
    ArmMesh->SetTargetHFOV(ViewMeshTargetFOV, TransientInterpSpeed);
    ViewWeaponMesh->SetTargetHFOV(ViewMeshTargetFOV, TransientInterpSpeed);

    if (GetNLPC())
    {
        GetNLPC()->SetLookSensitivity(GetNLPC()->GetBaseLookSensitivity() * LookSensitivityMultiplier);
    }

    if (LoopingControlShakeCurve)
    {
        ControlShakeManager->AddShake(-1.f, LoopingControlShakeCurve, FRotator(1.f, 1.f, 1.f), true);
    }
    else
    {
        ControlShakeManager->ClearLoopingShake();
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

void ANLPlayerCharacter::TiltCamera(float DeltaTime)
{
    if (GetLocalRole() == ROLE_SimulatedProxy)
    {
        return;
    }

    float TargetTiltDegree = 0.f;

    if (bIsSliding)
    {
        const FVector SlidingDirection = GetCharacterMovement()->GetLastUpdateVelocity().GetSafeNormal2D();
        TargetTiltDegree = SlidingTiltTargetDegree * SlidingDirection.Dot(GetActorRightVector()) * -1.f;
    }
    
    FRotator NewRot = FRotator(0.f, 0.f, TargetTiltDegree);
    if (GetWorldTimerManager().IsTimerActive(SlideTiltTimer))
    {
        NewRot = UKismetMathLibrary::RInterpTo(
            SpringArmComponent->GetRelativeRotation(),
            NewRot,
            DeltaTime,
            SlidingTiltInterpSpeed
        );
    }

    SpringArmComponent->SetRelativeRotation(NewRot);
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
            // 앉기는 interpolation이 시작되었다면 이 시점에서 불가능한 경우는 없으므로 추가 확인없이 진행.
            NLCharacterMovementComponent->ShrinkCapsuleHeight();
        }
    }
}

void ANLPlayerCharacter::OnGameplayTagCountChanged(const FGameplayTag Tag, int32 TagCount)
{
    if (Tag.MatchesTagExact(Ability_Block_Sprint))
    {
        bSprintBlocked = TagCount > 0;
    }
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

    FGameplayTag FOVTag = bIsADS ? NLCharacterComponent->GetCurrentWeaponADSFOVTag() : FGameplayTag();
    SetTargetFOVByTag(FOVTag);
}

void ANLPlayerCharacter::OnRep_IsSprinting()
{
    if (NLCharacterMovementComponent)
    {
        if (bIsSprinting)
        {
            NLCharacterMovementComponent->bWantsToSprint = true;
            NLCharacterMovementComponent->Sprint(true);
        }
        else
        {
            NLCharacterMovementComponent->bWantsToSprint = false;
            NLCharacterMovementComponent->StopSprint(true);
        }
        NLCharacterMovementComponent->bNetworkUpdateReceived = true;
    }
}

void ANLPlayerCharacter::OnRep_IsSliding()
{
    if (NLCharacterMovementComponent)
    {
        if (bIsSliding)
        {
            
        }
        else
        {
            
        }
        NLCharacterMovementComponent->bNetworkUpdateReceived = true;
    }
}

float ANLPlayerCharacter::GetCrouchedHalfHeightDelta()
{
    if (CrouchedHalfHeightDelta <= 0.f)
    {
        ACharacter* DefaultCharacter = GetClass()->GetDefaultObject<ACharacter>();
        const float DefaultHalfHeight = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
        const float CrouchedHalfHeight = GetCharacterMovement()->GetCrouchedHalfHeight();
        CrouchedHalfHeightDelta = FMath::Abs(DefaultHalfHeight - CrouchedHalfHeight);
    }
    return CrouchedHalfHeightDelta;
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

UNLAbilitySystemComponent* ANLPlayerCharacter::GetNLASC()
{
    if (!NLAbilitySystemComponent)
    {
        NLAbilitySystemComponent = Cast<UNLAbilitySystemComponent>(AbilitySystemComponent);
    }
    return NLAbilitySystemComponent;
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

void ANLPlayerCharacter::LowerWeapon()
{
    if (bIsWeaponLowered)
    {
        return;
    }

    bIsWeaponLowered = true;

    if (NLCharacterComponent)
    {
        NLCharacterComponent->LowerWeapon();
    }
}

void ANLPlayerCharacter::RaiseWeapon()
{
    if (!bIsWeaponLowered)
    {
        return;
    }

    bIsWeaponLowered = false;

    FTimerDelegate Dele;
    Dele.BindLambda(
        [this]()
        {
            if (NLCharacterComponent)
            {
                NLCharacterComponent->RaiseWeapon();
            }
        }
    );
    GetWorldTimerManager().SetTimer(WeaponRaiseTimer, Dele, WeaponLowerRaiseTime, false);
}

bool ANLPlayerCharacter::IsWeaponLoweredIncludeTransistion() const
{
    return bIsWeaponLowered || GetWorldTimerManager().IsTimerActive(WeaponRaiseTimer);
}

bool FLedgeClimbData::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
    uint8 Flags = 0;

    if (Ar.IsSaving())
    {
        if (bIsLedgeClimbing)
        {
            Flags |= 1 << 0;
        }
    }

    Ar.SerializeBits(&Flags, 2);

    if (Flags & (1 << 0))
    {
        bIsLedgeClimbing = true;
        Ar << TargetLocation;
    }

    bOutSuccess = true;
    return true;
}
