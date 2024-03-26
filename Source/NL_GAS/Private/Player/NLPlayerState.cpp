// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NLPlayerState.h"

#include "AbilitySystem/NLAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSet/NLAttributeSet.h"
#include "Net/UnrealNetwork.h"

ANLPlayerState::ANLPlayerState()
    : MaxSlotSize(3)
    , CurrentWeaponSlot(255)
{
    AbilitySystemComponent = CreateDefaultSubobject<UNLAbilitySystemComponent>("AbilitySystemComponent");
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->SetIsReplicated(true);
        AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
    }

    AttributeSet = CreateDefaultSubobject<UNLAttributeSet>("AttributeSet");

    // 초당 10번
    NetUpdateFrequency = 10.f;
}

void ANLPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(ANLPlayerState, CurrentWeaponSlot, COND_SimulatedOnly, REPNOTIFY_OnChanged);
    DOREPLIFETIME_CONDITION_NOTIFY(ANLPlayerState, WeaponTagSlot, COND_None, REPNOTIFY_OnChanged);
}

void ANLPlayerState::BeginPlay()
{
    Super::BeginPlay();

    WeaponTagSlot.Reset(MaxSlotSize);
}

void ANLPlayerState::OnRep_CurrentWeaponSlot(uint8 OldSlot)
{
    // 레플리케이트 되었다면 Simulated 액터라는 뜻이므로 3인칭 애니메이션과 3인칭 메시 설정.
}

void ANLPlayerState::OnRep_WeaponTagSlot()
{
    // TODO: Apply Weapon slot change
}

UAbilitySystemComponent* ANLPlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void ANLPlayerState::ChangeWeaponSlot(int32 NewWeaponSlot)
{
    // On Server and Client by GameplayAbility

    CurrentWeaponSlot = NewWeaponSlot;
    CurrentWeaponSlotChanged.Broadcast(CurrentWeaponSlot);
}

const FGameplayTag& ANLPlayerState::GetCurrentWeaponTag() const
{
    if (CurrentWeaponSlot <= WeaponTagSlot.Num())
    {
        return WeaponTagSlot[CurrentWeaponSlot];
    }
    return FGameplayTag::EmptyTag;
}
