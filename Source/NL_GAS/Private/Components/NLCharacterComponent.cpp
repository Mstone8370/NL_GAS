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
    : bStartupWeaponInitFinished(false)
{
    PrimaryComponentTick.bCanEverTick = false;

    SetIsReplicatedByDefault(true);

    WeaponSlot.MaxSlotSize = 3;
    WeaponSlot.CurrentSlot = WeaponSlot.MaxSlotSize;

    WeaponSlotSocketMap.Add(FName("weapon_slot_1"), nullptr);
    WeaponSlotSocketMap.Add(FName("weapon_slot_2"), nullptr);
    WeaponSlotSocketMap.Add(FName("weapon_slot_3"), nullptr);

    WeaponSlot.WeaponActorSlot.Reset(WeaponSlot.MaxSlotSize);
    WeaponSlot.WeaponActorSlot.Init(nullptr, WeaponSlot.MaxSlotSize);
}

void UNLCharacterComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UNLCharacterComponent, WeaponSlot, COND_None, REPNOTIFY_OnChanged);
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

    WeaponSlot.WeaponActorSlot.Reset(WeaponSlot.MaxSlotSize);
    WeaponSlot.WeaponActorSlot.Init(nullptr, WeaponSlot.MaxSlotSize);

    FScopedAbilityListLock ActiveScopeLock(*ASC);

    const int32 StartupWeaponNum = FMath::Min(int32(WeaponSlot.MaxSlotSize), PS->StartupWeapons.Num());
    for (int32 i = 0; i < StartupWeaponNum; i++)
    {
        const FGameplayTag& WeaponTag = PS->StartupWeapons[i];

        // Spawn Weapon
        FTransform SpawnTransform;
        SpawnTransform.SetLocation(OwningPlayer->GetActorLocation());
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
        WeaponSlot.WeaponActorSlot[i] = Weapon;

        // Add Weapon Abilities
        GetNLASC()->WeaponAdded(Weapon);
    }
}

