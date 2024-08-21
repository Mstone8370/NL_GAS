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

    if (!InputEnabled())
    {
        if (GetNLAbilitySystemComponent())
        {
            GetNLAbilitySystemComponent()->ClearAbilityInput();
        }
        RotationInput = FRotator::ZeroRotator;
        return;
    }

    if (GetNLAbilitySystemComponent())
    {
        if (bIsInteracting)
        {
            GetNLAbilitySystemComponent()->ClearAbilityInput();
        }
        else
        {
            GetNLAbilitySystemComponent()->ProcessAbilityInput(DeltaTime, bGamePaused);
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
    if (!InputEnabled())
    {
        return;
    }

    const FVector VectorValue = Value.Get<FVector>();
    MoveInputDirection += VectorValue;
    const FRotator MovementRotation = FRotator(0.f, GetControlRotation().Yaw, 0.f);
    const FVector ForwardDirection = MovementRotation.Vector();
    const FVector MoveDirection = ForwardDirection.RotateAngleAxis(VectorValue.Rotation().Yaw, FVector::UpVector);

    GetCharacter()->AddMovementInput(MoveDirection);
}

void ANLPlayerController::Look(const FInputActionValue& Value)
{
    FVector2D VectorValue = Value.Get<FVector2D>();
    AddYawInput(VectorValue.X * CurrentLookSensitivity);
    AddPitchInput(-VectorValue.Y * CurrentLookSensitivity);
}

void ANLPlayerController::Jump()
{
    if (!InputEnabled())
    {
        return;
    }

    GetCharacter()->Jump();
}

void ANLPlayerController::Crouch()
{
    if (!InputEnabled())
    {
        return;
    }

    GetCharacter()->Crouch();
}

void ANLPlayerController::UnCrouch()
{
    if (!InputEnabled())
    {
        return;
    }

    GetCharacter()->UnCrouch();
}

void ANLPlayerController::CrouchToggle()
{
    if (!InputEnabled())
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
    if (!bIsInteracting || !IsValid(InteractableActor))
    {
        return;
    }

    bIsInteracting = false;

    const FGameplayTag& InteractionType = InteractableActor->GetInteractionType();

    if (InteractionType.MatchesTag(Interaction_Pickup_Weapon))
    {
        if (GetNLPlayerCharacter())
        {
            GetNLPlayerCharacter()->Server_PickUp(InteractableActor);
        }
    }
}

void ANLPlayerController::OnInteractionHoldTriggered()
{
    Interaction();
}

void ANLPlayerController::BeginInteraction()
{
    if (!IsValid(InteractableActor))
    {
        bIsInteracting = false;
        return;
    }

    bIsInteracting = true;

    OnInteractionBegin.Broadcast();
}

void ANLPlayerController::EndInteraction()
{
    if (!IsValid(InteractableActor))
    {
        return;
    }

    bIsInteracting = false;

    OnInteractionEnd.Broadcast();
}

void ANLPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
    if (GetNLAbilitySystemComponent())
    {
        GetNLAbilitySystemComponent()->AbilityInputTagPressed(InputTag);
    }
}

void ANLPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
    if (GetNLAbilitySystemComponent())
    {
        GetNLAbilitySystemComponent()->AbilityInputTagReleased(InputTag);
    }
}

void ANLPlayerController::Respawn()
{
    UE_LOG(LogTemp, Warning, TEXT("Respawn requested"));
    Server_RespawnRequested(this);
}

UNLAbilitySystemComponent* ANLPlayerController::GetNLAbilitySystemComponent()
{
    if (!LNAbilitySystemComponent)
    {
        LNAbilitySystemComponent = Cast<UNLAbilitySystemComponent>(
            UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>())
        );
    }
    return LNAbilitySystemComponent;
}

ANLPlayerState* ANLPlayerController::GetNLPlayerState()
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

void ANLPlayerController::InitHUD(APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS, UNLCharacterComponent* NLC)
{
    if (GetNLHUD())
    {
        GetNLHUD()->Initialize(this, PS, ASC, AS, NLC);
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
    UNLFunctionLibrary::SpawnMultipleProjectileByTag(this, ProjectileTag, SpawnInfos, DummyArray);
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
}

void ANLPlayerController::Server_RespawnRequested_Implementation(APlayerController* PC)
{
    OnRequestRespawn.ExecuteIfBound(this);
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

void ANLPlayerController::OnDead(AActor* SourceActor, FGameplayTag DamageType)
{
    RemoveIMC(DefaultIMC);

    if (GetNLHUD())
    {
        GetNLHUD()->OnCharacterDead();
    }

    SetupDeathCam(SourceActor);

    OnPlayerDeath.Broadcast(SourceActor, GetPawn(), DamageType);
}

void ANLPlayerController::AddKillLog_Implementation(AActor* SourceActor, AActor* TargetActor, FGameplayTag DamageType)
{
    OnReceivedKillLog.Broadcast(SourceActor, TargetActor, DamageType);
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

void ANLPlayerController::EnableInteraction(AInteractable* Interactable, FString Message)
{
    if (!IsValid(Interactable) || (IsValid(InteractableActor) && InteractableActor == Interactable))
    {
        return;
    }

    InteractableActor = Interactable;

    OnInteractionEnabled.Broadcast(Interactable, Message);
}

void ANLPlayerController::DisableInteraction()
{
    InteractableActor = nullptr;

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
