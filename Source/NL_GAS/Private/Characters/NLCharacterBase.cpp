// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NLCharacterBase.h"

#include "AbilitySystem/NLAbilitySystemComponent.h"
#include "Components/NLCharacterComponent.h"
#include "Components/DamageTextWidgetComponent.h"
#include "NLFunctionLibrary.h"
#include "Net/UnrealNetwork.h"

ANLCharacterBase::ANLCharacterBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;

    NLCharacterComponent = CreateDefaultSubobject<UNLCharacterComponent>(FName("NLCharacterComponent"));
}

void ANLCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(ANLCharacterBase, DeathInfo, COND_None, REPNOTIFY_OnChanged);
}

void ANLCharacterBase::BeginPlay()
{
    Super::BeginPlay();
    
    UNLFunctionLibrary::LoadHitboxComponents(GetMesh());
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

void ANLCharacterBase::OnDead(const FDeathInfo& Info)
{
    if (!DeathInfo.bIsDead && Info.bIsDead)
    {
        OnDead_Internal(Info);
    }
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
    
    OnRespawned_Internal();
}

void ANLCharacterBase::InitAbilityActorInfo() {}

void ANLCharacterBase::AddStartupAbilities()
{
    if (!HasAuthority())
    {
        return;
    }

    if (UNLAbilitySystemComponent* NLASC = Cast<UNLAbilitySystemComponent>(GetAbilitySystemComponent()))
    {
        NLASC->AddAbilities(StartupAbilities);
    }
}

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
        OnDead_Internal(DeathInfo, GetLocalRole() == ROLE_SimulatedProxy);
    }
    else
    {
        OnRespawned_Internal(GetLocalRole() == ROLE_SimulatedProxy);
        UE_LOG(LogTemp, Warning, TEXT("Client Respawn"));
    }
}

void ANLCharacterBase::OnDead_Internal(const FDeathInfo& Info, bool bSimulated)
{
    DeathInfo = Info;

    EnableRagdoll();
    GetWorldTimerManager().SetTimer(DeathRagdollTimerHandle, this, &ANLCharacterBase::OnDeathRagdollTimeEnded, DeathRagdollTime, false);

    if (NLCharacterComponent)
    {
        NLCharacterComponent->HandleOwnerDeath();
    }

    OnDead_BP(Info, bSimulated);
}

void ANLCharacterBase::OnDeathRagdollTimeEnded()
{
    GetMesh()->SetVisibility(false);

    DisableRagdoll();

    SetActorLocation(FVector(0.f, 0.f, 0.f));
}

void ANLCharacterBase::OnRespawned_Internal(bool bSimulated)
{
    DeathInfo = FDeathInfo(false);

    if (HasAuthority())
    {
        InitDefaultAttribute();
    }

    GetMesh()->SetVisibility(true);

    if (ACharacter* DefaultCharacter = Cast<ACharacter>(GetClass()->GetDefaultObject()))
    {
        const FVector MeshLocation = DefaultCharacter->GetMesh()->GetRelativeLocation();
        const FRotator MeshRotation = DefaultCharacter->GetMesh()->GetRelativeRotation();

        GetMesh()->SetRelativeLocationAndRotation(MeshLocation, MeshRotation);
    }

    OnRespawned_BP(bSimulated);
}
