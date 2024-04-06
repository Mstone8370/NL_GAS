// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/NLCharacterComponent.h"

#include "Net/UnrealNetwork.h"
#include "NLGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Player/NLPlayerState.h"
#include "Characters/NLPlayerCharacter.h"
#include "Characters/NLCharacterBase.h"
#include "Actors/WeaponActor.h"
#include "NLFunctionLibrary.h"
#include "TimerManager.h"

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

bool UNLCharacterComponent::CanChangeWeaponSlot(int32 NewWeaponSlot) const
{
    if (!bIsChangingWeapon && CurrentWeaponSlot == NewWeaponSlot)
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

void UNLCharacterComponent::OnWeaponHolstered()
{
    bIsChangingWeapon = false;

    const AWeaponActor* PrevWeapon = GetCurrentWeaponActor();  // Could be nullptr
    const AWeaponActor* ChangedWeapon = GetWeaponActorAtSlot(WeaponChangePendingSlot);
    const bool bDrawFirst = !ChangedWeapon->IsEverDrawn();

    if (GetOwnerRole() == ROLE_AutonomousProxy)
    {
        if (ANLPlayerCharacter* PlayerCharacter = Cast<ANLPlayerCharacter>(GetOwningPlayer()))
        {
            PlayerCharacter->UpdateViewWeaponAndAnimLayer(
                ChangedWeapon->GetViewWeaponMesh(),
                ChangedWeapon->GetWeaponAnimInstanceClass(),
                ChangedWeapon->GetArmsAnimLayerClass()
            );
        }
    }
    CurrentWeaponSlot = WeaponChangePendingSlot;

    // Draw New Weapon
    const FGameplayTag& CurrentWeaponTag = GetCurrentWeaponTag();
    const FGameplayTag MontageTag = bDrawFirst ? Montage_Weapon_DrawFirst : Montage_Weapon_Draw;
    FTimerDelegate TimerDelegate;
    TimerDelegate.BindUObject(this, &UNLCharacterComponent::OnWeaponDrawn);
    PlayCurrentWeaponMontageAndSetCallback(MontageTag, DrawTimerHandle, TimerDelegate);
}

void UNLCharacterComponent::OnWeaponDrawn()
{
    if (AWeaponActor* Weapon = GetCurrentWeaponActor())
    {
        Weapon->Drawn();

        if (GetOwnerRole() == ROLE_Authority)
        {
            FScopedAbilityListLock ActiveScopeLock(*GetASC());

            FGameplayAbilitySpec* PAS = GetASC()->FindAbilitySpecFromHandle(Weapon->PrimaryAbilitySpecHandle);
            PAS->DynamicAbilityTags.RemoveTag(Status_Weapon_Holstered);
            GetASC()->MarkAbilitySpecDirty(*PAS);

            /*
            FGameplayAbilitySpec* SAS = GetASC()->FindAbilitySpecFromHandle(Weapon->SecondaryAbilitySpecHandle);
            SAS->DynamicAbilityTags.RemoveTag(Status_Weapon_Holstered);
            GetASC()->MarkAbilitySpecDirty(*SAS);

            FGameplayAbilitySpec* RAS = GetASC()->FindAbilitySpecFromHandle(Weapon->ReloadAbilitySpecHandle);
            RAS->DynamicAbilityTags.RemoveTag(Status_Weapon_Holstered);
            GetASC()->MarkAbilitySpecDirty(*RAS);
            */
        }
    }
}

void UNLCharacterComponent::PlayCurrentWeaponMontageAndSetCallback(const FGameplayTag& MontageTag, FTimerHandle& OutTimerHandle, FTimerDelegate TimerDelegate)
{
    OutTimerHandle = FTimerHandle();

    const FGameplayTag& CurrentWeaponTag = GetCurrentWeaponTag();
    if (!CurrentWeaponTag.IsValid())
    {
        return;
    }

    if (const FTaggedAnimMontageInfo* MontageInfo = UNLFunctionLibrary::GetAnimMontageByTag(this, CurrentWeaponTag, MontageTag))
    {
        const float MontageTimeOverride = MontageInfo->PlayLengthOverride;

        // TODO: ������ �ִ� ��Ÿ�� PlayRate�� Arms�� ��Ÿ�ֿ� �°� ������ �� ���.
        // �� ��Ÿ���� ���̴� ���� �ٸ��� �ִٴ� ���� ����.

        if (UAnimMontage* WeaponAnimMontage = MontageInfo->WeaponAnimMontage.LoadSynchronous())
        {
            GetOwningPlayer()->PlayWeaponAnimMontage(WeaponAnimMontage);  // Only on Client
        }
        if (UAnimMontage* ArmsAnimMontage = MontageInfo->ArmsAnimMontage.LoadSynchronous())
        {
            const float MontagePlayLength = ArmsAnimMontage->GetPlayLength();
            const float ActualMontageLength = MontageTimeOverride > 0.f ? MontageTimeOverride : MontagePlayLength;
            const float MontagePlayRate =  MontagePlayLength / ActualMontageLength;
            GetOwningPlayer()->PlayArmsAnimMontage(ArmsAnimMontage, MontagePlayRate);  // Only on Client

            // On Server and Client
            if (ActualMontageLength > 0.f)
            {
                GetWorld()->GetTimerManager().SetTimer(
                    OutTimerHandle,
                    TimerDelegate,
                    ActualMontageLength - .1f,
                    false
                );
            }
            else
            {
                TimerDelegate.ExecuteIfBound();  // Maybe?
            }
        }
    }
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
            else if (GetOwnerRole() == ROLE_SimulatedProxy)
            {
                // Update Simulated Character Mesh
                // ���߿� ������ Ŭ���̾�Ʈ ���忡���� ������ �����ߴ� �÷��̾���� ���� ������ �°� ������Ʈ
                UpdateOwningCharacterMesh();
            }

            // Clear up validation data
            InitializedStartupWeapons.Empty();
        }
    }
}

