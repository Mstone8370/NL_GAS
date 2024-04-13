// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/NLCharacterComponent.h"

#include "Net/UnrealNetwork.h"
#include "NLGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/NLAbilitySystemComponent.h"
#include "Player/NLPlayerState.h"
#include "Characters/NLPlayerCharacter.h"
#include "Characters/NLCharacterBase.h"
#include "Actors/WeaponActor.h"
#include "NLFunctionLibrary.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

UNLCharacterComponent::UNLCharacterComponent()
    : MaxWeaponSlotSize(3)
    , CurrentWeaponSlot(255)
    , bStartupWeaponInitFinished(false)
{
    PrimaryComponentTick.bCanEverTick = false;

    SetIsReplicatedByDefault(true);
}

void UNLCharacterComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UNLCharacterComponent, CurrentWeaponSlot, COND_SimulatedOnly, REPNOTIFY_OnChanged);
    DOREPLIFETIME_CONDITION_NOTIFY(UNLCharacterComponent, WeaponActorSlot, COND_None, REPNOTIFY_OnChanged);
}

void UNLCharacterComponent::AddStartupWeapons()
{
    // On Server

    ANLPlayerState* PS = GetOwningPlayerState();
    UAbilitySystemComponent* ASC = GetASC();
    ANLCharacterBase* OwningPlayer = GetOwningPlayer();

    // TODO: This code runs on server. Use alternative way to check pointers instead of check() macro.
    check(PS);
    check(ASC);
    check(OwningPlayer);

    int32 StartupWeaponNum = FMath::Min(int32(MaxWeaponSlotSize), PS->StartupWeapons.Num());
    WeaponActorSlot.Reset(StartupWeaponNum);

    FScopedAbilityListLock ActiveScopeLock(*ASC);

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
        GetNLASC()->WeaponAdded(Weapon);
    }
}

void UNLCharacterComponent::OnRep_CurrentWeaponSlot(uint8 OldSlot)
{
    // ���ø�����Ʈ �Ǿ��ٸ� Simulated ���Ͷ�� ���̹Ƿ� 3��Ī �ִϸ��̼ǰ� 3��Ī �޽� ����.
    AWeaponActor* Weapon = GetWeaponActorAtSlot(CurrentWeaponSlot);
    if (!IsValid(Weapon) || !Weapon->IsInitialized())
    {
        return;
    }

    AWeaponActor* OldWeapon = GetWeaponActorAtSlot(OldSlot);
    UpdateOwningCharacterMesh(OldWeapon);
}

void UNLCharacterComponent::OnRep_WeaponActorSlot()
{
    if (!bStartupWeaponInitFinished)
    {
        ValidateStartupWeapons();
    }
}

void UNLCharacterComponent::UpdateOwningCharacterMesh(AWeaponActor* OldWeaponActor)
{
    // Hide Previous Weapon
    if (IsValid(OldWeaponActor))
    {
        OldWeaponActor->SetActorHiddenInGame(true);
    }

    // Show Current Weapon
    AWeaponActor* CurrentWeapon = GetCurrentWeaponActor();
    if (IsValid(CurrentWeapon))
    {
        CurrentWeapon->SetActorHiddenInGame(false);
    }

    // Change Character Mesh AnimBP
    const FGameplayTag WeaponTag = GetCurrentWeaponTag();
    if (WeaponTag.IsValid())
    {
        const FWeaponInfo* Info = UNLFunctionLibrary::GetWeaponInfoByTag(this, WeaponTag);
        if (Info && Info->CharacterMeshAnimBP)
        {
            GetOwningPlayer()->GetMesh()->SetAnimInstanceClass(Info->CharacterMeshAnimBP);
        }
    }
}

void UNLCharacterComponent::CancelWeaponSwap()
{
    bIsSwappingWeapon = false;

    GetWorld()->GetTimerManager().ClearTimer(HolsterTimerHandle);

    UAnimMontage* ArmsHolsterMontage = GetOwningPlayer()->GetCurrentArmsMontage();
    GetOwningPlayer()->StopArmsAnimMontage();
    GetOwningPlayer()->StopWeaponAnimMontage(ArmsHolsterMontage);

    // Draw again
    const bool bDrawFirst = !GetCurrentWeaponActor()->IsEverDrawn();
    const FGameplayTag& CurrentWeaponTag = GetCurrentWeaponTag();
    const FGameplayTag MontageTag = bDrawFirst ? Montage_Weapon_DrawFirst : Montage_Weapon_Draw;
    FTimerDelegate TimerDelegate;
    TimerDelegate.BindUObject(this, &UNLCharacterComponent::OnWeaponDrawn);
    PlayCurrentWeaponMontageAndSetCallback(MontageTag, DrawTimerHandle, TimerDelegate);
}

