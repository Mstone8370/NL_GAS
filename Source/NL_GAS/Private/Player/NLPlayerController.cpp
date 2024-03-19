// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NLPlayerController.h"

#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "Input/NLEnhancedInputComponent.h"
#include "NLGameplayTags.h"

void ANLPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

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
    NLInputComponent->BindActionByTag(IC, Input_Default_Crouch, ETriggerEvent::Triggered, this, &ANLPlayerController::Crouch);
    NLInputComponent->BindActionByTag(IC, Input_Default_Crouch, ETriggerEvent::Completed, this, &ANLPlayerController::UnCrouch);
    NLInputComponent->BindActionByTag(IC, Input_Default_CrouchToggle, ETriggerEvent::Triggered, this, &ANLPlayerController::CrouchToggle);
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
    AddYawInput(VectorValue.X * LookSensitivity);
    AddPitchInput(-VectorValue.Y * LookSensitivity);
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
