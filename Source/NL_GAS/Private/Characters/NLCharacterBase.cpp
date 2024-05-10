// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NLCharacterBase.h"

#include "AbilitySystem/NLAbilitySystemComponent.h"
#include "Components/NLCharacterComponent.h"

ANLCharacterBase::ANLCharacterBase()
{
    PrimaryActorTick.bCanEverTick = false;

    NLCharacterComponent = CreateDefaultSubobject<UNLCharacterComponent>(FName("NLCharacterComponent"));
}

void ANLCharacterBase::BeginPlay()
{
    Super::BeginPlay();
    
}

UAbilitySystemComponent* ANLCharacterBase::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void ANLCharacterBase::OnWeaponAdded(AWeaponActor* Weapon)
{
    NLCharacterComponent->WeaponAdded(Weapon);
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
