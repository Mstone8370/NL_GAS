// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NLCharacterBase.h"

#include "AbilitySystem/NLAbilitySystemComponent.h"

ANLCharacterBase::ANLCharacterBase()
{
    PrimaryActorTick.bCanEverTick = false;

    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("WeaponMesh"));
    WeaponMesh->bOwnerNoSee = true;
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetupAttachment(GetMesh(), FName("weapon_r"));
}

void ANLCharacterBase::BeginPlay()
{
    Super::BeginPlay();
    
}

UAbilitySystemComponent* ANLCharacterBase::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void ANLCharacterBase::SetWeaponMesh(UStaticMesh* NewMesh)
{
    WeaponMesh->SetStaticMesh(NewMesh);
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
