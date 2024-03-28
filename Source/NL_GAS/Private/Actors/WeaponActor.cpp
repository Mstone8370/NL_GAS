// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/WeaponActor.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Data/WeaponInfo.h"
#include "NL_GAS/NL_GAS.h"

AWeaponActor::AWeaponActor()
{
 	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("WeaponMesh"));
	WeaponMesh->SetCollisionObjectType(ECC_WeaponProp);
	SetRootComponent(WeaponMesh);

	ViewWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("ViewWeaponMesh"));
	ViewWeaponMesh->bOnlyOwnerSee = true;
	ViewWeaponMesh->CastShadow = 0;
	ViewWeaponMesh->SetupAttachment(GetRootComponent());
}

void AWeaponActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (StartupWeaponInfo)
	{
		InitalizeWeapon(&StartupWeaponInfo->WeaponInfo);
	}
}

void AWeaponActor::InitalizeWeapon(const FWeaponInfo* Info)
{
	if (!Info)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] WeaponActor Initialize failed. Info in not valid"), *GetNameSafe(this));
		return;
	}

	// Init Weapon Mesh
	UStaticMesh* PropMesh = Info->PropMesh.Get();
	if (!PropMesh)
	{
		PropMesh = Info->PropMesh.LoadSynchronous();
	}
	WeaponMesh->SetStaticMesh(PropMesh);

	// Init View Weapon Mesh
	USkeletalMesh* ViewMesh = Info->ViewModelMesh.Get();
	if (!ViewMesh)
	{
		ViewMesh = Info->ViewModelMesh.LoadSynchronous();
	}
	ViewWeaponMesh->SetSkeletalMesh(ViewMesh);

	// Init Material Instance Dynamic
	for (uint8 i = 0; i < ViewWeaponMesh->GetNumMaterials(); i++)
	{
		UMaterialInstanceDynamic* MatInstDynamic = ViewWeaponMesh->CreateAndSetMaterialInstanceDynamic(i);
		MatInstDynamic->SetScalarParameterValue(FName("FOV"), 80.f);
	}

	// Is Prop?
	if (!IsValid(GetOwner()))
	{
		WeaponMesh->SetSimulatePhysics(true);
	}
}
