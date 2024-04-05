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
    // 레플리케이트 되었다면 Simulated 액터라는 뜻이므로 3인칭 애니메이션과 3인칭 메시 설정.
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
    CurrentWeaponSlot = WeaponChangePendingSlot;

    if (GetOwnerRole() == ROLE_AutonomousProxy)
    {
        if (ANLPlayerCharacter* PlayerCharacter = Cast<ANLPlayerCharacter>(GetOwningPlayer()))
        {
            AWeaponActor* ChangedWeapon = GetCurrentWeaponActor();
            PlayerCharacter->UpdateViewWeapon(
                ChangedWeapon->GetViewWeaponMesh(),
                ChangedWeapon->GetArmsAnimInstance()
            );
        }
    }

    // Draw New Weapon
    const FGameplayTag& CurrentWeaponTag = GetCurrentWeaponTag();
    const FWeaponAnims* ArmsAnimInfo = UNLFunctionLibrary::GetArmsAnimInfoByTag(this, CurrentWeaponTag);
    const FWeaponAnims* WeaponAnimInfo = UNLFunctionLibrary::GetWeaponAnimInfoByTag(this, CurrentWeaponTag);

    // Play Arms Anim Montage. Should be exist.
    if (UAnimMontage* ArmsDrawAnimMontage = ArmsAnimInfo->Draw.LoadSynchronous())
    {
        const float MontagePlayLength = ArmsDrawAnimMontage->GetPlayLength();
        const float MontageTimeOverride = ArmsAnimInfo->DrawTime;

        const float MontagePlayRate = MontageTimeOverride > 0.f ? MontagePlayLength / MontageTimeOverride : 1.f;
        const float ActualMontageLength = GetOwningPlayer()->PlayArmsAnimMontage(ArmsDrawAnimMontage, MontagePlayRate);

        if (ActualMontageLength > 0.f)
        {
            GetWorld()->GetTimerManager().SetTimer(
                DrawTimerHandle,
                this,
                &UNLCharacterComponent::OnWeaponDrawn,
                ActualMontageLength
            );
        }
        else
        {
            OnWeaponDrawn();
        }
    }
    // Play Weapon Anim Montage if exist.
    if (UAnimMontage* WeaponDrawAnimMontage = WeaponAnimInfo->Draw.LoadSynchronous())
    {
        const float MontagePlayLength = WeaponDrawAnimMontage->GetPlayLength();
        const float MontageTimeOverride = WeaponAnimInfo->DrawTime;

        const float MontagePlayRate = MontageTimeOverride > 0.f ? MontagePlayLength / MontageTimeOverride : 1.f;
        GetOwningPlayer()->PlayWeaponAnimMontage(WeaponDrawAnimMontage, MontagePlayRate);
    }
}

void UNLCharacterComponent::OnWeaponDrawn()
{
    if (GetOwnerRole() == ROLE_Authority)
    {
        if (AWeaponActor* Weapon = GetCurrentWeaponActor())
        {
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
                // Try Activate ChangeWeapon Ability
                FGameplayTagContainer TagContainer(Ability_WeaponChange_1);
                GetASC()->TryActivateAbilitiesByTag(TagContainer);
            }
            else if (GetOwnerRole() == ROLE_SimulatedProxy)
            {
                // Update Simulated Character Mesh
                // 나중에 접속한 클라이언트 입장에서도 기존에 접속했던 플레이어들의 무기 정보에 맞게 업데이트
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
        // TODO: 현재 Holster하고있는 무기와 같은 무기로 변경을 시도하는 경우
        // 무기 변경을 취소하고 상태를 되돌리는 방법 생각해보기.

        return true;
    }
    bIsChangingWeapon = true;

    const FGameplayTag& CurrentWeaponTag = GetCurrentWeaponTag();
    const FWeaponAnims* ArmsAnimInfo = UNLFunctionLibrary::GetArmsAnimInfoByTag(this, CurrentWeaponTag);
    const FWeaponAnims* WeaponAnimInfo = UNLFunctionLibrary::GetWeaponAnimInfoByTag(this, CurrentWeaponTag);

    // Play Arms Anim Montage. Should be exist.
    if (UAnimMontage* ArmsHolsterAnimMontage = ArmsAnimInfo->Holster.LoadSynchronous())
    {
        const float MontagePlayLength = ArmsHolsterAnimMontage->GetPlayLength();
        const float MontageTimeOverride = ArmsAnimInfo->HolsterTime;

        const float MontagePlayRate = MontageTimeOverride > 0.f ? MontagePlayLength / MontageTimeOverride : 1.f;
        const float ActualMontageLength = GetOwningPlayer()->PlayArmsAnimMontage(ArmsHolsterAnimMontage, MontagePlayRate);

        if (ActualMontageLength > 0.f)
        {
            GetWorld()->GetTimerManager().SetTimer(
                HolsterTimerHandle,
                this,
                &UNLCharacterComponent::OnWeaponHolstered,
                ActualMontageLength
            );
        }
        else
        {
            OnWeaponHolstered();
        }
    }
    // Play Weapon Anim Montage if exist.
    if (UAnimMontage* WeaponHolsterAnimMontage = WeaponAnimInfo->Holster.LoadSynchronous())
    {
        const float MontagePlayLength = WeaponHolsterAnimMontage->GetPlayLength();
        const float MontageTimeOverride = WeaponAnimInfo->HolsterTime;

        const float MontagePlayRate = MontageTimeOverride > 0.f ? MontagePlayLength / MontageTimeOverride : 1.f;
        GetOwningPlayer()->PlayWeaponAnimMontage(WeaponHolsterAnimMontage, MontagePlayRate);
    }

    if (GetOwnerRole() == ROLE_Authority)
    {
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
