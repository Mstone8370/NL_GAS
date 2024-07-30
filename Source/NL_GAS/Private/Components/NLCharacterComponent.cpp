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
#include "Data/WeaponInfo.h"
#include "Kismet/KismetMathLibrary.h"

UNLCharacterComponent::UNLCharacterComponent()
    : MaxWeaponSlotSize(3)
    , CurrentWeaponSlot(255)
    , bStartupWeaponInitFinished(false)
{
    PrimaryComponentTick.bCanEverTick = false;

    SetIsReplicatedByDefault(true);

    WeaponSlotSocketMap.Add(FName("weapon_slot_1"), nullptr);
    WeaponSlotSocketMap.Add(FName("weapon_slot_2"), nullptr);
    WeaponSlotSocketMap.Add(FName("weapon_slot_3"), nullptr);

    WeaponActorSlot.Reset(MaxWeaponSlotSize);
    WeaponActorSlot.Init(nullptr, MaxWeaponSlotSize);
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

    WeaponActorSlot.Reset(MaxWeaponSlotSize);
    WeaponActorSlot.Init(nullptr, MaxWeaponSlotSize);

    FScopedAbilityListLock ActiveScopeLock(*ASC);

    const int32 StartupWeaponNum = FMath::Min(int32(MaxWeaponSlotSize), PS->StartupWeapons.Num());
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

        Weapon->InitializeWeapon(WeaponTag);
        
        AttachWeaponToSocket(Weapon);

        Weapon->FinishSpawning(SpawnTransform);
        WeaponActorSlot[i] = Weapon;

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
    UpdateMeshes(OldWeapon, true);
}

void UNLCharacterComponent::OnRep_WeaponActorSlot(TArray<AWeaponActor*> OldWeaponActorSlot)
{
    if (!bStartupWeaponInitFinished)
    {
        ValidateStartupWeapons();
    }
    else
    {
        if (GetOwner()->GetLocalRole() != ROLE_SimulatedProxy)
        {
            UpdateWeaponTagSlot();
            WeaponSlotChanged.Broadcast(WeaponTagSlot);
        }

        UpdateMeshes(nullptr, GetOwner()->GetLocalRole() == ROLE_SimulatedProxy);
    }
}

void UNLCharacterComponent::UpdateMeshes(AWeaponActor* OldWeaponActor, bool bIsSimulated)
{
    if (!bIsSimulated)
    {
        const FGameplayTag WeaponTag = GetCurrentWeaponTag();
        if (WeaponTag.IsValid())
        {
            if (const FWeaponInfo* Info = UNLFunctionLibrary::GetWeaponInfoByTag(this, WeaponTag))
            {
                GetOwningPlayer()->UpdateViewWeaponAndAnimLayer(
                    Info->ViewModelMesh.LoadSynchronous(),
                    Info->WeaponAnimBP,
                    Info->ArmsAnimLayerClass
                );
            }
        }
    }
    UpdateOwningCharacterMesh(OldWeaponActor);
}

