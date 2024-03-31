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
    // DOREPLIFETIME_CONDITION_NOTIFY(ANLPlayerState, WeaponTagSlot, COND_None, REPNOTIFY_OnChanged);
}

void ANLPlayerState::BeginPlay()
{
    Super::BeginPlay();

    WeaponTagSlot.Reset(MaxSlotSize);
}

void ANLPlayerState::OnRep_CurrentWeaponSlot(uint8 OldSlot)
{
    // 레플리케이트 되었다면 Simulated 액터라는 뜻이므로 3인칭 애니메이션과 3인칭 메시 설정.
    const FGameplayTag WeaponTag = GetCurrentWeaponTag();
    if (!WeaponTag.IsValid())
    {
        return;
    }

    const FWeaponInfo* Info = UNLFunctionLibrary::GetWeaponInfoByTag(this, WeaponTag);
    if (Info)
    {
        if (ANLCharacterBase* NLCharacterBase = Cast<ANLCharacterBase>(GetPawn()))
        {
            // Change Weapon Mesh
            UStaticMesh* Mesh = Info->PropMesh.Get();
            if (!Info->PropMesh.IsValid()) // = !IsValid(Mesh)
            {
                Mesh = Info->PropMesh.LoadSynchronous();
            }
            NLCharacterBase->SetWeaponMesh(Mesh);

            // Change Character Mesh AnimBP
            if (Info->CharacterMeshAnimBP)
            {
                NLCharacterBase->GetMesh()->SetAnimInstanceClass(Info->CharacterMeshAnimBP);
            }
        }
    }
}

void ANLPlayerState::AddStartupWeapons()
{
    // On Server

    ANLPlayerCharacter* PlayerCharacter = Cast<ANLPlayerCharacter>(GetPawn());

    for (const FGameplayTag& Tag : StartupWeapons)
    {
        if (WeaponTagSlot.Num() >= MaxSlotSize)
        {
            break;
        }

        WeaponTagSlot.Add(Tag);

        FTransform SpawnTransform;
        SpawnTransform.SetLocation(GetPawn()->GetActorLocation());
        SpawnTransform.SetRotation(FQuat::Identity);
        SpawnTransform.SetScale3D(FVector::OneVector);
        AWeaponActor* Weapon = GetWorld()->SpawnActorDeferred<AWeaponActor>(
            AWeaponActor::StaticClass(),
            SpawnTransform,
            this,
            GetPawn(),
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn
        );
        Weapon->InitalizeWeapon(Tag);
        Weapon->AttachToActor(GetPawn(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        Weapon->ViewWeaponMesh->AttachToComponent(PlayerCharacter->ArmMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("weapon"));
        Weapon->WeaponMesh->AttachToComponent(PlayerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("hand_r"));
        Weapon->FinishSpawning(SpawnTransform);

        GetAbilitySystemComponent()->GiveAbility(Weapon->PrimaryAbilitySpec);
        GetAbilitySystemComponent()->GiveAbility(Weapon->SecondaryAbilitySpec);
        GetAbilitySystemComponent()->GiveAbility(Weapon->ReloadAbilitySpec);
    }
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

const FGameplayTag ANLPlayerState::GetCurrentWeaponTag() const
{
    if (CurrentWeaponSlot <= WeaponTagSlot.Num())
    {
        return WeaponTagSlot[CurrentWeaponSlot];
    }
    return FGameplayTag();
}
