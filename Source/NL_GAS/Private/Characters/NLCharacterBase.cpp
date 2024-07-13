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

void ANLCharacterBase::OnRep_DeathInfo()
{
    if (!DeathInfo.bIsDead)
    {
        return;
    }

    if (GetLocalRole() == ROLE_SimulatedProxy)
    {
        OnDead_Internal(DeathInfo, true);
    }
    else
    {
        OnDead_Internal(DeathInfo, false);
    }
}

void ANLCharacterBase::OnDead_Internal(const FDeathInfo& Info, bool bSimulated)
{
    DeathInfo = Info;

    EnableRagdoll();

    OnDead_BP(Info, bSimulated);
}
