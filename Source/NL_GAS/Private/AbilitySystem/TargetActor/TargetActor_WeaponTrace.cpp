// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/TargetActor/TargetActor_WeaponTrace.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/LightWeightInstanceSubsystem.h"
#include "Abilities/GameplayAbility.h"
#include "Interface/PlayerInterface.h"

// --------------------------------------------------------------------------------------------------------------------------------------------------------
//
//	ATargetActor_WeaponTrace
//
// --------------------------------------------------------------------------------------------------------------------------------------------------------

ATargetActor_WeaponTrace::ATargetActor_WeaponTrace(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FHitResult ATargetActor_WeaponTrace::PerformTrace(AActor* InSourceActor)
{
	bool bTraceComplex = false;
	TArray<AActor*> ActorsToIgnore;

	ActorsToIgnore.Add(InSourceActor);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(ATargetActor_WeaponTrace), bTraceComplex);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActors(ActorsToIgnore);

	/*
	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();// InSourceActor->GetActorLocation();
	FVector TraceEnd;
	AimWithPlayerController(InSourceActor, Params, TraceStart, TraceEnd);		//Effective on server and launching client only
	*/

	// 플레이어 컨트롤러의 에임 정보를 기반으로 트레이싱하지만, 카메라에 에니메이션이 적용되지 않은 정보를 사용.
	APlayerController* PC = OwningAbility->GetCurrentActorInfo()->PlayerController.Get();
	AActor* AvatarActor = OwningAbility->GetCurrentActorInfo()->AvatarActor.Get();
	check(PC);
	check(AvatarActor);

	FVector ViewStart;
	FRotator ViewRot;
	PC->GetPlayerViewPoint(ViewStart, ViewRot);
	ViewRot = PC->GetControlRotation();

	FVector ViewDir = ViewRot.Vector();
	IPlayerInterface::Execute_ApplyWeaponRandomSpreadAtViewDirection(AvatarActor, ViewDir);
	FVector ViewEnd = ViewStart + (ViewDir * MaxRange);

	FVector TraceStart = ViewStart;
	FVector TraceEnd = ViewEnd;

	// ------------------------------------------------------

	FHitResult ReturnHitResult;
	LineTraceWithFilter(ReturnHitResult, InSourceActor->GetWorld(), Filter, TraceStart, TraceEnd, TraceProfile.Name, Params);
	//Default to end of trace line if we don't hit anything.
	if (!ReturnHitResult.bBlockingHit)
	{
		ReturnHitResult.Location = TraceEnd;
	}
	if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActor.Get())
	{
		const bool bHitActor = (ReturnHitResult.bBlockingHit && (ReturnHitResult.HitObjectHandle.IsValid()));
		const FVector ReticleLocation = (bHitActor && LocalReticleActor->bSnapToTargetedActor) ? FLightWeightInstanceSubsystem::Get().GetLocation(ReturnHitResult.HitObjectHandle) : ReturnHitResult.Location;

		LocalReticleActor->SetActorLocation(ReticleLocation);
		LocalReticleActor->SetIsTargetAnActor(bHitActor);
	}

#if ENABLE_DRAW_DEBUG
	if (bDebug)
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green);
		DrawDebugSphere(GetWorld(), TraceEnd, 100.0f, 16, FColor::Green);
	}
#endif // ENABLE_DRAW_DEBUG
	return ReturnHitResult;
}