void UNLCharacterComponent::OnWeaponHolstered()
{
    bIsSwappingWeapon = false;

    if (const AWeaponActor* PrevWeapon = GetCurrentWeaponActor())  // Could be nullptr
    {
        GetNLASC()->WeaponHolstered(PrevWeapon);
    }
    const AWeaponActor* ChangedWeapon = GetWeaponActorAtSlot(WeaponSwapPendingSlot);

    CurrentWeaponSlot = WeaponSwapPendingSlot;

    GetOwningPlayer()->UpdateViewWeaponAndAnimLayer(
        ChangedWeapon->GetViewWeaponMesh(),
        ChangedWeapon->GetWeaponAnimInstanceClass(),
        ChangedWeapon->GetArmsAnimLayerClass()
    );

    // Draw New Weapon
    const bool bDrawFirst = !ChangedWeapon->IsEverDrawn();
    const FGameplayTag& CurrentWeaponTag = GetCurrentWeaponTag();
    const FGameplayTag MontageTag = bDrawFirst ? Montage_Weapon_DrawFirst : Montage_Weapon_Draw;
    FTimerDelegate TimerDelegate;
    TimerDelegate.BindUObject(this, &UNLCharacterComponent::OnWeaponDrawn);
    PlayCurrentWeaponMontageAndSetCallback(MontageTag, DrawTimerHandle, TimerDelegate);
}

void UNLCharacterComponent::OnWeaponDrawn()
{
    // This function could be called by Anim Notify before timer.
    GetWorld()->GetTimerManager().ClearTimer(DrawTimerHandle);

    GetASC()->SetLooseGameplayTagCount(Ability_Block_Weapon, 0);
    GetNLASC()->WeaponDrawn(GetCurrentWeaponActor());

    GetCurrentWeaponActor()->Drawn();
}

ANLCharacterBase* UNLCharacterComponent::GetOwningCharacter() const
{
    return Cast<ANLCharacterBase>(GetOwner());
}

ANLPlayerCharacter* UNLCharacterComponent::GetOwningPlayer() const
{
    return Cast<ANLPlayerCharacter>(GetOwningCharacter());
}

ANLPlayerState* UNLCharacterComponent::GetOwningPlayerState() const
{
    if (IsValid(GetOwningPlayer()))
    {
        return GetOwningPlayer()->GetPlayerState<ANLPlayerState>();
    }
    return nullptr;
}

UAbilitySystemComponent* UNLCharacterComponent::GetASC() const
{
    if (GetOwningPlayerState())
    {
        return GetOwningPlayerState()->GetAbilitySystemComponent();
    }
    return nullptr;
}

UNLAbilitySystemComponent* UNLCharacterComponent::GetNLASC() const
{
    if (GetASC())
    {
        return Cast<UNLAbilitySystemComponent>(GetASC());
    }
    return nullptr;
}

AWeaponActor* UNLCharacterComponent::GetWeaponActorAtSlot(uint8 Slot) const
{
    if (Slot < WeaponActorSlot.Num())
    {
        return WeaponActorSlot[Slot];
    }
    return nullptr;
}

AWeaponActor* UNLCharacterComponent::GetCurrentWeaponActor() const
{
    return GetWeaponActorAtSlot(CurrentWeaponSlot);
}

const FGameplayTag UNLCharacterComponent::GetWeaponTagAtSlot(uint8 Slot) const
{
    if (IsValid(GetWeaponActorAtSlot(Slot)))
    {
        return GetWeaponActorAtSlot(Slot)->GetWeaponTag();
    }
    return FGameplayTag();
}

const FGameplayTag UNLCharacterComponent::GetCurrentWeaponTag() const
{
    return GetWeaponTagAtSlot(CurrentWeaponSlot);
}

void UNLCharacterComponent::WeaponAdded(AWeaponActor* Weapon)
{
    if (!bStartupWeaponInitFinished)
    {
        InitializedStartupWeapons.AddUnique(Weapon);
        ValidateStartupWeapons();
    }
    else
    {
        // TODO: New weapon equipped while playing
    }
}

void UNLCharacterComponent::ValidateStartupWeapons()
{
    // For Client, but also works on Server.

    if (bStartupWeaponInitFinished)
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
            if (GetOwnerRole() == ROLE_SimulatedProxy)
            {
                // Update Simulated Character Mesh
                // ���߿� ������ Ŭ���̾�Ʈ ���忡���� ������ �����ߴ� �÷��̾���� ���� ������ �°� ������Ʈ
                UpdateOwningCharacterMesh();
            }
            else
            {
                UAbilitySystemComponent* ASC = GetASC();
                FTimerHandle TimerHandle;
                FTimerDelegate TimerDelegate;
                TimerDelegate.BindLambda(
                    [ASC]()
                    {
                        // Try Activate ChangeWeapon Ability
                        FGameplayTagContainer TagContainer(Ability_WeaponChange_1);
                        ASC->TryActivateAbilitiesByTag(TagContainer);
                    }
                );
                GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 1.f, false);
            }

            // Clear up validation data
            InitializedStartupWeapons.Empty();
        }
    }
}

