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
    , CurrentWeaponSlot(0)
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
    DOREPLIFETIME_CONDITION_NOTIFY(ANLPlayerState, WeaponActorSlot, COND_None, REPNOTIFY_OnChanged);
}

void ANLPlayerState::BeginPlay()
{
    Super::BeginPlay();
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
            if (AWeaponActor* OldWeapon = GetWeaponAtSlot(OldSlot))
            {
                OldWeapon->SetActorHiddenInGame(true);
            }
            if (AWeaponActor* CurrentWeapon = GetCurrentWeapon())
            {
                CurrentWeapon->SetActorHiddenInGame(false);
            }

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

    int32 WeaponNum = FMath::Min(int32(MaxSlotSize), StartupWeapons.Num());
    WeaponActorSlot.Reset(WeaponNum);

    for (const FGameplayTag& Tag : StartupWeapons)
    {
        FTransform SpawnTransform;
        SpawnTransform.SetLocation(GetPawn()->GetActorLocation());
        SpawnTransform.SetRotation(FRotator(0.f, -90.f, 0.f).Quaternion());
        SpawnTransform.SetScale3D(FVector::OneVector);
        AWeaponActor* Weapon = GetWorld()->SpawnActorDeferred<AWeaponActor>(
            AWeaponActor::StaticClass(),
            SpawnTransform,
            GetPawn(),
            GetPawn(),
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn
        );
        Weapon->InitalizeWeapon(Tag);
        FAttachmentTransformRules AttachRule = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
        AttachRule.RotationRule = EAttachmentRule::KeepRelative;
        Weapon->AttachToComponent(PlayerCharacter->GetMesh(), AttachRule, FName("weapon_r"));
        Weapon->FinishSpawning(SpawnTransform);
        WeaponActorSlot.Add(Weapon);

        FGameplayAbilitySpec PrimaryAbilitySpec = FGameplayAbilitySpec(Weapon->PrimaryAbilityClass, 1);
        PrimaryAbilitySpec.DynamicAbilityTags.AddTag(Input_Weapon_PrimaryAction);
        PrimaryAbilitySpec.DynamicAbilityTags.AddTag(Status_Weapon_Holstered);
        Weapon->PrimaryAbilitySpecHandle = GetAbilitySystemComponent()->GiveAbility(PrimaryAbilitySpec);

        FGameplayAbilitySpec SecondaryAbilitySpec = FGameplayAbilitySpec(Weapon->PrimaryAbilityClass, 1);
        SecondaryAbilitySpec.DynamicAbilityTags.AddTag(Input_Weapon_SecondaryAction);
        SecondaryAbilitySpec.DynamicAbilityTags.AddTag(Status_Weapon_Holstered);
        Weapon->SecondaryAbilitySpecHandle = GetAbilitySystemComponent()->GiveAbility(SecondaryAbilitySpec);

        FGameplayAbilitySpec ReloadAbilitySpec = FGameplayAbilitySpec(Weapon->PrimaryAbilityClass, 1);
        ReloadAbilitySpec.DynamicAbilityTags.AddTag(Input_Weapon_Reload);
        ReloadAbilitySpec.DynamicAbilityTags.AddTag(Status_Weapon_Holstered);
        Weapon->ReloadAbilitySpecHandle = GetAbilitySystemComponent()->GiveAbility(ReloadAbilitySpec);
    }

    if (WeaponNum > 0)
    {
        ChangeWeaponSlot(0);
    }
}

UAbilitySystemComponent* ANLPlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

AWeaponActor* ANLPlayerState::GetCurrentWeapon() const
{
    return GetWeaponAtSlot(CurrentWeaponSlot);
}

AWeaponActor* ANLPlayerState::GetWeaponAtSlot(uint8 InSlot) const
{
    if (0 <= InSlot && InSlot < WeaponActorSlot.Num())
    {
        return WeaponActorSlot[InSlot];
    }
    return nullptr;
}

void ANLPlayerState::ChangeWeaponSlot(int32 NewWeaponSlot)
{
    // On Server and Client by GameplayAbility

    const FGameplayTag PrevWeaponTag = GetCurrentWeaponTag();

    if (HasAuthority())
    {
        if (AWeaponActor* PrevWeapon = GetCurrentWeapon())
        {
            GetAbilitySystemComponent()->CancelAbilityHandle(PrevWeapon->PrimaryAbilitySpecHandle);
            FGameplayAbilitySpec* PS = GetAbilitySystemComponent()->FindAbilitySpecFromHandle(PrevWeapon->PrimaryAbilitySpecHandle);
            PS->DynamicAbilityTags.AddTag(Status_Weapon_Holstered);
            GetAbilitySystemComponent()->MarkAbilitySpecDirty(*PS);
        }
        if (AWeaponActor* NewWeapon = GetWeaponAtSlot(NewWeaponSlot))
        {
            FGameplayAbilitySpec* PS = GetAbilitySystemComponent()->FindAbilitySpecFromHandle(NewWeapon->PrimaryAbilitySpecHandle);
            PS->DynamicAbilityTags.RemoveTag(Status_Weapon_Holstered);
            GetAbilitySystemComponent()->MarkAbilitySpecDirty(*PS);
        }
    }
    CurrentWeaponSlot = NewWeaponSlot;
    CurrentWeaponSlotChanged.Broadcast(CurrentWeaponSlot);

    ANLPlayerCharacter* PlayerCharacter = Cast<ANLPlayerCharacter>(GetPawn());
    PlayerCharacter->OnCurrentWeaponChanged(GetCurrentWeaponTag());
}

const FGameplayTag ANLPlayerState::GetCurrentWeaponTag() const
{
    if (CurrentWeaponSlot < WeaponActorSlot.Num() && IsValid(WeaponActorSlot[CurrentWeaponSlot]))
    {
        return WeaponActorSlot[CurrentWeaponSlot]->GetWeaponTag();
    }
    return FGameplayTag();
}
