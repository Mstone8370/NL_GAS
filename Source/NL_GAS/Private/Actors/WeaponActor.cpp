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

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("WeaponMesh"));
	WeaponMesh->SetCollisionObjectType(ECC_WeaponProp);
	WeaponMesh->SetMassOverrideInKg(NAME_None, 2.f, true);
	WeaponMesh->bOwnerNoSee = true;
	SetRootComponent(WeaponMesh);

	ViewWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("ViewWeaponMesh"));
	ViewWeaponMesh->bOnlyOwnerSee = true;
	ViewWeaponMesh->CastShadow = 0;
	ViewWeaponMesh->SetupAttachment(GetRootComponent());
}

void AWeaponActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWeaponActor, WeaponTag, COND_InitialOnly);
}

void AWeaponActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (!bInitialized)
	{
		InitalizeWeapon(WeaponTag);
	}

	if (IsValid(GetOwner()))
	{

	}
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

	if (HasAuthority())
	{
		PrimaryAbilitySpec = FGameplayAbilitySpec(Info->PrimaryAbility, 1);
		PrimaryAbilitySpec.DynamicAbilityTags.AddTag(Input_Weapon_PrimaryAction);
		SecondaryAbilitySpec = FGameplayAbilitySpec(Info->SecondaryAbility, 1);
		SecondaryAbilitySpec.DynamicAbilityTags.AddTag(Input_Weapon_SecondaryAction);
		ReloadAbilitySpec = FGameplayAbilitySpec(Info->ReloadAbility, 1);
		ReloadAbilitySpec.DynamicAbilityTags.AddTag(Input_Weapon_Reload);
	}

	bInitialized = true;
}
