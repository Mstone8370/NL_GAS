// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/WeaponActor.h"

#include "Components/SphereComponent.h"
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
#include "Interface/PlayerInterface.h"

AWeaponActor::AWeaponActor()
	: MagSize(0)
	, CurrentBulletNum(0)
	, bIsInitialized(false)
	, bIsEverDrawn(false)
	, ReloadState(EReloadState::None)
{
 	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	InteractionType = Interaction_Pickup_Weapon;

	RootMesh->SetCollisionObjectType(ECC_WeaponProp);
	RootMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	RootMesh->SetMassOverrideInKg(NAME_None, 2.f, true);

	SphereCollision->SetSphereRadius(200.f, false);
}

void AWeaponActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWeaponActor, WeaponTag, COND_InitialOnly);
	DOREPLIFETIME_CONDITION_NOTIFY(AWeaponActor, CurrentBulletNum, COND_AutonomousOnly, REPNOTIFY_OnChanged);
}

void AWeaponActor::OnRep_AttachmentReplication()
{
	// ����ġ ������ ���ø�����Ʈ�ɶ� �⺻������ �ϴ� �۾��� ������.
	// ��� ����ġ �۾��� ���� �����Ǵ� ��.
}

void AWeaponActor::BeginPlay()
{
	Super::BeginPlay();

	InitializeWeapon(WeaponTag);
}

void AWeaponActor::OnRep_CurrentBulletNum(int32 OldNum)
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] CurrentBulletNum Replicated. From %d to %d"), *WeaponTag.ToString(), CurrentBulletNum, OldNum);
	BulletNumChanged.Broadcast(this, CurrentBulletNum);
}

void AWeaponActor::InitializeWeapon(const FGameplayTag& InWeaponTag, bool bForceInit)
{
	if (!InWeaponTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] WeaponActor Initialize failed. WeaponTag is not valid"), *GetNameSafe(this));
		return;
	}

	if (!bForceInit && bIsInitialized)
	{
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
	RootMesh->SetStaticMesh(PropMesh);

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
	SetBulletNum_Internal(MagSize);

	SpreadInfo = Info->SpreadInfo;

	IronsightADSFOVTag = Info->IronsightADSFOVTag;

	// Initialize finished
	bIsInitialized = true;

	bool bHasOwner = IsValid(GetOwner());
	SetWeaponState(bHasOwner);
}

void AWeaponActor::SetWeaponState(bool bInIsEuipped)
{
	bIsInteracting = bInIsEuipped;

	if (bIsInteracting)
	{
		RootMesh->bOwnerNoSee = true;
		RootMesh->MarkRenderStateDirty();
		RootMesh->SetSimulatePhysics(false);
		RootMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		
		SetActorRelativeRotation(FRotator(0.f, -90.f, 0.f));

		if (GetOwner() && GetOwner()->Implements<UCombatInterface>())
		{
			Cast<ICombatInterface>(GetOwner())->OnWeaponAdded(this);
		}
		else
		{
			// �ùķ���Ƽ�� ���Ͻ��� �����ε� ������ ���Ⱑ ���� ���ø�����Ʈ �Ǵ� ���
			UE_LOG(LogTemp, Error, TEXT("Weapon is equipped, but owner is nullptr"));
		}
	}
	else
	{
		RootMesh->bOwnerNoSee = false;
		RootMesh->MarkRenderStateDirty();
		RootMesh->SetSimulatePhysics(true);
		RootMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

		bIsEverDrawn = false;
		ReloadState = EReloadState::None;
		BulletNumChanged.Clear();
	}
}

bool AWeaponActor::CanInteract() const
{
	return !bIsInteracting;
}

void AWeaponActor::OnStartInteraction_Implementation(APawn* Interactor)
{
	SetWeaponState(true);
}

void AWeaponActor::OnEndInteraction_Implementation()
{
	SetOwner(nullptr);
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetWeaponState(false);

	RootMesh->AddImpulse(GetActorRightVector() * 200.f, NAME_None, true);
}

const FGameplayTag& AWeaponActor::GetADSFOVTag() const
{
	// TODO:
	return IronsightADSFOVTag;
}

bool AWeaponActor::CanAttack() const
{
	return !IsReloading() && !IsMagEmpty();
}

void AWeaponActor::Drawn()
{
	bIsEverDrawn = true;

	CheckReloadState();
}

void AWeaponActor::OnRep_IsInteracting()
{
	SetWeaponState(bIsInteracting);
}

void AWeaponActor::OnRep_Owner()
{
	SetWeaponState(GetOwner() != nullptr);
}

void AWeaponActor::SetBulletNum_Internal(int32 NewBulletNum)
{
	CurrentBulletNum = NewBulletNum;
	BulletNumChanged.Broadcast(this, CurrentBulletNum);
}

void AWeaponActor::Holstered()
{

}

void AWeaponActor::PickedUp(APawn* Interactor)
{
	StartInteraction(Interactor);
}

void AWeaponActor::Dropped()
{
	EndInteraction();
}

bool AWeaponActor::CommitWeaponCost()
{
	if (CanAttack())
	{
		SetBulletNum_Internal(CurrentBulletNum - 1);
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
			SetBulletNum_Internal(0);
		}
		else if (StateTag.MatchesTagExact(Event_Reload_MagIn))
		{
			ReloadState = EReloadState::MagIn;
			SetBulletNum_Internal(bIsTacticalReload ? MagSize + 1 : MagSize);
		}
		else if (StateTag.MatchesTagExact(Event_Reload_Finished))
		{
			ReloadState = EReloadState::None;
			bIsTacticalReload = false;
		}
	}
}

const FWeaponSpreadInfo* AWeaponActor::GetSpreadInfo() const
{
	return &SpreadInfo;
}

void AWeaponActor::CheckReloadState()
{
	if (HasAuthority() && IsReloading())
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
		{
			ASC->TryActivateAbility(ReloadAbilitySpecHandle);
		}
	}
}
