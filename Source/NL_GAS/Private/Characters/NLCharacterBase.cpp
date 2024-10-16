// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NLCharacterBase.h"

#include "AbilitySystem/NLAbilitySystemComponent.h"
#include "Components/NLCharacterComponent.h"
#include "Components/DamageTextWidgetComponent.h"
#include "NLFunctionLibrary.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/NLWidgetComponent.h"

ANLCharacterBase::ANLCharacterBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;

    NLCharacterComponent = CreateDefaultSubobject<UNLCharacterComponent>(FName("NLCharacterComponent"));

    CharacterNameWidgetComponent = CreateDefaultSubobject<UNLWidgetComponent>(FName("CharacterNameWidgetComponent"));
    CharacterNameWidgetComponent->SetupAttachment(GetMesh());
}

void ANLCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(ANLCharacterBase, DeathInfo, COND_None, REPNOTIFY_OnChanged);
}

void ANLCharacterBase::BeginPlay()
{
    Super::BeginPlay();
    
    if (HasAuthority())
    {
        UNLFunctionLibrary::LoadHitboxComponents(GetMesh());
    }

    if (CharacterNameWidgetComponent)
    {
        if (CharacterNameWidgetComponent->IsWidgetInitialized())
        {
            BP_OnCharacterNameWidgetInitialized();
        }
        else
        {
            CharacterNameWidgetComponent->OnWidgetInitialized.AddLambda(
                [this]()
                {
                    BP_OnCharacterNameWidgetInitialized();
                }
            );
        }
    }
}

UAbilitySystemComponent* ANLCharacterBase::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void ANLCharacterBase::OnWeaponAdded(AWeaponActor* Weapon)
{
    NLCharacterComponent->WeaponAdded(Weapon);
}

void ANLCharacterBase::ShowDamageText_Implementation(float Value, bool bIsCriticalHit)
{
    if (!DamageTextWidgetComponentClass)
    {
        return;
    }

    if (!(LastDamageText && LastDamageText->IsWatingUpdate()))
    {
        LastDamageText = NewObject<UDamageTextWidgetComponent>(this, DamageTextWidgetComponentClass);
        LastDamageText->RegisterComponent();
        LastDamageText->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        LastDamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    }
    LastDamageText->UpdateValue(Value, bIsCriticalHit);
}

void ANLCharacterBase::OnDeath(const FDeathInfo& Info)
{
    // On Server

    if (!DeathInfo.bIsDead && Info.bIsDead)
    {
        DeathInfo = Info;

        HandleDeath();
    }
}

void ANLCharacterBase::Destroyed()
{
    if (NLCharacterComponent)
    {
        NLCharacterComponent->HandleOwnerDestroyed();
    }
    if (CharacterNameWidgetComponent)
    {
        CharacterNameWidgetComponent->OnWidgetInitialized.Clear();
    }

    Super::Destroyed();
}

void ANLCharacterBase::EnableRagdoll()
{
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
}

void ANLCharacterBase::DisableRagdoll()
{
    GetMesh()->SetSimulatePhysics(false);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    if (ACharacter* DefaultCharacter = GetClass()->GetDefaultObject<ACharacter>())
    {
        FVector DefaultLocation = DefaultCharacter->GetMesh()->GetRelativeLocation();
        FRotator DefaultRotation = DefaultCharacter->GetMesh()->GetRelativeRotation();
        GetMesh()->SetRelativeLocation(DefaultLocation);
        GetMesh()->SetRelativeRotation(DefaultRotation);
    }
}

void ANLCharacterBase::OnRespawned()
{
    // On Server

    DeathInfo = FDeathInfo(false);

    HandleRespawn();
}

void ANLCharacterBase::OnResetted()
{
    // On Server

    HandleReset();
}

void ANLCharacterBase::InitAbilityActorInfo() {}

void ANLCharacterBase::InitDefaultAttribute()
{
    check(AbilitySystemComponent);

    if (DefaultAttribute)
    {
        FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
        ContextHandle.AddInstigator(this, this);
        ContextHandle.AddSourceObject(this);

        const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttribute, 1.f, ContextHandle);

        AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }
}

void ANLCharacterBase::OnRep_DeathInfo(FDeathInfo OldDeathInfo)
{
    if (DeathInfo.bIsDead == OldDeathInfo.bIsDead)
    {
        return;
    }

    if (DeathInfo.bIsDead)
    {
        HandleDeath(GetLocalRole() == ROLE_SimulatedProxy);
    }
    else
    {
        HandleRespawn(GetLocalRole() == ROLE_SimulatedProxy);
    }
}

void ANLCharacterBase::HandleDeath(bool bSimulated)
{
    // On Server and Client

    EnableRagdoll();
    GetWorldTimerManager().SetTimer(DeathRagdollTimerHandle, this, &ANLCharacterBase::OnDeathRagdollTimeEnded, DeathRagdollTime, false);

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (NLCharacterComponent)
    {
        NLCharacterComponent->HandleOwnerDeath();
    }

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->bWantsToCrouch = false;
        GetCharacterMovement()->DisableMovement();
    }

    bPressedJump = false;
}

void ANLCharacterBase::OnDeathRagdollTimeEnded()
{
    GetMesh()->SetVisibility(false);

    DisableRagdoll();

    SetActorLocation(FVector(0.f, 0.f, 0.f));
}

void ANLCharacterBase::HandleRespawn(bool bSimulated)
{
    if (HasAuthority())
    {
        InitDefaultAttribute();
    }

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    GetMesh()->SetVisibility(true);

    if (ACharacter* DefaultCharacter = Cast<ACharacter>(GetClass()->GetDefaultObject()))
    {
        const FVector MeshLocation = DefaultCharacter->GetMesh()->GetRelativeLocation();
        const FRotator MeshRotation = DefaultCharacter->GetMesh()->GetRelativeRotation();

        GetMesh()->SetRelativeLocationAndRotation(MeshLocation, MeshRotation);

        GetCapsuleComponent()->SetCollisionEnabled(DefaultCharacter->GetCapsuleComponent()->GetCollisionEnabled());
    }

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->SetMovementMode(GetCharacterMovement()->DefaultLandMovementMode);
    }
}

void ANLCharacterBase::HandleReset(bool bSimulated)
{
    // On Server

    InitDefaultAttribute();
    NLCharacterComponent->ClearWeapons();
}

void ANLCharacterBase::GetAimPoint(FVector& OutViewLocation, FRotator& OutViewRotation) const
{
    OutViewLocation = GetActorLocation();
    OutViewRotation = GetActorRotation();
}

bool ANLCharacterBase::IsWeaponSlotFull() const
{
    if (NLCharacterComponent)
    {
        return NLCharacterComponent->IsWeaponSlotFull();
    }
    return false;
}

void ANLCharacterBase::SetAsEnemy_Implementation()
{
    
}

void ANLCharacterBase::SetAsFriendly_Implementation(bool bSelf)
{

}

void ANLCharacterBase::SetAsNeutral_Implementation()
{

}
