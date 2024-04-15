// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/WeaponActor.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Data/WeaponInfo.h"
#include "NL_GAS/NL_GAS.h"
#include "Abilities/GameplayAbility.h"
#include "NLFunctionLibrary.h"
#include "Net/UnrealNetwork.h"
#include "NLGameplayTags.h"
#include "Interface/CombatInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

AWeaponActor::AWeaponActor()
	: MagSize(0)
	, CurrentBulletNum(0)
	, bIsInitialized(false)
	, bIsEverDrawn(false)
	, bIsEquipped(false)
	, ReloadState(EReloadState::None)
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
	DOREPLIFETIME_CONDITION_NOTIFY(AWeaponActor, CurrentBulletNum, COND_AutonomousOnly, REPNOTIFY_OnChanged);
}

void AWeaponActor::BeginPlay()
{
	Super::BeginPlay();
	
	InitalizeWeapon(WeaponTag);
}

void AWeaponActor::OnRep_CurrentBulletNum(int32 OldNum)
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] CurrentBulletNum Replicated. From %d to %d"), *WeaponTag.ToString(), CurrentBulletNum, OldNum);
}

void AWeaponActor::InitalizeWeapon(const FGameplayTag& InWeaponTag)
{
	if (bIsInitialized)
	{
		return;
	}

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

	WeaponName = Info->WeaponName;

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
	WeaponAnimInstanceClass = Info->WeaponAnimBP;

	ArmsAnimLayerClass = Info->ArmsAnimLayerClass;

	// Set Ability Class;
	PrimaryAbilityClass = Info->PrimaryAbility;
	SecondaryAbilityClass = Info->SecondaryAbility;
	ReloadAbilityClass = Info->ReloadAbility;

	MagSize = Info->MagSize;
	CurrentBulletNum = MagSize;

	// Initialize finished
	bIsInitialized = true;

	bool bHasOwner = IsValid(GetOwner());
	SetWeaponState(bHasOwner);
	if (bHasOwner)
	{
		if (GetOwner()->Implements<UCombatInterface>())
		{
			Cast<ICombatInterface>(GetOwner())->OnWeaponAdded(this);
		}
	}
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
		SetActorHiddenInGame(true);
	}
	else
	{
		WeaponMeshComponent->bOwnerNoSee = false;
		WeaponMeshComponent->SetSimulatePhysics(true);
		WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMeshComponent->CastShadow = 1;
		SetActorHiddenInGame(false);
	}
}

bool AWeaponActor::CanAttack() const
{
	return !IsReloading() && !IsMagEmpty();
}

void AWeaponActor::Drawn()
{
	bIsEverDrawn = true;

	if (HasAuthority() && IsReloading())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
		{
			ASC->TryActivateAbility(ReloadAbilitySpecHandle);
		}
	}
}

void AWeaponActor::Holstered()
{

}

bool AWeaponActor::CommitWeaponCost()
{
	if (CanAttack())
	{
		CurrentBulletNum--;
		return true;
	}
	return false;
}

void AWeaponActor::ReloadStateChanged(const FGameplayTag& StateTag)
{
	if (StateTag.IsValid())
	{
		if (StateTag.MatchesTagExact(Event_Reload_MagOut))
		{
			ReloadState = EReloadState::MagOut;
			bIsTacticalReload = CurrentBulletNum > 0;
			CurrentBulletNum = 0;
		}
		else if (StateTag.MatchesTagExact(Event_Reload_MagIn))
		{
			ReloadState = EReloadState::MagIn;
			CurrentBulletNum = bIsTacticalReload ? MagSize + 1 : MagSize;
		}
		else if (StateTag.MatchesTagExact(Event_Reload_Finished))
		{
			ReloadState = EReloadState::None;
			bIsTacticalReload = false;
		}
	}
}