void UNLCharacterComponent::UpdateOwningCharacterMesh(AWeaponActor* OldWeaponActor)
{
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

    if (AWeaponActor* PrevWeapon = GetCurrentWeaponActor())  // Could be nullptr
    {
        GetNLASC()->WeaponHolstered(PrevWeapon);
        PrevWeapon->Holstered();
        AttachWeaponToSocket(PrevWeapon);
    }
    AWeaponActor* ChangedWeapon = GetWeaponActorAtSlot(WeaponSwapPendingSlot);

    CurrentWeaponSlot = WeaponSwapPendingSlot;

    UpdateMeshes();
    AttachWeaponToHand(ChangedWeapon);

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

void UNLCharacterComponent::OnWeaponBulletNumChanged(const AWeaponActor* Weapon, int32 NewBulletNum)
{
    if (Weapon == GetCurrentWeaponActor())
    {
        CurrentWeaponBulletNumChanged.Broadcast(NewBulletNum);
    }
}

void UNLCharacterComponent::BindWeaponDelegate(AWeaponActor* Weapon)
{
    Weapon->BulletNumChanged.AddDynamic(this, &UNLCharacterComponent::OnWeaponBulletNumChanged);
}

void UNLCharacterComponent::UnBindWeaponDelegate(AWeaponActor* Weapon)
{
    Weapon->BulletNumChanged.RemoveAll(this);
}

void UNLCharacterComponent::AttachWeaponToSocket(AWeaponActor* Weapon)
{
    FName TargetSocketName = NAME_None;
    for (TPair<FName, AWeaponActor*>& Item : WeaponSlotSocketMap)
    {
        if (Item.Value == nullptr)
        {
            TargetSocketName = Item.Key;
            break;
        }
    }
    if (TargetSocketName.IsNone())
    {
        return;
    }

    WeaponSlotSocketMap[TargetSocketName] = Weapon;

    FAttachmentTransformRules AttachRule = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
    AttachRule.RotationRule = EAttachmentRule::KeepRelative;
    Weapon->AttachToComponent(GetOwningPlayer()->GetMesh(), AttachRule, TargetSocketName);
}

void UNLCharacterComponent::AttachWeaponToHand(AWeaponActor* Weapon)
{
    FName TargetSocketName = NAME_None;
    for (TPair<FName, AWeaponActor*>& Item : WeaponSlotSocketMap)
    {
        if (Item.Value == Weapon)
        {
            TargetSocketName = Item.Key;
            break;
        }
    }
    if (TargetSocketName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("Error: No available socket for attachment."));
        return;
    }

    WeaponSlotSocketMap[TargetSocketName] = nullptr;

    FAttachmentTransformRules AttachRule = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
    AttachRule.RotationRule = EAttachmentRule::KeepRelative;
    Weapon->AttachToComponent(GetOwningPlayer()->GetMesh(), AttachRule, FName("weapon_r"));
}