bool UNLCharacterComponent::TryChangeWeaponSlot(int32 NewWeaponSlot)
{
    // On Server and Client by GameplayAbility

    if (!IsValid(GetCurrentWeaponActor()))
    {
        // Start up
        OnWeaponHolstered();
        return true;
    }

    if (!CanChangeWeaponSlot(NewWeaponSlot))
    {
        return false;
    }

    WeaponChangePendingSlot = NewWeaponSlot;

    if (bIsChangingWeapon)
    {
        // TODO: ���� Holster�ϰ��ִ� ����� ���� ����� ������ �õ��ϴ� ���
        // ���� ������ ����ϰ� ���¸� �ǵ����� ��� �����غ���.

        return true;
    }
    bIsChangingWeapon = true;

    // Set Ability State
    if (GetOwnerRole() == ROLE_Authority)
    {
        FScopedAbilityListLock ActiveScopeLock(*GetASC());

        if (AWeaponActor* PrevWeapon = GetCurrentWeaponActor())
        {
            GetASC()->CancelAbilityHandle(PrevWeapon->PrimaryAbilitySpecHandle);
            //GetASC()->CancelAbilityHandle(PrevWeapon->SecondaryAbilitySpecHandle);
            //GetASC()->CancelAbilityHandle(PrevWeapon->ReloadAbilitySpecHandle);

            FGameplayAbilitySpec* PAS = GetASC()->FindAbilitySpecFromHandle(PrevWeapon->PrimaryAbilitySpecHandle);
            PAS->DynamicAbilityTags.AddTag(Status_Weapon_Holstered);
            GetASC()->MarkAbilitySpecDirty(*PAS);

            /*
            FGameplayAbilitySpec* SAS = GetASC()->FindAbilitySpecFromHandle(PrevWeapon->SecondaryAbilitySpecHandle);
            SAS->DynamicAbilityTags.AddTag(Status_Weapon_Holstered);
            GetASC()->MarkAbilitySpecDirty(*SAS);

            FGameplayAbilitySpec* RAS = GetASC()->FindAbilitySpecFromHandle(PrevWeapon->ReloadAbilitySpecHandle);
            RAS->DynamicAbilityTags.AddTag(Status_Weapon_Holstered);
            GetASC()->MarkAbilitySpecDirty(*RAS);
            */
        }
    }

    // Stop any playing anim montage. e.g. fire montage.
    GetOwningPlayer()->StopArmsAnimMontage();
    GetOwningPlayer()->StopWeaponAnimMontage();

    const FGameplayTag& CurrentWeaponTag = GetCurrentWeaponTag();
    FTimerDelegate TimerDelegate;
    TimerDelegate.BindUObject(this, &UNLCharacterComponent::OnWeaponHolstered);
    PlayCurrentWeaponMontageAndSetCallback(Montage_Weapon_Holster, HolsterTimerHandle, TimerDelegate);

    return true;
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