bool UNLCharacterComponent::CanSwapWeaponSlot(int32 NewWeaponSlot) const
{
    if (!bIsSwappingWeapon && CurrentWeaponSlot == NewWeaponSlot)
    {
        return false;
    }

    if (0 <= NewWeaponSlot && NewWeaponSlot < WeaponActorSlot.Num())
    {
        AWeaponActor* Weapon = WeaponActorSlot[NewWeaponSlot];
        if (IsValid(Weapon) && Weapon->IsInitialized())
        {
            return true;
        }
    }
    return false;
}

void UNLCharacterComponent::TrySwapWeaponSlot(int32 NewWeaponSlot)
{
    // On Server and Client by GameplayAbility

    if (!IsValid(GetCurrentWeaponActor()))
    {
        // Start up
        OnWeaponHolstered();
        return;
    }

    if (!CanSwapWeaponSlot(NewWeaponSlot))
    {
        return;
    }

    WeaponSwapPendingSlot = NewWeaponSlot;

    if (bIsSwappingWeapon)
    {
        if (WeaponSwapPendingSlot == CurrentWeaponSlot)
        {
            CancelWeaponSwap();
        }
        return;
    }
    bIsSwappingWeapon = true;

    // Block Weapon Ability
    GetASC()->AddLooseGameplayTag(Ability_Block_Weapon);
    
    // ���� ������ ���� ���Ⱑ drawn �Ǳ� ���� ���۵� �� ����.
    // �׷� ��쿡�� draw Ÿ�̸Ӱ� �����Ǿ������Ƿ� holster���� ���Ⱑ drawn �Ǿ ��� �����ϰ� ��.
    // ���� draw Ÿ�̸Ӹ� clear��.
    GetWorld()->GetTimerManager().ClearTimer(DrawTimerHandle);

    const FGameplayTag& CurrentWeaponTag = GetCurrentWeaponTag();
    FTimerDelegate TimerDelegate;
    TimerDelegate.BindUObject(this, &UNLCharacterComponent::OnWeaponHolstered);
    PlayCurrentWeaponMontageAndSetCallback(Montage_Weapon_Holster, HolsterTimerHandle, TimerDelegate);
}

bool UNLCharacterComponent::CanAttack() const
{
    if (AWeaponActor* CurrentWeapon = GetCurrentWeaponActor())
    {
        return CurrentWeapon->CanAttack();
    }
    return false;
}

bool UNLCharacterComponent::CommitWeaponCost()
{
    if (CanAttack())
    {
        return GetCurrentWeaponActor()->CommitWeaponCost();
    }
    return false;
}

float UNLCharacterComponent::PlayCurrentWeaponMontage(const FGameplayTag& MontageTag)
{
    const FGameplayTag& CurrentWeaponTag = GetCurrentWeaponTag();
    const FTaggedAnimMontageInfo* MontageInfo = UNLFunctionLibrary::GetAnimMontageByTag(this, CurrentWeaponTag, MontageTag);
    if (!MontageInfo)
    {
        return 0.f;
    }

    const float MontageTimeOverride = MontageInfo->PlayLengthOverride;

    UAnimMontage* ArmsAnimMontage = MontageInfo->ArmsAnimMontage.LoadSynchronous();
    UAnimMontage* WeaponAnimMontage = MontageInfo->WeaponAnimMontage.LoadSynchronous();  // Could be nullptr

    const float MontagePlayLength = ArmsAnimMontage ? ArmsAnimMontage->GetPlayLength() : 0.f;
    const float OverriddenPlayLength = MontageTimeOverride > 0.f ? MontageTimeOverride : MontagePlayLength;
    const float MontagePlayRate = UKismetMathLibrary::SafeDivide(MontagePlayLength, OverriddenPlayLength);

    // Only on Client
    GetOwningPlayer()->PlayWeaponAnimMontage(WeaponAnimMontage, MontagePlayRate);
    GetOwningPlayer()->PlayArmsAnimMontage(ArmsAnimMontage, MontagePlayRate);

    return OverriddenPlayLength;
}

float UNLCharacterComponent::PlayCurrentWeaponMontageAndSetCallback(const FGameplayTag& MontageTag, FTimerHandle& OutTimerHandle, FTimerDelegate TimerDelegate, bool bOnBlendOut)
{
    OutTimerHandle = FTimerHandle();

    float MontagePlayLength = PlayCurrentWeaponMontage(MontageTag);

    // On Server and Client
    if (MontagePlayLength > 0.f)
    {
        const float BlendOutTime = GetOwningPlayer()->GetCurrentArmsMontage()->GetDefaultBlendOutTime();
        GetWorld()->GetTimerManager().SetTimer(
            OutTimerHandle,
            TimerDelegate,
            bOnBlendOut ? MontagePlayLength - BlendOutTime : MontagePlayLength,
            false
        );
    }
    else
    {
        TimerDelegate.ExecuteIfBound();
    }

    return MontagePlayLength;
}