// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Player/NLPlayerComponent.h"

#include "Net/UnrealNetwork.h"
#include "NLGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Player/NLPlayerState.h"
#include "Characters/NLPlayerCharacter.h"
#include "Actors/WeaponActor.h"

UNLPlayerComponent::UNLPlayerComponent()
    : MaxWeaponSlotSize(3)
    , CurrentWeaponSlot(255)
    , bStartupWeaponInitFinished(false)
{
    PrimaryComponentTick.bCanEverTick = false;

    SetIsReplicatedByDefault(true);
}

void UNLPlayerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UNLPlayerComponent, CurrentWeaponSlot, COND_SimulatedOnly, REPNOTIFY_OnChanged);
    DOREPLIFETIME_CONDITION_NOTIFY(UNLPlayerComponent, WeaponActorSlot, COND_None, REPNOTIFY_OnChanged);
}

void UNLPlayerComponent::AddStartupWeapons()
{
    // On Server

    ANLPlayerState* PS = GetOwningPlayerState();
    UAbilitySystemComponent* ASC = GetASC();
    ANLPlayerCharacter* OwningPlayer = GetOwningPlayer();

    check(PS);
    check(ASC);
    check(OwningPlayer);

    int32 StartupWeaponNum = FMath::Min(int32(MaxWeaponSlotSize), PS->StartupWeapons.Num());
    WeaponActorSlot.Reset(StartupWeaponNum);

    for (int32 i = 0; i < StartupWeaponNum; i++)
    {
        const FGameplayTag& WeaponTag = PS->StartupWeapons[i];

        // Spawn Weapon
        FTransform SpawnTransform;
        SpawnTransform.SetLocation(OwningPlayer->GetActorLocation());
        SpawnTransform.SetRotation(FRotator(0.f, -90.f, 0.f).Quaternion());
        SpawnTransform.SetScale3D(FVector::OneVector);

        AWeaponActor* Weapon = GetWorld()->SpawnActorDeferred<AWeaponActor>(
            AWeaponActor::StaticClass(),
            SpawnTransform,
            OwningPlayer,
            OwningPlayer,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn
        );

        Weapon->InitalizeWeapon(WeaponTag);
        FAttachmentTransformRules AttachRule = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
        AttachRule.RotationRule = EAttachmentRule::KeepRelative;
        Weapon->AttachToComponent(GetOwningPlayer()->GetMesh(), AttachRule, FName("weapon_r"));

        Weapon->FinishSpawning(SpawnTransform);
        WeaponActorSlot.Add(Weapon);

        // Add Weapon Abilities
        FGameplayAbilitySpec PrimaryAbilitySpec = FGameplayAbilitySpec(Weapon->PrimaryAbilityClass, 1);
        PrimaryAbilitySpec.DynamicAbilityTags.AddTag(Input_Weapon_PrimaryAction);
        PrimaryAbilitySpec.DynamicAbilityTags.AddTag(Status_Weapon_Holstered);
        Weapon->PrimaryAbilitySpecHandle = ASC->GiveAbility(PrimaryAbilitySpec);

        FGameplayAbilitySpec SecondaryAbilitySpec = FGameplayAbilitySpec(Weapon->SecondaryAbilityClass, 1);
        SecondaryAbilitySpec.DynamicAbilityTags.AddTag(Input_Weapon_SecondaryAction);
        SecondaryAbilitySpec.DynamicAbilityTags.AddTag(Status_Weapon_Holstered);
        Weapon->SecondaryAbilitySpecHandle = ASC->GiveAbility(SecondaryAbilitySpec);

        FGameplayAbilitySpec ReloadAbilitySpec = FGameplayAbilitySpec(Weapon->ReloadAbilityClass, 1);
        ReloadAbilitySpec.DynamicAbilityTags.AddTag(Input_Weapon_Reload);
        ReloadAbilitySpec.DynamicAbilityTags.AddTag(Status_Weapon_Holstered);
        Weapon->ReloadAbilitySpecHandle = ASC->GiveAbility(ReloadAbilitySpec);
    }
}

void UNLPlayerComponent::OnRep_CurrentWeaponSlot(uint8 OldSlot)
{
    // 레플리케이트 되었다면 Simulated 액터라는 뜻이므로 3인칭 애니메이션과 3인칭 메시 설정.
    AWeaponActor* Weapon = GetWeaponActorAtSlot(CurrentWeaponSlot);
    if (!IsValid(Weapon) || !Weapon->IsInitialized())
    {
        return;
    }

    AWeaponActor* OldWeapon = GetWeaponActorAtSlot(OldSlot);
    GetOwningPlayer()->UpdateCharacterMesh(OldWeapon);
}

void UNLPlayerComponent::OnRep_WeaponActorSlot()
{
    if (!bStartupWeaponInitFinished)
    {
        ValidateStartupWeapons();
    }
}

ANLPlayerCharacter* UNLPlayerComponent::GetOwningPlayer() const
{
    return Cast<ANLPlayerCharacter>(GetOwner());
}

