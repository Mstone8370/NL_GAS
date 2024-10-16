// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NLPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "Input/NLEnhancedInputComponent.h"
#include "NLGameplayTags.h"
#include "AbilitySystem/NLAbilitySystemComponent.h"
#include "Player/NLPlayerState.h"
#include "Characters/NLPlayerCharacter.h"
#include "HUD/NLHUD.h"
#include "Interface/CombatInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Data/AimPunchData.h"
#include "Actors/DeathCam.h"
#include "NLFunctionLibrary.h"
#include "Actors/Abstract/Interactable.h"
#include "Net/UnrealNetwork.h"

void ANLPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (GetNetMode() == NM_ListenServer && HasAuthority())
    {
        bIsListenServerController = true;
    }

    // Setup LookSensitivity
    CurrentLookSensitivity = LookSensitivity;

    // Add IMCs
    AddIMC(DefaultIMC);

    // Bind Actions
    UNLEnhancedInputComponent* NLInputComponent = Cast<UNLEnhancedInputComponent>(InputComponent);

    checkf(NLInputComponent, TEXT("EnhancedInputComponent is not NLEnhancedInputComponent"));
    checkf(!InputConfig.IsNull(), TEXT("[NLPlayerController] InputConfig is not setted"));

    UInputConfig* IC = InputConfig.LoadSynchronous();
    NLInputComponent->BindActionByTag(IC, Input_Default_Move, ETriggerEvent::Triggered, this, &ANLPlayerController::Move);
    NLInputComponent->BindActionByTag(IC, Input_Default_Look, ETriggerEvent::Triggered, this, &ANLPlayerController::Look);
    NLInputComponent->BindActionByTag(IC, Input_Default_Jump, ETriggerEvent::Triggered, this, &ANLPlayerController::Jump);
    NLInputComponent->BindActionByTag(IC, Input_Default_CrouchHold, ETriggerEvent::Triggered, this, &ANLPlayerController::Crouch);
    NLInputComponent->BindActionByTag(IC, Input_Default_CrouchHold, ETriggerEvent::Completed, this, &ANLPlayerController::UnCrouch);
    NLInputComponent->BindActionByTag(IC, Input_Default_CrouchToggle, ETriggerEvent::Triggered, this, &ANLPlayerController::CrouchToggle);
    NLInputComponent->BindActionByTag(IC, Input_Default_Interaction, ETriggerEvent::Triggered, this, &ANLPlayerController::OnInteractionHoldTriggered);
    NLInputComponent->BindActionByTag(IC, Input_Default_Interaction, ETriggerEvent::Started, this, &ANLPlayerController::BeginInteraction);
    NLInputComponent->BindActionByTag(IC, Input_Default_Interaction, ETriggerEvent::Canceled, this, &ANLPlayerController::EndInteraction);
    NLInputComponent->BindActionByTag(IC, Input_DeathCam_Respawn, ETriggerEvent::Triggered, this, &ANLPlayerController::Respawn);

    NLInputComponent->BindAbilityActions(
        IC,
        this,
        &ANLPlayerController::AbilityInputTagPressed,
        &ANLPlayerController::AbilityInputTagReleased
    );
}

void ANLPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
    Super::PostProcessInput(DeltaTime, bGamePaused);

    if (!InputEnabled() || IsActionDisabled())
    {
        if (GetNLASC())
        {
            GetNLASC()->ClearAbilityInput();
        }
        if (!InputEnabled())
        {
            RotationInput = FRotator::ZeroRotator;
        }
        return;
    }

    if (GetNLASC())
    {
        if (bIsInteracting)
        {
            GetNLASC()->ClearAbilityInput();
        }
        else
        {
            GetNLASC()->ProcessAbilityInput(DeltaTime, bGamePaused);
        }
    }

    if (GetNLPlayerCharacter())
    {
        MoveInputDirection.Normalize();
        if (MoveInputDirection.SquaredLength() > 0.25f && MoveInputDirection.X > 0.7f)
        {
            GetNLPlayerCharacter()->Sprint();
        }
        else
        {
            GetNLPlayerCharacter()->StopSprint();
        }
    }
    MoveInputDirection = FVector::ZeroVector;
}

