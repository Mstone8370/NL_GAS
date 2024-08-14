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
	, bIsEquipped(false)
	, ReloadState(EReloadState::None)
{
 	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("WeaponMesh"));
	WeaponMeshComponent->SetCollisionObjectType(ECC_WeaponProp);
	WeaponMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMeshComponent->SetMassOverrideInKg(NAME_None, 2.f, true);
	SetRootComponent(WeaponMeshComponent);

	PickUpCollision = CreateDefaultSubobject<USphereComponent>(FName("PickUpCollision"));
	PickUpCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickUpCollision->SetGenerateOverlapEvents(true);
	PickUpCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	PickUpCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	PickUpCollision->SetSphereRadius(200.f, false);
	PickUpCollision->SetupAttachment(WeaponMeshComponent);
}

void AWeaponActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWeaponActor, WeaponTag, COND_InitialOnly);
	DOREPLIFETIME_CONDITION_NOTIFY(AWeaponActor, CurrentBulletNum, COND_AutonomousOnly, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION_NOTIFY(AWeaponActor, bIsEquipped, COND_None, REPNOTIFY_OnChanged);
}

void AWeaponActor::OnRep_AttachmentReplication()
{
	// 어태치 정보가 레플리케이트될때 기본적으로 하는 작업을 무시함.
	// 모든 어태치 작업은 직접 관리되는 중.
}

void AWeaponActor::BeginPlay()
{
	Super::BeginPlay();

	PickUpCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeaponActor::OnPickUpCollisionBeginOverlap);
	PickUpCollision->OnComponentEndOverlap.AddDynamic(this, &AWeaponActor::OnPickUpCollisionEndOverlap);
	
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
	bIsEquipped = bInIsEuipped;

	if (bIsEquipped)
	{
		WeaponMeshComponent->bOwnerNoSee = true;
		WeaponMeshComponent->MarkRenderStateDirty();
		WeaponMeshComponent->SetSimulatePhysics(false);
		WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		PickUpCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		
		SetActorRelativeRotation(FRotator(0.f, -90.f, 0.f));

		if (GetOwner() && GetOwner()->Implements<UCombatInterface>())
		{
			Cast<ICombatInterface>(GetOwner())->OnWeaponAdded(this);
		}
		else
		{
			// 시뮬레이티드 프록시의 무기인데 폰보다 무기가 먼저 레플리케이트 되는 경우
			UE_LOG(LogTemp, Error, TEXT("Weapon is equipped, but owner is nullptr"));
		}
	}
	else
	{
		WeaponMeshComponent->bOwnerNoSee = false;
		WeaponMeshComponent->MarkRenderStateDirty();
		WeaponMeshComponent->SetSimulatePhysics(true);
		WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		PickUpCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

		bIsEverDrawn = false;
		ReloadState = EReloadState::None;
		BulletNumChanged.Clear();
	}
}

void AWeaponActor::OnPickUpCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!Execute_CanPickedUp(this))
	{
		return;
	}

	IPlayerInterface::Execute_OnPickupableRangeEnter(OtherActor);
}

void AWeaponActor::OnPickUpCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!Execute_CanPickedUp(this))
	{
		return;
	}

	IPlayerInterface::Execute_OnPickupableRangeExit(OtherActor);
}

bool AWeaponActor::CanPickedUp_Implementation()
{
	return !bIsEquipped;
}

void AWeaponActor::OnPickedUp_Implementation()
{
	SetWeaponState(true);
}

void AWeaponActor::OnDropped_Implementation()
{
	SetOwner(nullptr);
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetWeaponState(false);

	WeaponMeshComponent->AddImpulse(GetActorRightVector() * 200.f, NAME_None, true);
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

void AWeaponActor::OnRep_IsEquipped()
{
	SetWeaponState(bIsEquipped);
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

void AWeaponActor::Dropped()
{
	OnDropped_Implementation();
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
