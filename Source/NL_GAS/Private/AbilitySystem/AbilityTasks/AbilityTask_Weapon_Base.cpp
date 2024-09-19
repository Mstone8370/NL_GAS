// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AbilityTasks/AbilityTask_Weapon_Base.h"

#include "NL_GAS/NL_GAS.h"
#include "AbilitySystemComponent.h"
#include "Interface/PlayerInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

UAbilityTask_Weapon_Base* UAbilityTask_Weapon_Base::CreateWeaponTargetData(UGameplayAbility* OwningAbility, float InTraceLength, uint8 InTraceCount)
{
    UAbilityTask_Weapon_Base* MyObj = NewAbilityTask<UAbilityTask_Weapon_Base>(OwningAbility);
    MyObj->TraceLength = InTraceLength;
    MyObj->TraceCount = InTraceCount;

    return MyObj;
}

void UAbilityTask_Weapon_Base::Activate()
{
    UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
    
    check(ASC);
    check(Ability);

    if (Ability->IsLocallyControlled())
    {
        // On Client

        SendWeaponTargetData();
    }
    else
    {
        // On Server

        const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
        const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();

        ASC->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(
            this, &UAbilityTask_Weapon_Base::OnTargetDataReplicatedCallback
        );
        ASC->AbilityTargetDataCancelledDelegate(SpecHandle, ActivationPredictionKey).AddUObject(
            this, &UAbilityTask_Weapon_Base::OnTargetDataReplicatedCancelledCallback
        );

        // TargetData�� �̹� �޾����� TargetSetDelegate�� broadcast�ϰ� true ����.
        const bool bCalledDelegate = ASC->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);
        if (!bCalledDelegate)
        {
            // TargetData�� ���� ���� �ʾ����� ��ٸ�.
            // AbilitySystemComponent�� AbilityTargetDataMap�� �־ ����ص�.
            SetWaitingOnRemotePlayerData();
        }
    }
}

void UAbilityTask_Weapon_Base::SendWeaponTargetData()
{
    // �� scope�� �ִ� �͵��� prediction�Ǿ���Ѵٴ� �ǹ�
    FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());

    FGameplayAbilityTargetDataHandle DataHandle;

    APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
    AActor* AvatarActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
    check(PC);
    check(AvatarActor);
    
    FVector ViewStart;
    FRotator ViewRot;
    PC->GetPlayerViewPoint(ViewStart, ViewRot);
    ViewRot = PC->GetControlRotation();

    float SpreadVal = IPlayerInterface::Execute_GetWeaponSpreadValue(AvatarActor);

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(AvatarActor);

    for (uint8 i = 0; i < TraceCount; i++)
    {
        FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
        
        FVector ViewDir = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(ViewRot.Vector(), SpreadVal);
        
        FVector TraceStart = ViewStart;
        FVector TraceEnd = TraceStart + (ViewDir * TraceLength);

        FHitResult HitRes;
        SingleLineTrace(AvatarActor, ActorsToIgnore, HitRes, TraceStart, TraceEnd, TraceLength);

        Data->HitResult = HitRes;
        DataHandle.Add(Data);
    }

    FPredictionKey PK = AbilitySystemComponent->ScopedPredictionKey;
    FPredictionKey NewPK = AbilitySystemComponent->ScopedPredictionKey.CreateNewPredictionKey(AbilitySystemComponent.Get());

    FScopedPredictionWindow ScpedPrediction(AbilitySystemComponent.Get(), NewPK);

    AbilitySystemComponent->ServerSetReplicatedTargetData(
        GetAbilitySpecHandle(),
        GetActivationPredictionKey(),
        DataHandle,
        FGameplayTag(),
        NewPK
    );

    if (ShouldBroadcastAbilityTaskDelegates())
    {
        // Prediction
        ValidData.Broadcast(DataHandle);
    }
}

void UAbilityTask_Weapon_Base::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag)
{
    // TargetData�� �ޱ� ���� ��ٸ� ��쿡�� AbilitySystemComponent�� AbilityTargetDataMap�� �־�ιǷ�, �� �����͸� ����.
    AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());
    
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        ValidData.Broadcast(DataHandle);
    }

    EndTask();
}

void UAbilityTask_Weapon_Base::OnTargetDataReplicatedCancelledCallback()
{
    if (ShouldBroadcastAbilityTaskDelegates())
    {
        Cancelled.Broadcast(FGameplayAbilityTargetDataHandle());
    }
    EndTask();
}

void UAbilityTask_Weapon_Base::SingleLineTrace(const UObject* WorldContextObject, TArray<AActor*> ActorsToIgnore, FHitResult& OutHitResult, FVector TraceStart, FVector TraceEnd, float Length)
{
    OutHitResult = FHitResult();

    UKismetSystemLibrary::LineTraceSingle(
        WorldContextObject,
        TraceStart,
        TraceEnd,
        UEngineTypes::ConvertToTraceType(ECC_BulletHitscan),
        false,
        ActorsToIgnore,
        EDrawDebugTrace::None,
        OutHitResult,
        true,
        FLinearColor::Red,
        FLinearColor::Green,
        5.f
    );
}