void ANLPlayerController::AddIMC(TArray<TSoftObjectPtr<UInputMappingContext>>& IMCs)
{
    if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
    {
        if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            for (const TSoftObjectPtr<UInputMappingContext>& IMC : DefaultIMC)
            {
                AddIMC(IMC, InputSystem);
            }
        }
    }
}

void ANLPlayerController::AddIMC(TSoftObjectPtr<UInputMappingContext> IMC, UEnhancedInputLocalPlayerSubsystem* InputSystem)
{
    UInputMappingContext* IMCObject = IMC.LoadSynchronous();
    if (!IMCObject)
    {
        return;
    }

    if (!InputSystem)
    {
        if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
        {
            InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
        }
    }
    if (!InputSystem)
    {
        return;
    }

    InputSystem->AddMappingContext(IMCObject, 0);
}

void ANLPlayerController::RemoveIMC(TArray<TSoftObjectPtr<UInputMappingContext>>& IMCs)
{
    if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
    {
        if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            for (const TSoftObjectPtr<UInputMappingContext>& IMC : DefaultIMC)
            {
                RemoveIMC(IMC, InputSystem);
            }
        }
    }
}

void ANLPlayerController::RemoveIMC(TSoftObjectPtr<UInputMappingContext> IMC, UEnhancedInputLocalPlayerSubsystem* InputSystem)
{
    UInputMappingContext* IMCObject = IMC.LoadSynchronous();
    if (!IMCObject)
    {
        return;
    }

    if (!InputSystem)
    {
        if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
        {
            InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
        }
    }
    if (!InputSystem)
    {
        return;
    }

    InputSystem->RemoveMappingContext(IMCObject);
}

void ANLPlayerController::Move(const FInputActionValue& Value)
{
    if (!InputEnabled() || IsActionDisabled() || !IsValid(GetPawn()))
    {
        return;
    }

    const FVector VectorValue = Value.Get<FVector>();
    MoveInputDirection += VectorValue;
    const FRotator MovementRotation = FRotator(0.f, GetControlRotation().Yaw, 0.f);
    const FVector ForwardDirection = MovementRotation.Vector();
    const FVector MoveDirection = ForwardDirection.RotateAngleAxis(VectorValue.Rotation().Yaw, FVector::UpVector);

    GetPawn()->AddMovementInput(MoveDirection);
}

void ANLPlayerController::Look(const FInputActionValue& Value)
{
    FVector2D VectorValue = Value.Get<FVector2D>();
    AddYawInput(VectorValue.X * CurrentLookSensitivity);
    AddPitchInput(-VectorValue.Y * CurrentLookSensitivity);
}

void ANLPlayerController::Jump()
{
    if (!InputEnabled() || IsActionDisabled() || !IsValid(GetCharacter()))
    {
        return;
    }

    GetCharacter()->Jump();
}

void ANLPlayerController::Crouch()
{
    if (!InputEnabled() || IsActionDisabled() || !IsValid(GetCharacter()))
    {
        return;
    }

    GetCharacter()->Crouch();
}

void ANLPlayerController::UnCrouch()
{
    if (!InputEnabled() || IsActionDisabled() || !IsValid(GetCharacter()))
    {
        return;
    }

    GetCharacter()->UnCrouch();
}

void ANLPlayerController::CrouchToggle()
{
    if (!InputEnabled() || IsActionDisabled() || !IsValid(GetCharacter()))
    {
        return;
    }

    if (GetCharacter()->bIsCrouched)
    {
        GetCharacter()->UnCrouch();
    }
    else
    {
        GetCharacter()->Crouch();
    }
}

void ANLPlayerController::Interaction()
{
    if (!bIsInteracting || !IsValid(InteractableActor) || !InputEnabled() || IsActionDisabled() || !IsValid(GetCharacter()))
    {
        return;
    }

    bIsInteracting = false;

    Server_Interaction(InteractableActor);
}

