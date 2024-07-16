// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NLPlayerState.h"

#include "AbilitySystem/NLAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/NLAttributeSet.h"
#include "NLFunctionLibrary.h"
#include "Data/WeaponInfo.h"
#include "Net/UnrealNetwork.h"
#include "Characters/NLCharacterBase.h"
#include "Characters/NLPlayerCharacter.h"
#include "NLGameplayTags.h"
#include "Actors/WeaponActor.h"

ANLPlayerState::ANLPlayerState()
{
    AbilitySystemComponent = CreateDefaultSubobject<UNLAbilitySystemComponent>("AbilitySystemComponent");
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->SetIsReplicated(true);
        AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
    }

    AttributeSet = CreateDefaultSubobject<UNLAttributeSet>("AttributeSet");

    bUseCustomPlayerNames = true;

    // ÃÊ´ç 10¹ø
    NetUpdateFrequency = 10.f;
}

void ANLPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

UAbilitySystemComponent* ANLPlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

FString ANLPlayerState::GetPlayerNameCustom() const
{
    return Super::GetPlayerNameCustom();
}
