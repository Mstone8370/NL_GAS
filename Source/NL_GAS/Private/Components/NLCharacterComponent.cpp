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
    // 레플리케이트 되었다면 Simulated 액터라는 뜻이므로 3인칭 애니메이션과 3인칭 메시 설정.
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

            if (!IsValid(GetCurrentWeaponActor()))
            {
                // Current Weapon Dropped
                OnCurrentWeaponDropped(true);
            }
            else if (GetCurrentWeaponActor() != OldWeaponActorSlot[CurrentWeaponSlot])
            {
                // Weapon exchanged;
                OnCurrentWeaponDropped();

                // TODO: draw
            }
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

    CurrentWeaponSlot = WeaponSwapPendingSlot;

    UpdateMeshes();

    if (AWeaponActor* ChangedWeapon = GetWeaponActorAtSlot(WeaponSwapPendingSlot))
    {
        AttachWeaponToHand(ChangedWeapon);

        // Draw New Weapon
        const bool bDrawFirst = !ChangedWeapon->IsEverDrawn();
        const FGameplayTag& CurrentWeaponTag = GetCurrentWeaponTag();
        const FGameplayTag MontageTag = bDrawFirst ? Montage_Weapon_DrawFirst : Montage_Weapon_Draw;
        FTimerDelegate TimerDelegate;
        TimerDelegate.BindUObject(this, &UNLCharacterComponent::OnWeaponDrawn);
        PlayCurrentWeaponMontageAndSetCallback(MontageTag, DrawTimerHandle, TimerDelegate);
    }
}

void UNLCharacterComponent::OnWeaponDrawn()
{
    // This function could be called by Anim Notify before timer.
    GetWorld()->GetTimerManager().ClearTimer(DrawTimerHandle);

    GetASC()->SetLooseGameplayTagCount(Ability_Block_Weapon, 0);

    if (GetCurrentWeaponActor())
    {
        GetNLASC()->WeaponDrawn(GetCurrentWeaponActor());

        GetCurrentWeaponActor()->Drawn();
    }
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
        if (TargetSocketName.IsNone() && Item.Value == nullptr)
        {
            TargetSocketName = Item.Key;
        }
        if (Item.Value == Weapon)
        {
            // 이미 소켓에 어태치 되어있는 상태
            return;
        }
    }
    if (TargetSocketName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("Error: No available socket for attachment."));
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
    if (!TargetSocketName.IsNone())
    {
        WeaponSlotSocketMap[TargetSocketName] = nullptr;
    }

    FAttachmentTransformRules AttachRule = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
    AttachRule.RotationRule = EAttachmentRule::KeepRelative;
    Weapon->AttachToComponent(GetOwningPlayer()->GetMesh(), AttachRule, FName("weapon_r"));
}