void ANLPlayerController::OnInteractionHoldTriggered()
{
    Interaction();
}

void ANLPlayerController::BeginInteraction()
{
    if (!IsValid(InteractableActor) || !InputEnabled() || IsActionDisabled() || !IsValid(GetCharacter()))
    {
        bIsInteracting = false;
        return;
    }

    bIsInteracting = true;

    bool bShouldHoldKeyPress = InteractableActor->ShouldHoldKeyPress();
    if (!bShouldHoldKeyPress 
        && InteractableActor->GetInteractionType().MatchesTag(Interaction_Pickup_Weapon)
        && GetNLPlayerCharacter())
    {
        // 무기를 교체해야하는 경우에는 키 홀드
        bShouldHoldKeyPress |= GetNLPlayerCharacter()->IsWeaponSlotFull();
    }

    if (bShouldHoldKeyPress)
    {
        OnInteractionBegin.Broadcast();
    }
    else
    {
        Interaction();
    }
}

void ANLPlayerController::EndInteraction()
{
    if (!IsValid(InteractableActor) || !InputEnabled() || IsActionDisabled() || !IsValid(GetCharacter()))
    {
        return;
    }

    bIsInteracting = false;

    OnInteractionEnd.Broadcast();
}

void ANLPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
    if (GetNLASC())
    {
        GetNLASC()->AbilityInputTagPressed(InputTag);
    }
}

void ANLPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
    if (GetNLASC())
    {
        GetNLASC()->AbilityInputTagReleased(InputTag);
    }
}

void ANLPlayerController::Respawn()
{
    Server_RespawnRequested(this);
}

UNLAbilitySystemComponent* ANLPlayerController::GetNLASC()
{
    if (!LNAbilitySystemComponent)
    {
        LNAbilitySystemComponent = Cast<UNLAbilitySystemComponent>(
            UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>())
        );
    }
    return LNAbilitySystemComponent;
}

ANLPlayerState* ANLPlayerController::GetNLPS()
{
    if (!NLPlayerState)
    {
        NLPlayerState = Cast<ANLPlayerState>(GetPlayerState<ANLPlayerState>());
    }
    return NLPlayerState;
}

ANLPlayerCharacter* ANLPlayerController::GetNLPlayerCharacter()
{
    return Cast<ANLPlayerCharacter>(GetCharacter());
}

ANLHUD* ANLPlayerController::GetNLHUD()
{
    if (!NLHUD)
    {
        NLHUD = Cast<ANLHUD>(GetHUD());
    }
    return NLHUD;
}

void ANLPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(ANLPlayerController, bIsActionDisabled, COND_None, REPNOTIFY_OnChanged);
}

void ANLPlayerController::InitHUD(APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS, UNLCharacterComponent* NLC)
{
    if (GetNLHUD())
    {
        GetNLHUD()->Initialize(this, PS, ASC, AS, NLC);
    }
}

void ANLPlayerController::Server_Interaction_Implementation(AInteractable* Interactable)
{
    if (!IsValid(Interactable) || !Interactable->CanInteract() || !GetNLPlayerCharacter())
    {
        return;
    }

    const FGameplayTag& InteractionType = Interactable->GetInteractionType();
    if (InteractionType.MatchesTag(Interaction_Pickup))
    {
        GetNLPlayerCharacter()->PickUp(Interactable);
    }
    else if (InteractionType.MatchesTag(Interaction_Button))
    {
        // TODO:
        Interactable->StartInteraction(GetPawn());
    }
    else
    {
        // Default Interaction
        Interactable->StartInteraction(GetPawn());
    }
}

void ANLPlayerController::Client_ShowDamageCauseIndicator_Implementation(float InDamage, bool bInIsCriticalHit, AActor* DamagedActor)
{
    if (GetNLHUD())
    {
        GetNLHUD()->ShowDamageCauseIndicator(InDamage, bInIsCriticalHit, DamagedActor);
    }

    if (DamagedActor->Implements<UCombatInterface>())
    {
        ICombatInterface::Execute_ShowDamageText(DamagedActor, InDamage, bInIsCriticalHit);
    }
}