void UNLCharacterComponent::ClearWeapons()
{
    bStartupWeaponInitFinished = false;
    /**
    * �������Ҷ� 0�� ���� ���⸦ ��� �Ϸ��� CurrentWeaponSlot�� 0�̸� �ȵǹǷ� �ٸ� ���� �־����
    * �ٵ� �� ���� Ÿ���� uint�� ������ �ȵǴ� ������ �ε����� �Ѵ� ������ ������.
    */
    CurrentWeaponSlot = MaxWeaponSlotSize;

    for (TPair<FName, AWeaponActor*>& Item : WeaponSlotSocketMap)
    {
        Item.Value = nullptr;
    }

    if (GetOwnerRole() == ENetRole::ROLE_Authority)
    {
        for (uint8 i = 0; i < WeaponActorSlot.Num(); i++)
        {
            if (IsValid(WeaponActorSlot[i]))
            {
                WeaponActorSlot[i]->Destroy();
            }
            WeaponActorSlot[i] = nullptr;
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

UNLAbilitySystemComponent* UNLCharacterComponent::GetNLASC() const
{
    if (GetASC())
    {
        return Cast<UNLAbilitySystemComponent>(GetASC());
    }
    return nullptr;
}

void UNLCharacterComponent::UpdateWeaponTagSlot()
{
    WeaponTagSlot.Empty();
    for (const AWeaponActor* Weapon : WeaponActorSlot)
    {
        if (IsValid(Weapon))
        {
            WeaponTagSlot.Add(Weapon->GetWeaponTag());
        }
        else
        {
            WeaponTagSlot.Add(FGameplayTag());
        }
    }
}

int32 UNLCharacterComponent::GetWeaponSlotSize() const
{
    return WeaponActorSlot.Num();
}

int32 UNLCharacterComponent::GetWeaponSlotMaxSize() const
{
    return MaxWeaponSlotSize;
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

AWeaponActor* UNLCharacterComponent::GetEquippedWeaponActorByTag(const FGameplayTag& WeaponTag) const
{
    for (AWeaponActor* Weapon : WeaponActorSlot)
    {
        if (IsValid(Weapon) && Weapon->GetWeaponTag().MatchesTagExact(WeaponTag))
        {
            return Weapon;
        }
    }
    return nullptr;
}

const FGameplayTag UNLCharacterComponent::GetWeaponTagAtSlot(uint8 Slot) const
{
    if (IsValid(GetWeaponActorAtSlot(Slot)))
    {
        return GetWeaponActorAtSlot(Slot)->GetWeaponTag();
    }
    return Weapon_Unarmed;
}

const FGameplayTag UNLCharacterComponent::GetCurrentWeaponTag() const
{
    return GetWeaponTagAtSlot(CurrentWeaponSlot);
}

void UNLCharacterComponent::WeaponAdded(AWeaponActor* Weapon)
{
    BindWeaponDelegate(Weapon);

    if (!bStartupWeaponInitFinished)
    {
        InitializedStartupWeapons.AddUnique(Weapon);
        ValidateStartupWeapons();
    }
    else
    {
        // TODO: New weapon equipped while playing
        // �̶����� validation�� �ؾ��ϴ��� �����غ�����.
        // startup ����� WeaponActorSlot�� ���ø�����Ʈ �Ǿ Ŭ���̾�Ʈ���� ���Ͱ� �������� ������ nullptr��.
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

    const int32 StartupWeaponNum = FMath::Min(int32(MaxWeaponSlotSize), PS->StartupWeapons.Num());
    if (InitializedStartupWeapons.Num() == StartupWeaponNum)
    {
        bool bAllInitalizedAndValid = true;
        for (int32 i = 0; i < StartupWeaponNum; i++)
        {
            AWeaponActor* InitializedWeapon = InitializedStartupWeapons[i];
            if (!InitializedWeapon->IsInitialized() || !WeaponActorSlot.Contains(InitializedWeapon))
            {
                bAllInitalizedAndValid = false;
                break;
            }

            const FName AttachedSocketName = InitializedWeapon->GetAttachParentSocketName();
            if (WeaponSlotSocketMap.Contains(AttachedSocketName))
            {
                WeaponSlotSocketMap[AttachedSocketName] = InitializedWeapon;
            }
        }

        bStartupWeaponInitFinished = bAllInitalizedAndValid;
        if (bStartupWeaponInitFinished)
        {
            if (GetOwnerRole() == ROLE_SimulatedProxy)
            {
                // Update Simulated Character Mesh
                // ���߿� ������ Ŭ���̾�Ʈ ���忡���� ������ �����ߴ� �÷��̾���� ���� ������ �°� ������Ʈ
                CurrentWeaponSlot = 0;
                UpdateOwningCharacterMesh();
            }
            else if (GetOwningPlayer()->GetNLPC()) // Ŭ���̾�Ʈ�� Ŭ���̾�Ʈ���� �����Ƽ Ȱ��ȭ �ϰ� ��.
            {
                UAbilitySystemComponent* ASC = GetASC();
                FGameplayTagContainer TagContainer(Ability_WeaponChange_1);
                ASC->TryActivateAbilitiesByTag(TagContainer);

                UpdateWeaponTagSlot();
                WeaponSlotChanged.Broadcast(WeaponTagSlot);
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

    if (!CanSwapWeaponSlot(NewWeaponSlot))
    {
        return;
    }

    WeaponSwapPendingSlot = NewWeaponSlot;

    // Update Widget
    WeaponSwapped.Broadcast(
        GetCurrentWeaponTag(),
        CurrentWeaponSlot,
        GetWeaponTagAtSlot(WeaponSwapPendingSlot),
        WeaponSwapPendingSlot
    );
    CurrentWeaponBulletNumChanged.Broadcast(
        GetWeaponActorAtSlot(WeaponSwapPendingSlot)->GetCurrentBulletNum()
    );

    if (!IsValid(GetCurrentWeaponActor()))
    {
        // Start up
        OnWeaponHolstered();
        return;
    }

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
    GetOwningPlayer()->StopArmsAnimMontage();
    GetOwningPlayer()->StopWeaponAnimMontage();
    
    // ���� ���⸦ draw���� ������� Ȯ��
    if (GetWorld()->GetTimerManager().IsTimerActive(DrawTimerHandle))
    {
        /**
        * ���� ������ ���� ���Ⱑ drawn �Ǳ� ���� ���۵� �� ����.
        * �׷� ��쿡�� draw Ÿ�̸Ӱ� �����Ǿ������Ƿ� holster���� ���Ⱑ drawn �Ǿ ��� �����ϰ� ��.
        * ���� draw Ÿ�̸Ӹ� clear��.
        */
        GetWorld()->GetTimerManager().ClearTimer(DrawTimerHandle);
    }

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

bool UNLCharacterComponent::CommitWeaponCost(bool& bIsLast)
{
    bool Ret = false;
    bIsLast = false;

    if (CanAttack())
    {
        Ret = GetCurrentWeaponActor()->CommitWeaponCost();
        if (Ret)
        {
            int32 BulletRemained = GetCurrentWeaponActor()->GetCurrentBulletNum();
            bIsLast = BulletRemained < 1;
        }
    }
    return Ret;
}

float UNLCharacterComponent::PlayCurrentWeaponMontage(const FGameplayTag& MontageTag, FName StartSectionName)
{
    const FGameplayTag& CurrentWeaponTag = GetCurrentWeaponTag();
    const FTaggedAnimMontageInfo* MontageInfo = UNLFunctionLibrary::GetAnimMontageByTag(this, CurrentWeaponTag, MontageTag);
    if (!MontageInfo)
    {
        return 0.f;
    }

    const float MontageTimeOverride = MontageInfo->PlayLengthOverride;

    UAnimMontage* ArmsAnimMontage = MontageInfo->ArmsAnimMontage.LoadSynchronous();
    UAnimMontage* WeaponAnimMontage = MontageInfo->WeaponAnimMontage.LoadSynchronous();
    if (!ArmsAnimMontage && !WeaponAnimMontage)
    {
        return 0.f;
    }

    const float MontagePlayLength = ArmsAnimMontage ? ArmsAnimMontage->GetPlayLength() : WeaponAnimMontage->GetPlayLength();  // Arms Montage First
    const float OverriddenPlayLength = MontageTimeOverride > 0.f ? MontageTimeOverride : MontagePlayLength;
    const float MontagePlayRate = UKismetMathLibrary::SafeDivide(MontagePlayLength, OverriddenPlayLength);

    // Only on Client
    GetOwningPlayer()->PlayWeaponAnimMontage(WeaponAnimMontage, MontagePlayRate, StartSectionName);
    GetOwningPlayer()->PlayArmsAnimMontage(ArmsAnimMontage, MontagePlayRate, StartSectionName);

    return OverriddenPlayLength;
}

float UNLCharacterComponent::PlayCurrentWeaponMontageAndSetCallback(const FGameplayTag& MontageTag, FTimerHandle& OutTimerHandle, FTimerDelegate TimerDelegate, bool bOnBlendOut, FName StartSectionName)
{
    OutTimerHandle = FTimerHandle();

    const float MontagePlayLength = PlayCurrentWeaponMontage(MontageTag, StartSectionName);
    const float BlendOutTime = GetOwningPlayer()->GetCurrentArmsMontage()->GetDefaultBlendOutTime();
    const float TimerTime = bOnBlendOut ? MontagePlayLength - BlendOutTime : MontagePlayLength;

    // On Server and Client
    if (!FMath::IsNearlyZero(TimerTime) && TimerTime > 0.f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            OutTimerHandle,
            TimerDelegate,
            TimerTime,
            false
        );
    }
    else
    {
        TimerDelegate.ExecuteIfBound();
    }

    return TimerTime;
}

bool UNLCharacterComponent::IsCurrentWeaponMagEmpty() const
{
    if (GetCurrentWeaponActor())
    {
        return GetCurrentWeaponActor()->IsMagEmpty();
    }
    return false;
}

bool UNLCharacterComponent::CanReload() const
{
    return GetCurrentWeaponActor()->CanReload();
}

bool UNLCharacterComponent::StartReload()
{
    if (!CanReload())
    {
        return false;
    }

    if (GetCurrentWeaponActor()->IsReloading())
    {
        const FGameplayTag ReloadMontageTag = GetCurrentWeaponActor()->IsTacticalReload() ? Montage_Weapon_ReloadShort : Montage_Weapon_ReloadLong;
        const EReloadState ReloadState = GetCurrentWeaponActor()->GetReloadState();
        
        FName MontageSectionName = NAME_None;
        if (ReloadState == EReloadState::MagOut)
        {
            MontageSectionName = "MagOut";
        }
        else if (ReloadState == EReloadState::MagIn)
        {
            MontageSectionName = "MagIn";
        }
        PlayCurrentWeaponMontage(ReloadMontageTag, MontageSectionName);
    }
    else
    {
        const FGameplayTag ReloadMontageTag = GetCurrentWeaponActor()->IsMagEmpty() ? Montage_Weapon_ReloadLong : Montage_Weapon_ReloadShort;
        PlayCurrentWeaponMontage(ReloadMontageTag);
    }
    
    return true;
}

void UNLCharacterComponent::OnWeaponReloadStateChanged(const FGameplayTag& WeaponTag, const FGameplayTag& StateTag)
{
    GetCurrentWeaponActor()->ReloadStateChanged(StateTag);
}

float UNLCharacterComponent::GetCurrentWeaponSpreadValue(bool bADS, bool bFalling, bool bCrouched, float CharacterSpeedSquared, int32 RecoilOffset) const
{
    if (bADS)
    {
        return 0.f;
    }

    if (AWeaponActor* Weapon = GetCurrentWeaponActor())
    {
        if (const FWeaponSpreadInfo* SpreadInfo = Weapon->GetSpreadInfo())
        {
            if (bFalling)
            {
                return SpreadInfo->Fall;
            }

            float Val = 0.f;
            if (bCrouched)
            {
                Val = SpreadInfo->Crouch;
            }
            else
            {
                Val = SpreadInfo->Hip;
                if (CharacterSpeedSquared >= 100.f)
                {
                    Val += SpreadInfo->Additive_Walk;
                }
            }
            Val += FMath::Min(RecoilOffset, SpreadInfo->RecoilOffsetMax) * SpreadInfo->Additive_Recoil;

            return Val;
        }
    }
    return 0.f;
}

const FGameplayTag UNLCharacterComponent::GetCurrentWeaponADSFOVTag() const
{
    if (const AWeaponActor* Weapon = GetCurrentWeaponActor())
    {
        return Weapon->GetADSFOVTag();
    }
    return FGameplayTag();
}

void UNLCharacterComponent::LowerWeapon()
{
    if (GetASC())
    {
        if (GetOwnerRole() == ROLE_Authority)
        {
            // Cancel abilities
            FGameplayTagContainer CancelTags;
            CancelTags.AddTag(Ability_Weapon_Primary);
            CancelTags.AddTag(Ability_Weapon_Secondary);
            CancelTags.AddTag(Ability_Weapon_Reload);
            GetNLASC()->CancelAbilities(&CancelTags);
        }

        GetASC()->AddLooseGameplayTag(Ability_Block_Weapon);
    }

    GetOwningPlayer()->StopArmsAnimMontage();
    GetOwningPlayer()->StopWeaponAnimMontage();
}

void UNLCharacterComponent::RaiseWeapon()
{
    if (GetASC())
    {
        GetASC()->RemoveLooseGameplayTag(Ability_Block_Weapon);
    }
    CheckCurrentWeaponReloadState();
}

void UNLCharacterComponent::CheckCurrentWeaponReloadState()
{
    if (GetOwnerRole() == ENetRole::ROLE_Authority && GetCurrentWeaponActor())
    {
        GetCurrentWeaponActor()->CheckReloadState();
    }
}

void UNLCharacterComponent::HandleOwnerDeath()
{
    DropCurrentWeapon();
    
    ClearWeapons();
}

void UNLCharacterComponent::DropCurrentWeapon()
{
    AWeaponActor* WeaponActor = GetCurrentWeaponActor();
    if (!IsValid(WeaponActor))
    {
        return;
    }

    if (GetOwnerRole() == ENetRole::ROLE_Authority)
    {
        GetNLASC()->WeaponDropped(WeaponActor);

        WeaponActor->SetOwner(nullptr);
        WeaponActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        WeaponActor->SetWeaponState(false);
        WeaponActorSlot[CurrentWeaponSlot] = nullptr;

        UpdateMeshes();
    }
    else
    {
        // WeaponActorSlot[CurrentWeaponSlot] = nullptr;
    }
}