ANLPlayerState* UNLPlayerComponent::GetOwningPlayerState() const
{
    if (IsValid(GetOwningPlayer()))
    {
        return GetOwningPlayer()->GetPlayerState<ANLPlayerState>();
    }
    return nullptr;
}

UAbilitySystemComponent* UNLPlayerComponent::GetASC() const
{
    if (GetOwningPlayerState())
    {
        return GetOwningPlayerState()->GetAbilitySystemComponent();
    }
    return nullptr;
}

AWeaponActor* UNLPlayerComponent::GetWeaponActorAtSlot(uint8 Slot) const
{
    if (Slot < WeaponActorSlot.Num())
    {
        return WeaponActorSlot[Slot];
    }
    return nullptr;
}

AWeaponActor* UNLPlayerComponent::GetCurrentWeaponActor() const
{
    return GetWeaponActorAtSlot(CurrentWeaponSlot);
}

const FGameplayTag UNLPlayerComponent::GetWeaponTagAtSlot(uint8 Slot) const
{
    if (IsValid(GetWeaponActorAtSlot(Slot)))
    {
        return GetWeaponActorAtSlot(Slot)->GetWeaponTag();
    }
    return FGameplayTag();
}

const FGameplayTag UNLPlayerComponent::GetCurrentWeaponTag() const
{
    return GetWeaponTagAtSlot(CurrentWeaponSlot);
}

void UNLPlayerComponent::WeaponAdded(AWeaponActor* Weapon)
{
    if (!bStartupWeaponInitFinished)
    {
        InitializedStartupWeapons.AddUnique(Weapon);
        ValidateStartupWeapons();
    }
}

void UNLPlayerComponent::ValidateStartupWeapons()
{
    // On Client

    if (GetOwnerRole() >= ROLE_Authority || bStartupWeaponInitFinished)
    {
        return;
    }

    ANLPlayerState* PS = GetOwningPlayerState();
    if (!PS)
    {
        // PlayerState is not replicated yet.
        return;
    }

    int32 StartupWeaponNum = FMath::Min(int32(MaxWeaponSlotSize), GetOwningPlayerState()->StartupWeapons.Num());
    if (WeaponActorSlot.Num() == StartupWeaponNum && InitializedStartupWeapons.Num() == StartupWeaponNum)
    {
        bool bAllInitalizedAndValid = true;
        for (int32 i = 0; i < StartupWeaponNum; i++)
        {
            const AWeaponActor* InitializedWeapon = InitializedStartupWeapons[i];
            if (!InitializedWeapon->IsInitialized() || !WeaponActorSlot.Contains(InitializedWeapon))
            {
                bAllInitalizedAndValid = false;
                break;
            }
        }

        bStartupWeaponInitFinished = bAllInitalizedAndValid;
        if (bStartupWeaponInitFinished)
        {
            if (GetOwnerRole() == ROLE_AutonomousProxy)
            {
                // Try Activate ChangeWeapon Ability
                FGameplayTagContainer TagContainer(Ability_WeaponChange_1);
                GetASC()->TryActivateAbilitiesByTag(TagContainer);
            }
            else if (GetOwnerRole() == ROLE_SimulatedProxy)
            {
                /**
                * 캐릭터 메시는 들고있는 무기가 바뀔때 업데이트 되는데,
                * 늦게 들어온 플레이어 입장에서는 다른 Simulated 캐릭터의 무기가 바뀌기 전까지는 업데이트가 안되므로 기본 상태로 유지됨.
                * 따라서 Simulated 캐릭터의 플레이어 스테이트가 레플리케이트 되면 한번 업데이트 함.
                */
                GetOwningPlayer()->UpdateCharacterMesh();
            }

            // Clear up validation data
            InitializedStartupWeapons.Empty();
        }
    }
}

void UNLPlayerComponent::ChangeWeaponSlot_Simple(int32 NewWeaponSlot)
{
    // On Server and Client by GameplayAbility

    if (GetOwnerRole() == ROLE_Authority)
    {
        if (AWeaponActor* PrevWeapon = GetCurrentWeaponActor())
        {
            GetASC()->CancelAbilityHandle(PrevWeapon->PrimaryAbilitySpecHandle);
            FGameplayAbilitySpec* PAS = GetASC()->FindAbilitySpecFromHandle(PrevWeapon->PrimaryAbilitySpecHandle);
            PAS->DynamicAbilityTags.AddTag(Status_Weapon_Holstered);
            GetASC()->MarkAbilitySpecDirty(*PAS);
        }
        if (AWeaponActor* NewWeapon = GetWeaponActorAtSlot(NewWeaponSlot))
        {
            FGameplayAbilitySpec* PAS = GetASC()->FindAbilitySpecFromHandle(NewWeapon->PrimaryAbilitySpecHandle);
            PAS->DynamicAbilityTags.RemoveTag(Status_Weapon_Holstered);
            GetASC()->MarkAbilitySpecDirty(*PAS);
        }
    }
    CurrentWeaponSlot = NewWeaponSlot;

    GetOwningPlayer()->OnCurrentWeaponChanged(GetCurrentWeaponTag());
}