void UNLCharacterComponent::ClearWeapons()
{
    bStartupWeaponInitFinished = false;
    /**
    * 리스폰할때 0번 슬롯 무기를 들게 하려면 CurrentWeaponSlot이 0이면 안되므로 다른 값을 넣어야함
    * 근데 이 변수 타입이 uint라서 음수는 안되니 마지막 인덱스를 넘는 값으로 지정함.
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

void UNLCharacterComponent::OnCurrentWeaponDropped(bool bSwapSlot)
{
    // On Server and Client

    if (GetOwningPlayer())
    {
        GetOwningPlayer()->StopWeaponAnimMontage();
        GetOwningPlayer()->StopArmsAnimMontage();
    }

    if (GetWorld()->GetTimerManager().IsTimerActive(HolsterTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(HolsterTimerHandle);
        OnWeaponHolstered();
    }
    if (GetWorld()->GetTimerManager().IsTimerActive(DrawTimerHandle))
    {
        GetWorld()->GetTimerManager().ClearTimer(DrawTimerHandle);
        OnWeaponDrawn();
    }

    if (bSwapSlot)
    {
        /**
        * 이 함수 호출은 어빌리티를 activate하는게 아니라서 서버와 클라이언트간 동기화가 안됨.
        * 따라서 서버와 클라이언트 모두 호출될 수 있게 해야함.
        */
        TrySwapWeaponSlot_Next();
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
            WeaponTagSlot.Add(FGameplayTag::EmptyTag);
        }
    }
    WeaponSlotChanged.Broadcast(WeaponTagSlot);
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
        // Do nothing
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
                // 나중에 접속한 클라이언트 입장에서도 기존에 접속했던 플레이어들의 무기 정보에 맞게 업데이트
                CurrentWeaponSlot = 0;
                UpdateOwningCharacterMesh();
            }
            else if (GetOwningPlayer()->GetNLPC()) // 클라이언트면 클라이언트에서 어빌리티 활성화 하게 함.
            {
                UAbilitySystemComponent* ASC = GetASC();
                FGameplayTagContainer TagContainer(Ability_WeaponChange_1);
                ASC->TryActivateAbilitiesByTag(TagContainer);

                UpdateWeaponTagSlot();
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

void UNLCharacterComponent::TrySwapWeaponSlot(int32 NewWeaponSlot, bool bCheckCondition, bool bSkipHolsterAnim)
{
    // On Server and Client by GameplayAbility

    if (bCheckCondition && !CanSwapWeaponSlot(NewWeaponSlot))
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

    if (!IsValid(GetCurrentWeaponActor()) || bSkipHolsterAnim)
    {
        // Start up or weapon drop
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
    
    // 현재 무기를 draw중인 경우인지 확인
    if (GetWorld()->GetTimerManager().IsTimerActive(DrawTimerHandle))
    {
        /**
        * 무기 스왑은 현재 무기가 drawn 되기 전에 시작될 수 있음.
        * 그런 경우에는 draw 타이머가 설정되어있으므로 holster중인 무기가 drawn 되어서 사용 가능하게 됨.
        * 따라서 draw 타이머를 clear함.
        */
        GetWorld()->GetTimerManager().ClearTimer(DrawTimerHandle);
    }

    const FGameplayTag& CurrentWeaponTag = GetCurrentWeaponTag();
    FTimerDelegate TimerDelegate;
    TimerDelegate.BindUObject(this, &UNLCharacterComponent::OnWeaponHolstered);
    PlayCurrentWeaponMontageAndSetCallback(Montage_Weapon_Holster, HolsterTimerHandle, TimerDelegate);
}

void UNLCharacterComponent::TrySwapWeaponSlot_Next(bool bPrev)
{
    for (int32 i = 1; i < MaxWeaponSlotSize; i++)
    {
        const int32 Offset = bPrev ? -i : i;
        const int32 NewSlotNum = ((int32)(CurrentWeaponSlot + MaxWeaponSlotSize) + Offset) % MaxWeaponSlotSize;
        if (CanSwapWeaponSlot(NewSlotNum))
        {
            TrySwapWeaponSlot(NewSlotNum);
            return;
        }
    }
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

void UNLCharacterComponent::DropCurrentWeapon(bool bSwapSlot)
{
    AWeaponActor* WeaponActor = GetCurrentWeaponActor();
    if (!IsValid(WeaponActor))
    {
        return;
    }

    if (GetOwnerRole() == ENetRole::ROLE_Authority)
    {
        GetNLASC()->WeaponDropped(WeaponActor);
        WeaponActor->Dropped();

        WeaponActorSlot[CurrentWeaponSlot] = nullptr;

        UpdateMeshes();

        UpdateWeaponTagSlot(); // for authorized player

        OnCurrentWeaponDropped(bSwapSlot);
    }
}

void UNLCharacterComponent::PickUp(AActor* Pickupable)
{
    if (!IsValid(Pickupable))
    {
        return;
    }

    // On Server

    if (AWeaponActor* WeaponActor = Cast<AWeaponActor>(Pickupable))
    {
        PickUpWeapon(WeaponActor);
    }
}

void UNLCharacterComponent::PickUpWeapon(AWeaponActor* WeaponActor)
{
    int32 Slot = -1;
    if (IsWeaponSlotFull())
    {
        Slot = CurrentWeaponSlot;

        DropCurrentWeapon();
    }
    else
    {
        for (int i = 0; i < MaxWeaponSlotSize; i++)
        {
            if (!IsValid(GetWeaponActorAtSlot(i)))
            {
                Slot = i;
                break;
            }
        }
        if (Slot < 0)
        {
            UE_LOG(LogTemp, Error, TEXT("Error: IsWeaponSlotFull() returns false but all slots are full"));
            return;
        }
    }

    AttachWeaponToSocket(WeaponActor);
    WeaponActor->SetOwner(GetOwningPlayer());
    IPickupable::Execute_OnPickedUp(WeaponActor);

    WeaponActorSlot[Slot] = WeaponActor;
    UpdateWeaponTagSlot();

    GetNLASC()->WeaponAdded(WeaponActor);

    TrySwapWeaponSlot(Slot, false, true);
}

bool UNLCharacterComponent::IsWeaponSlotFull() const
{
    for (const AWeaponActor* Weapon : WeaponActorSlot)
    {
        if (!IsValid(Weapon))
        {
            return false;
        }
    }
    return true;
}

bool UNLCharacterComponent::IsWeaponSlotEmpty() const
{
    for (const AWeaponActor* Weapon : WeaponActorSlot)
    {
        if (IsValid(Weapon))
        {
            return false;
        }
    }
    return true;
}
