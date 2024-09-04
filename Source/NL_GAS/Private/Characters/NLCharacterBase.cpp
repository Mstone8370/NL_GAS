// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NLCharacterBase.h"

#include "AbilitySystem/NLAbilitySystemComponent.h"
#include "Components/NLCharacterComponent.h"
#include "Components/DamageTextWidgetComponent.h"
#include "NLFunctionLibrary.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/NLWidgetComponent.h"

ANLCharacterBase::ANLCharacterBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;

    NLCharacterComponent = CreateDefaultSubobject<UNLCharacterComponent>(FName("NLCharacterComponent"));

    CharacterNameWidgetComponent = CreateDefaultSubobject<UNLWidgetComponent>(FName("CharacterNameWidgetComponent"));
    CharacterNameWidgetComponent->SetupAttachment(GetMesh());
}

void ANLCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(ANLCharacterBase, DeathInfo, COND_None, REPNOTIFY_OnChanged);
}

void ANLCharacterBase::BeginPlay()
{
    Super::BeginPlay();
    
    if (HasAuthority())
    {
        UNLFunctionLibrary::LoadHitboxComponents(GetMesh());
    }

    if (CharacterNameWidgetComponent)
    {
        if (CharacterNameWidgetComponent->IsWidgetInitialized())
        {
            BP_OnCharacterNameWidgetInitialized();
        }
        else
        {
            CharacterNameWidgetComponent->OnWidgetInitialized.AddLambda(
                [this]()
                {
                    BP_OnCharacterNameWidgetInitialized();
                }
            );
        }
    }
}

UAbilitySystemComponent* ANLCharacterBase::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void ANLCharacterBase::OnWeaponAdded(AWeaponActor* Weapon)
{
    NLCharacterComponent->WeaponAdded(Weapon);
}

void ANLCharacterBase::ShowDamageText_Implementation(float Value, bool bIsCriticalHit)
{
    if (!DamageTextWidgetComponentClass)
    {
        return;
    }

    if (!(LastDamageText && LastDamageText->IsWatingUpdate()))
    {
        LastDamageText = NewObject<UDamageTextWidgetComponent>(this, DamageTextWidgetComponentClass);
        LastDamageText->RegisterComponent();
        LastDamageText->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        LastDamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    }
    LastDamageText->UpdateValue(Value, bIsCriticalHit);
}

void ANLCharacterBase::OnDead(const AActor* SourceActor)
{
    // On Server

    // TODO: combat interface는 캐릭터가 상속받고, 플레이어가 아닌 캐릭터도 combat interface를 상속
    // 받으면 사망처리를 해줘야하므로, 캐릭터에서 먼저 사망 처리를 하는게 맞는거같음.
    // 근데 플레이어 스테이트도 해당 정보를 가지고있어야하므로 캐릭터에서 플레이어 스테이트에게 알려줘야함.
    // 플레이어 스테이트에서는 동기화를 위해 bIsDead를 레플리케이트 되게 설정하는게 안전함.
    // 사망했을때 처치한 액터도 전달해주기 위해 RPC를 사용할건데 이 정보는 플레이어 컨트롤러한테 필요할듯.
    // 따라서 플레이어 컨트롤러가 RPC를 담당하고, 플레이어 스테이트가 레플리케이트 함.
    // 서버에서는 캐릭터가 먼저 정보를 받는데 서버에서는 한 틱에 처리되므로 문제없을것.
    // 서버에서는 캐릭터의 사망 처리를 제일 먼저 다루지만, 클라이언트에서는 플레이어 스테이트 또는 플레이어
    // 컨트롤러에서 먼저 다루게 될테니 클라이언트에서는 캐릭터의 사망 처리 함수를 저 두 곳에서 호출해줘야함.
    // 플레이어 스테이트에서 먼저 정보를 받으면 처치한 액터 정보 없이 진행하다가
    // 플레이어 컨트롤러에서 처치 액터 정보 받으면 그때 관련 정보 처리. 그 반대의 경우에는 별 문제 없을것.
    // 서버에서 캐릭터가 플레이어 스테이트에 사망 정보를 보내고, 플레이어 스테이트에서 캐릭터에게 사망 처리
    // 함수를 호출하면 어차피 클라이언트에서도 레플리케이트 된 후에 캐릭터의 사망 처리 함수를 호출하지 않을까.
    // 서버에서는 플레이어 컨트롤러에서 클라이언트 RPC외에 추가로 작업할것은 없음.

    /*
    if (!DeathInfo.bIsDead && Info.bIsDead)
    {
        OnDead_Internal(Info);
    }
    */
}