void ANLPlayerController::Client_TakenDamage_Implementation(FVector DamageOrigin, FVector HitDirection, bool bIsCriticalHit, FGameplayTag DamageType)
{
    OnTakenDamageDelegate.Broadcast(DamageOrigin);

    if (HitDirection.SquaredLength() > 0.f)
    {
        if (const FTaggedAimPunch* AimPunch = AimPunchData->GetAimPunchData(DamageType))
        {
            if (GetPawn()->Implements<UCombatInterface>())
            {
                ICombatInterface::Execute_AddAimPunch(GetPawn(), *AimPunch, HitDirection, bIsCriticalHit);
            }
        }
    }
}

void ANLPlayerController::OnRespawnableState()
{
    Client_OnRespawnableState();
}

void ANLPlayerController::Client_OnRespawnableState_Implementation()
{
    AddIMC(DeathIMC);

    OnRespawnable.Broadcast();
}

void ANLPlayerController::Client_OnKilled_Implementation(AActor* TargetActor)
{
    OnKill.Broadcast(TargetActor);
}

void ANLPlayerController::SetupDeathCam(AActor* TargetActor)
{
    if (IsLocalController())
    {
        if (ADeathCam* DeathCam = GetWorld()->SpawnActor<ADeathCam>(GetPawn()->GetActorLocation(), GetPawn()->GetActorRotation()))
        {
            DeathCam->SetTargetActor(TargetActor);
            SetViewTargetWithBlend(DeathCam, 0.2f);
        }
    }
}

void ANLPlayerController::ClearDeathCam()
{
    if (IsLocalController())
    {
        AActor* ViewTargetActor = GetViewTarget();
        if (ViewTargetActor && ViewTargetActor->IsA<ADeathCam>())
        {
            SetViewTarget(GetPawn());
            ViewTargetActor->Destroy();
        }
    }
}

void ANLPlayerController::Client_SpawnParticles_Implementation(const FGameplayTag& ParticleTag, const TArray<FParticleSpawnInfo>& SpawnInfos)
{
    UNLFunctionLibrary::SpawnMultipleParticleByTag(this, ParticleTag, SpawnInfos);
}

void ANLPlayerController::Client_SpawnProjectiles_Implementation(const FGameplayTag& ProjectileTag, const TArray<FProjectileSpawnInfo>& SpawnInfos)
{
    // 이 플레이어 컨트롤러 입장에선 시뮬레이티드 프록시가 발사한 발사체임.
    TArray<ANLProjectile*> DummyArray;
    UNLFunctionLibrary::SpawnMultipleProjectileByTag(this, ProjectileTag, SpawnInfos, nullptr, DummyArray);
}

void ANLPlayerController::OnRep_IsActionDisabled()
{
    if (bIsActionDisabled)
    {
        DisableAction();
    }
    else
    {
        EnableAction();
    }
}

void ANLPlayerController::SetLookSensitivity(float InLookSensitivity)
{
    CurrentLookSensitivity = InLookSensitivity;
}

void ANLPlayerController::GetPlayerAimPoint(FVector& ViewLocation, FRotator& ViewRotation) const
{
    FRotator ViewRot;
    GetPlayerViewPoint(ViewLocation, ViewRot);
    ViewRotation = GetControlRotation();
}

void ANLPlayerController::OnCausedDamage(float InDamage, bool bInIsCriticalHit, AActor* DamagedActor)
{
    Client_ShowDamageCauseIndicator(InDamage, bInIsCriticalHit, DamagedActor);

    if (ANLPlayerState* NLPS = GetNLPS())
    {
        NLPS->AddPlayerStat(Stat_Hit);
        if (bInIsCriticalHit)
        {
            NLPS->AddPlayerStat(Stat_Hit_Critical);
        }
    }
}

