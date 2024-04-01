// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/WeaponActor.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Data/WeaponInfo.h"
#include "NL_GAS/NL_GAS.h"
#include "Abilities/GameplayAbility.h"
#include "NLFunctionLibrary.h"
#include "Net/UnrealNetwork.h"
#include "NLGameplayTags.h"

AWeaponActor::AWeaponActor()
{
 	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("WeaponMesh"));
	WeaponMeshComponent->SetCollisionObjectType(ECC_WeaponProp);
	WeaponMeshComponent->SetMassOverrideInKg(NAME_None, 2.f, true);
	WeaponMeshComponent->bOwnerNoSee = true;
	SetRootComponent(WeaponMeshComponent);
}

void AWeaponActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWeaponActor, WeaponTag, COND_InitialOnly);
}

void AWeaponActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (!bInitialized && WeaponTag.IsValid())
	{
		InitalizeWeapon(WeaponTag);
	}

	SetWeaponState(IsValid(GetOwner()));
}

void AWeaponActor::InitalizeWeapon(const FGameplayTag& InWeaponTag)
{
	if (!InWeaponTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] WeaponActor Initialize failed. WeaponTag is not valid"), *GetNameSafe(this));
		return;
	}

	const FWeaponInfo* Info = UNLFunctionLibrary::GetWeaponInfoByTag(this, InWeaponTag);
	if (!Info)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] WeaponActor Initialize failed. Info is not valid"), *GetNameSafe(this));
		return;
	}

	WeaponTag = InWeaponTag;

	// Init Weapon Mesh
	UStaticMesh* PropMesh = Info->PropMesh.Get();
	if (!PropMesh)
	{
		PropMesh = Info->PropMesh.LoadSynchronous();
	}
	WeaponMeshComponent->SetStaticMesh(PropMesh);

	// Init View Weapon Mesh
	ViewWeaponMesh = Info->ViewModelMesh.Get();
	if (!ViewWeaponMesh)
	{
		ViewWeaponMesh = Info->ViewModelMesh.LoadSynchronous();
	}

	PrimaryAbilityClass = Info->PrimaryAbility;
	SecondaryAbilityClass = Info->SecondaryAbility;
	ReloadAbilityClass = Info->ReloadAbility;

	bInitialized = true;
}

void AWeaponActor::SetWeaponState(bool bInIsEuipped)
{
	bIsEquipped = bInIsEuipped;

	if (bIsEquipped)
	{
		WeaponMeshComponent->bOwnerNoSee = true;
		WeaponMeshComponent->SetSimulatePhysics(false);
		WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMeshComponent->CastShadow = 0;
	}
	else
	{
		WeaponMeshComponent->bOwnerNoSee = false;
		WeaponMeshComponent->SetSimulatePhysics(true);
		WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMeshComponent->CastShadow = 1;
	}
}