void ANLCharacterBase::Destroyed()
{
    if (NLCharacterComponent)
    {
        NLCharacterComponent->HandleOwnerDestroyed();
    }
    if (CharacterNameWidgetComponent)
    {
        CharacterNameWidgetComponent->OnWidgetInitialized.Clear();
    }

    Super::Destroyed();
}

void ANLCharacterBase::EnableRagdoll()
{
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
}

void ANLCharacterBase::DisableRagdoll()
{
    GetMesh()->SetSimulatePhysics(false);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    if (ACharacter* DefaultCharacter = GetClass()->GetDefaultObject<ACharacter>())
    {
        FVector DefaultLocation = DefaultCharacter->GetMesh()->GetRelativeLocation();
        FRotator DefaultRotation = DefaultCharacter->GetMesh()->GetRelativeRotation();
        GetMesh()->SetRelativeLocation(DefaultLocation);
        GetMesh()->SetRelativeRotation(DefaultRotation);
    }
}

void ANLCharacterBase::OnRespawned()
{
    // On Server
    
    OnRespawned_Internal();
}

void ANLCharacterBase::InitAbilityActorInfo() {}

void ANLCharacterBase::AddStartupAbilities()
{
    if (!HasAuthority())
    {
        return;
    }

    if (UNLAbilitySystemComponent* NLASC = Cast<UNLAbilitySystemComponent>(GetAbilitySystemComponent()))
    {
        NLASC->AddAbilities(StartupAbilities);
    }
}

void ANLCharacterBase::InitDefaultAttribute()
{
    check(AbilitySystemComponent);

    if (DefaultAttribute)
    {
        FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
        ContextHandle.AddInstigator(this, this);
        ContextHandle.AddSourceObject(this);

        const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttribute, 1.f, ContextHandle);

        AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
    }
}

void ANLCharacterBase::OnRep_DeathInfo(FDeathInfo OldDeathInfo)
{
    if (DeathInfo.bIsDead == OldDeathInfo.bIsDead)
    {
        return;
    }

    if (DeathInfo.bIsDead)
    {
        OnDead_Internal(DeathInfo, GetLocalRole() == ROLE_SimulatedProxy);
    }
    else
    {
        OnRespawned_Internal(GetLocalRole() == ROLE_SimulatedProxy);
    }
}

void ANLCharacterBase::OnDead_Internal(const FDeathInfo& Info, bool bSimulated)
{
    DeathInfo = Info;

    EnableRagdoll();
    GetWorldTimerManager().SetTimer(DeathRagdollTimerHandle, this, &ANLCharacterBase::OnDeathRagdollTimeEnded, DeathRagdollTime, false);

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (NLCharacterComponent)
    {
        NLCharacterComponent->HandleOwnerDeath();
    }

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->bWantsToCrouch = false;
        GetCharacterMovement()->DisableMovement();
    }
}

void ANLCharacterBase::OnDeathRagdollTimeEnded()
{
    GetMesh()->SetVisibility(false);

    DisableRagdoll();

    SetActorLocation(FVector(0.f, 0.f, 0.f));
}

void ANLCharacterBase::OnRespawned_Internal(bool bSimulated)
{
    DeathInfo = FDeathInfo(false);

    if (HasAuthority())
    {
        InitDefaultAttribute();
    }

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    GetMesh()->SetVisibility(true);

    if (ACharacter* DefaultCharacter = Cast<ACharacter>(GetClass()->GetDefaultObject()))
    {
        const FVector MeshLocation = DefaultCharacter->GetMesh()->GetRelativeLocation();
        const FRotator MeshRotation = DefaultCharacter->GetMesh()->GetRelativeRotation();

        GetMesh()->SetRelativeLocationAndRotation(MeshLocation, MeshRotation);

        GetCapsuleComponent()->SetCollisionEnabled(DefaultCharacter->GetCapsuleComponent()->GetCollisionEnabled());
    }

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->SetMovementMode(GetCharacterMovement()->DefaultLandMovementMode);
    }
}

void ANLCharacterBase::GetAimPoint(FVector& OutViewLocation, FRotator& OutViewRotation) const
{
    OutViewLocation = GetActorLocation();
    OutViewRotation = GetActorRotation();
}

bool ANLCharacterBase::IsWeaponSlotFull() const
{
    if (NLCharacterComponent)
    {
        return NLCharacterComponent->IsWeaponSlotFull();
    }
    return false;
}

void ANLCharacterBase::SetAsEnemy_Implementation()
{
    
}

void ANLCharacterBase::SetAsFriendly_Implementation(bool bSelf)
{

}

void ANLCharacterBase::SetAsNeutral_Implementation()
{

}