void ANLPlayerController::OnTakenDamage(const FHitResult* InHitResult, FVector DamageOrigin, bool bIsCriticalHit, const FGameplayTag& DamageType)
{
    FVector HitDirection = FVector::ZeroVector;
    if (InHitResult)
    {
        HitDirection = (InHitResult->TraceEnd - InHitResult->TraceStart).GetSafeNormal();
    }

    Client_TakenDamage(DamageOrigin, HitDirection, bIsCriticalHit, DamageType);
}

void ANLPlayerController::OnKilled(AActor* TargetActor)
{
    Client_OnKilled(TargetActor);

    if (ANLPlayerState* NLPS = GetNLPS())
    {
        NLPS->AddPlayerStat(Stat_Kill);
    }
}

void ANLPlayerController::Server_RespawnRequested_Implementation(APlayerController* PC)
{
    OnRequestRespawn.ExecuteIfBound(this, false);
}

void ANLPlayerController::Client_OnRespawned_Implementation()
{
    RemoveIMC(DeathIMC);
    AddIMC(DefaultIMC);

    if (GetNLHUD())
    {
        GetNLHUD()->OnCharacterRespawn();
    }

    ClearDeathCam();

    OnPlayerRespawn.Broadcast();
}

void ANLPlayerController::OnPlayerDeath(AActor* SourceActor, FGameplayTag DamageType)
{
    RemoveIMC(DefaultIMC);

    if (GetNLHUD())
    {
        GetNLHUD()->OnCharacterDead();
    }

    SetupDeathCam(SourceActor);

    OnPlayerDeathDelegate.Broadcast(SourceActor, GetPawn(), DamageType);

    if (ANLPlayerState* NLPS = GetNLPS())
    {
        NLPS->AddPlayerStat(Stat_Death);
    }
}

void ANLPlayerController::AddKillLog_Implementation(APlayerState* SourcePS, APlayerState* TargetPS, FGameplayTag DamageType)
{
    OnReceivedKillLog.Broadcast(SourcePS, TargetPS, DamageType);
}

void ANLPlayerController::SetRespawnTime(float RespawnTime)
{
    GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ANLPlayerController::OnRespawnableState, RespawnTime, false);
}

void ANLPlayerController::OnRespawned(FVector Direction)
{
    if (GetNLPlayerCharacter())
    {
        GetNLPlayerCharacter()->OnRespawned();
    }
    
    ClientSetRotation(Direction.Rotation());
    Client_OnRespawned();
}

void ANLPlayerController::OnResetted(FVector Direction)
{
    if (GetNLPlayerCharacter())
    {
        GetNLPlayerCharacter()->OnResetted();
    }

    ClientSetRotation(Direction.Rotation());
}

void ANLPlayerController::EnableInteraction(AInteractable* Interactable, FString Message)
{
    if (!IsValid(Interactable) || !Interactable->CanInteract() || (IsValid(InteractableActor) && InteractableActor == Interactable))
    {
        return;
    }

    if (IsValid(InteractableActor))
    {
        InteractableActor->Unfocused();
    }
    InteractableActor = Interactable;
    InteractableActor->Focused();

    OnInteractionEnabled.Broadcast(Interactable, Message);
}

void ANLPlayerController::DisableInteraction()
{
    if (IsValid(InteractableActor))
    {
        InteractableActor->Unfocused();
        InteractableActor = nullptr;
    }

    bIsInteracting = false;

    OnInteractionDisabled.Broadcast();
}

void ANLPlayerController::ReplicateParticlesToClient(const FGameplayTag& ParticleTag, const TArray<FParticleSpawnInfo>& SpawnInfos)
{
    Client_SpawnParticles(ParticleTag, SpawnInfos);
}

void ANLPlayerController::ReplicateProjectilesToClient(const FGameplayTag& ProjectileTag, const TArray<FProjectileSpawnInfo>& SpawnInfos)
{
    Client_SpawnProjectiles(ProjectileTag, SpawnInfos);
}

void ANLPlayerController::DisableAction()
{
    bIsActionDisabled = true;

    if (GetCharacter())
    {
        GetCharacter()->UnCrouch();
    }
}

void ANLPlayerController::EnableAction()
{
    bIsActionDisabled = false;
}
