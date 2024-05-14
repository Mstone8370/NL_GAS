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
#include "HUD/NLHUD.h"

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
    if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
    {
        if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            for (const TSoftObjectPtr<UInputMappingContext>& IMC : StartupIMC)
            {
                if (!IMC.IsNull())
                {
                    InputSystem->AddMappingContext(IMC.LoadSynchronous(), 0);
                }
            }
        }
    }

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

    NLInputComponent->BindAbilityActions(
        IC,
        this,
        &ANLPlayerController::AbilityInputTagPressed,
        &ANLPlayerController::AbilityInputTagReleased
    );
}

void ANLPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
    if (GetNLAbilitySystemComponent())
    {
        GetNLAbilitySystemComponent()->ProcessAbilityInput(DeltaTime, bGamePaused);
    }

    Super::PostProcessInput(DeltaTime, bGamePaused);
}

void ANLPlayerController::Move(const FInputActionValue& Value)
{
    const FVector VectorValue = Value.Get<FVector>();
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
    GetCharacter()->Jump();
}

void ANLPlayerController::Crouch()
{
    GetCharacter()->Crouch();
}

void ANLPlayerController::UnCrouch()
{
    GetCharacter()->UnCrouch();
}

void ANLPlayerController::CrouchToggle()
{
    if (GetCharacter()->bIsCrouched)
    {
        GetCharacter()->UnCrouch();
    }
    else
    {
        GetCharacter()->Crouch();
    }
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

void ANLPlayerController::Client_ShowDamageCauseIndicator_Implementation(float InDamage, bool bInIsCriticalHit, AActor* DamagedActor)
{
    UE_LOG(LogTemp, Warning, TEXT("Damage Caused: [%s], Damage: %f"), *GetNameSafe(DamagedActor), InDamage);
    if (ANLHUD* NLHUD = Cast<ANLHUD>(GetHUD()))
    {
        NLHUD->ShowDamageCauseIndicator(InDamage, bInIsCriticalHit, DamagedActor);
    }
}

void ANLPlayerController::SetLookSensitivity(float InLookSensitivity)
{
    CurrentLookSensitivity = InLookSensitivity;
}

void ANLPlayerController::OnCausedDamage(float InDamage, bool bInIsCriticalHit, AActor* DamagedActor)
{
    Client_ShowDamageCauseIndicator(InDamage, bInIsCriticalHit, DamagedActor);
}
