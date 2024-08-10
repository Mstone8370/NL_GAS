// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/DeathCam.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

ADeathCam::ADeathCam()
{
 	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(FName("SpringArm"));
	SpringArmComponent->TargetArmLength = 300.f;
	SetRootComponent(SpringArmComponent);

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(FName("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
}

void ADeathCam::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Tracking(DeltaTime);
}

void ADeathCam::Tracking(float DeltaTime)
{
	if (!IsValid(TargetActor))
	{
		return;
	}

	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FVector TargetDirection = (TargetLocation - GetActorLocation()).GetSafeNormal(UE_SMALL_NUMBER, TargetActor->GetActorForwardVector());

	const FRotator InterpedRotation = FMath::RInterpTo(GetActorRotation(), TargetDirection.Rotation(), DeltaTime, InterpSpeed);

	SetActorRotation(InterpedRotation);
}

void ADeathCam::SetTargetActor(AActor* InTargetActor)
{
	TargetActor = InTargetActor;
}