void UNLCharacterComponent::OnRep_WeaponSlot(const FWeaponSlot& OldSlot)
{
    if (!bStartupWeaponInitFinished)
    {
        ValidateStartupWeapons();
        return;
    }

    TSet<AWeaponActor*> OldSet(OldSlot.WeaponActorSlot);
    TSet<AWeaponActor*> NewSet(WeaponSlot.WeaponActorSlot);

    TSet<AWeaponActor*> RemovedWeapons;
    for (AWeaponActor* Wpn : OldSet)
    {
        if (Wpn && !NewSet.Contains(Wpn))
        {
            RemovedWeapons.Add(Wpn);
            Wpn->Dropped();
        }
    }
    // 새로운 무기를 추가하기 전에 제거된 무기를 소켓 맵에서도 제거해서 빈 공간을 만들어야 함.
    for (TPair<FName, AWeaponActor*>& Item : WeaponSlotSocketMap)
    {
        if (Item.Value && RemovedWeapons.Contains(Item.Value))
        {
            Item.Value = nullptr;
        }
    }

    // Added weapons
    for (AWeaponActor* Wpn : NewSet)
    {
        if (Wpn && !OldSet.Contains(Wpn))
        {
            AttachWeaponToSocket(Wpn);
            Wpn->SetOwner(GetOwner()); // WeaponActor의 Owner는 아직 레플리케이트되지 않았을수도 있음.
            Wpn->PickedUp(GetOwningCharacter());
        }
    }

    UpdateWeaponTagSlot();
    
    bool bShouldSwapSlot = false;
    if (WeaponSlot.CurrentSlot < WeaponSlot.MaxSlotSize)
    {
        const bool bCurrentSlotChanged = OldSlot.CurrentSlot != WeaponSlot.CurrentSlot;

        // CurrentSlot은 동일하다는 전제조건
        const bool bCurrentWeaponChanged = OldSlot.WeaponActorSlot[WeaponSlot.CurrentSlot] != WeaponSlot.WeaponActorSlot[WeaponSlot.CurrentSlot];
        
        if (bCurrentSlotChanged || bCurrentWeaponChanged)
        {
            bShouldSwapSlot = true;
        }
    }
    
    const bool bIsSimulated = GetOwnerRole() == ROLE_SimulatedProxy;
    UpdateMeshes(nullptr, bIsSimulated);

    if (bShouldSwapSlot)
    {
        for (AWeaponActor* Wpn : WeaponSlot.WeaponActorSlot)
        {
            AttachWeaponToSocket(Wpn);
        }

        const uint8 RealCurrentSlot = WeaponSlot.CurrentSlot;
        if (!bIsSimulated)
        {
            /**
            * OnCurrentWeaponDropped 함수 호출로 인해서 current slot이 변경될 수 있음.
            * 따라서 레플리케이트 된 current slot을 따로 저장해놓음.
            * e.g. holster중인데 무기 교체가 된 경우, 교체된 무기가 있는 슬롯으로 스왑하는게 맞음.
            *      하지만 holster중인 무기를 버리는걸 우선으로 처리해야하는 정해진 절차가 있으므로
            *      swap pending 슬롯으로 스왑하는 과정을 먼저 하게 됨.
            *      (무기를 버리는 경우에는 pending 슬롯으로 스왑하는게 맞는 행동이기 때문.)
            *      pending 슬롯으로 스왑하게 되면 current slot이 변경되는 문제가 발생하게 되고,
            *      그렇게 로컬에서 변경된 current slot 값으로 무기 스왑을 하게되면 서버와 클라이언트가
            *      서로 다른 슬롯의 무기를 들고있게 됨.
            *      서버에서는 모든 작업을 끝낸 결과를 레플리케이트 시켰는데 클라이언트에서는 그 값을 바꾼 상황.
            *      이 함수에서는 클라이언트의 입장에서, 서버에서 발생했던 사건의 결과물만 보고 판단해야하므로,
            *      서버에서 모든것이 의도대로 작동했다고 생각해야하고 그 결과를 그대로 따라야함.
            */
            OnCurrentWeaponDropped(); // This function is not const
            TrySwapWeaponSlot(RealCurrentSlot, false, true);
        }
        else if (MontageTemp && GetOwningCharacter() && GetOwningCharacter()->GetMesh() && GetOwningCharacter()->GetMesh()->GetAnimInstance())
        {
            AttachWeaponToHand(GetWeaponActorAtSlot(RealCurrentSlot));
            // TODO: temp
            GetOwningCharacter()->GetMesh()->GetAnimInstance()->Montage_Play(MontageTemp);
        }
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

void UNLCharacterComponent::RevertWeaponSwap()
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

    WeaponSlot.CurrentSlot = WeaponSwapPendingSlot;

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
    if (!Weapon->BulletNumChanged.IsAlreadyBound(this, &UNLCharacterComponent::OnWeaponBulletNumChanged))
    {
        Weapon->BulletNumChanged.AddDynamic(this, &UNLCharacterComponent::OnWeaponBulletNumChanged);
    }
}

void UNLCharacterComponent::UnBindWeaponDelegate(AWeaponActor* Weapon)
{
    Weapon->BulletNumChanged.RemoveAll(this);
}

void UNLCharacterComponent::AttachWeaponToSocket(AWeaponActor* Weapon)
{
    if (!IsValid(Weapon))
    {
        return;
    }

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
    if (!IsValid(Weapon))
    {
        return;
    }

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
    WeaponSlot.CurrentSlot = WeaponSlot.MaxSlotSize;

    for (TPair<FName, AWeaponActor*>& Item : WeaponSlotSocketMap)
    {
        Item.Value = nullptr;
    }

    if (GetOwnerRole() == ENetRole::ROLE_Authority)
    {
        for (uint8 i = 0; i < WeaponSlot.WeaponActorSlot.Num(); i++)
        {
            if (IsValid(WeaponSlot.WeaponActorSlot[i]))
            {
                if (GetNLASC())
                {
                    GetNLASC()->WeaponDropped(WeaponSlot.WeaponActorSlot[i]);
                }
                WeaponSlot.WeaponActorSlot[i]->Destroy();
            }
            WeaponSlot.WeaponActorSlot[i] = nullptr;
        }
    }
}

void UNLCharacterComponent::OnCurrentWeaponDropped()
{
    // On Server and Client

    if (GetOwningPlayer())
    {
        GetOwningPlayer()->StopWeaponAnimMontage();
        GetOwningPlayer()->StopArmsAnimMontage();
    }

    const bool bHolstering = GetWorld()->GetTimerManager().IsTimerActive(HolsterTimerHandle);
    const bool bDrawing = GetWorld()->GetTimerManager().IsTimerActive(DrawTimerHandle);
    if (bHolstering)
    {
        // 무기를 holster하는 중이었으면, 이미 무기를 스왑중이고, pending 슬롯이 정해져있음
        GetWorld()->GetTimerManager().ClearTimer(HolsterTimerHandle);
        OnWeaponHolstered();
    }
    else
    {
        if (bDrawing)
        {
            // 무기를 draw하는 중이었으면, 다음 슬롯 무기로 스왑해야함.
            GetWorld()->GetTimerManager().ClearTimer(DrawTimerHandle);
        }

        if (GetOwnerRole() == ROLE_Authority)
        {
            TrySwapWeaponSlot_Next();
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
    for (const AWeaponActor* Weapon : WeaponSlot.WeaponActorSlot)
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
    return WeaponSlot.WeaponActorSlot.Num();
}

int32 UNLCharacterComponent::GetWeaponSlotMaxSize() const
{
    return WeaponSlot.MaxSlotSize;
}

AWeaponActor* UNLCharacterComponent::GetWeaponActorAtSlot(uint8 Slot) const
{
    if (Slot < WeaponSlot.WeaponActorSlot.Num())
    {
        return WeaponSlot.WeaponActorSlot[Slot];
    }
    return nullptr;
}

AWeaponActor* UNLCharacterComponent::GetCurrentWeaponActor() const
{
    return GetWeaponActorAtSlot(WeaponSlot.CurrentSlot);
}

AWeaponActor* UNLCharacterComponent::GetEquippedWeaponActorByTag(const FGameplayTag& WeaponTag) const
{
    for (AWeaponActor* Weapon : WeaponSlot.WeaponActorSlot)
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
    return GetWeaponTagAtSlot(WeaponSlot.CurrentSlot);
}

void UNLCharacterComponent::WeaponAdded(AWeaponActor* Weapon)
{
    BindWeaponDelegate(Weapon);

    AttachWeaponToSocket(Weapon);

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

    const int32 StartupWeaponNum = FMath::Min(int32(WeaponSlot.MaxSlotSize), PS->StartupWeapons.Num());
    if (InitializedStartupWeapons.Num() == StartupWeaponNum)
    {
        bool bAllInitalizedAndValid = true;
        for (int32 i = 0; i < StartupWeaponNum; i++)
        {
            AWeaponActor* InitializedWeapon = InitializedStartupWeapons[i];
            if (!InitializedWeapon->IsInitialized() || !WeaponSlot.WeaponActorSlot.Contains(InitializedWeapon))
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
            const AController* Controller = GetOwningPlayer()->GetController();
            const bool bIsSimulated = !(Controller && Controller->IsLocalPlayerController());

            if (!bIsSimulated)
            {
                // 플레이어의 캐릭터면 어빌리티 활성화 하게 함.
                UAbilitySystemComponent* ASC = GetASC();
                FGameplayTagContainer TagContainer(Ability_WeaponChange_1);
                ASC->TryActivateAbilitiesByTag(TagContainer);

                UpdateWeaponTagSlot();
            }
            else
            {
                // 나중에 접속한 클라이언트 입장에서도 기존에 접속했던 플레이어들의 무기 정보에 맞게 업데이트
                AttachWeaponToHand(GetCurrentWeaponActor());
            }

            // Clear up validation data
            InitializedStartupWeapons.Empty();

            UpdateMeshes(nullptr, bIsSimulated);
        }
    }
}

bool UNLCharacterComponent::CanSwapWeaponSlot(int32 NewWeaponSlot) const
{
    if (!bIsSwappingWeapon && WeaponSlot.CurrentSlot == NewWeaponSlot)
    {
        return false;
    }

    if (0 <= NewWeaponSlot && NewWeaponSlot < WeaponSlot.WeaponActorSlot.Num())
    {
        AWeaponActor* Weapon = WeaponSlot.WeaponActorSlot[NewWeaponSlot];
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
        WeaponSlot.CurrentSlot,
        GetWeaponTagAtSlot(WeaponSwapPendingSlot),
        WeaponSwapPendingSlot
    );
    AWeaponActor* PendingWeapon = GetWeaponActorAtSlot(WeaponSwapPendingSlot);
    CurrentWeaponBulletNumChanged.Broadcast(
        IsValid(PendingWeapon) ? PendingWeapon->GetCurrentBulletNum() : 0
    );

    if (bIsSwappingWeapon)
    {
        if (WeaponSwapPendingSlot == WeaponSlot.CurrentSlot)
        {
            RevertWeaponSwap();
        }
        return;
    }
    bIsSwappingWeapon = true;

    // Block Weapon Ability
    GetASC()->AddLooseGameplayTag(Ability_Block_Weapon);
    GetOwningPlayer()->StopArmsAnimMontage();
    GetOwningPlayer()->StopWeaponAnimMontage();

    if (!IsValid(GetCurrentWeaponActor()) || bSkipHolsterAnim)
    {
        // Start up or weapon drop
        OnWeaponHolstered();
        return;
    }
    
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
    for (int32 i = 1; i < WeaponSlot.MaxSlotSize; i++)
    {
        const int32 Offset = bPrev ? -i : i;
        const int32 NewSlotNum = ((int32)(WeaponSlot.CurrentSlot + WeaponSlot.MaxSlotSize) + Offset) % WeaponSlot.MaxSlotSize;
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
    if (AWeaponActor* Weapon = GetCurrentWeaponActor())
    {
        if (const FWeaponSpreadInfo* SpreadInfo = Weapon->GetSpreadInfo())
        {
            if (bADS)
            {
                return SpreadInfo->ADS;
            }

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

void UNLCharacterComponent::HandleOwnerDestroyed()
{
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
        WeaponActor->Dropped();

        WeaponSlot.WeaponActorSlot[WeaponSlot.CurrentSlot] = nullptr;

        UpdateMeshes();

        UpdateWeaponTagSlot(); // for authorized player

        OnCurrentWeaponDropped();
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
    uint8 Slot = GetFirstEmptySlot();
    if (Slot == WeaponSlot.MaxSlotSize)
    {
        Slot = WeaponSlot.CurrentSlot;
        DropCurrentWeapon();
    }

    AttachWeaponToSocket(WeaponActor);
    WeaponActor->SetOwner(GetOwningPlayer());
    WeaponActor->PickedUp(GetOwningCharacter());

    WeaponSlot.WeaponActorSlot[Slot] = WeaponActor;
    UpdateWeaponTagSlot();

    GetNLASC()->WeaponAdded(WeaponActor);

    TrySwapWeaponSlot(Slot, false, true);
}

bool UNLCharacterComponent::IsWeaponSlotFull() const
{
    for (const AWeaponActor* Weapon : WeaponSlot.WeaponActorSlot)
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
    for (const AWeaponActor* Weapon : WeaponSlot.WeaponActorSlot)
    {
        if (IsValid(Weapon))
        {
            return false;
        }
    }
    return true;
}

uint8 UNLCharacterComponent::GetFirstEmptySlot() const
{
    for (uint8 i = 0; i < WeaponSlot.MaxSlotSize; i++)
    {
        if (!IsValid(WeaponSlot.WeaponActorSlot[i]))
        {
            return i;
        }
    }
    return WeaponSlot.MaxSlotSize;
}
