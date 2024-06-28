// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AttributeSet/NLAttributeSet.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/NLAbilitySystemTypes.h"
#include "Player/NLPlayerController.h"
#include "AbilitySystem/NLAbilitySystemTypes.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "NLGameplayTags.h"
#include "Components/HitboxComponent.h"

void UNLAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    /**
    * Default Attribute
    */
    DOREPLIFETIME_CONDITION_NOTIFY(UNLAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UNLAttributeSet, Health, COND_None, REPNOTIFY_Always);
}

void UNLAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
    }
}

void UNLAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    if (Attribute == GetHealthAttribute())
    {
        
    }
}

void UNLAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    FEffectContextParams Params;
    SetEffectContextParams(Data, Params);

    FNLGameplayEffectContext* NLContext = static_cast<FNLGameplayEffectContext*>(Params.ContextHandle.Get());
    ANLPlayerController* SourceNLPC = Cast<ANLPlayerController>(Params.SourceController);
    ANLPlayerController* TargetNLPC = Cast<ANLPlayerController>(Params.TargetController);

    if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
    {
        float LocalIncomingDamage = GetIncomingDamage();
        SetIncomingDamage(0.f);
        LocalIncomingDamage = FMath::Floor(LocalIncomingDamage);

        FGameplayTag DamageType = FGameplayTag();
        if (NLContext->DamageType.Get())
        {
            DamageType = *NLContext->DamageType.Get();
        }

        FVector DamageOrigin = FVector::ZeroVector;
        if (NLContext->HasOrigin())
        {
            DamageOrigin = NLContext->GetOrigin();
        }
        else
        {
            if (SourceNLPC)
            {
                DamageOrigin = SourceNLPC->GetPawn()->GetActorLocation();
            }
            else if (TargetNLPC)
            {
                DamageOrigin = TargetNLPC->GetPawn()->GetActorLocation();
            }
        }

        bool bIsCriticalHit = false;
        if (NLContext->bCanCriticalHit)
        {
            if (NLContext->GetHitResult()->GetComponent())
            {
                if (UHitboxComponent* Hitbox = Cast<UHitboxComponent>(NLContext->GetHitResult()->GetComponent()))
                {
                    bIsCriticalHit = Hitbox->IsWeakHitbox();
                    if (bIsCriticalHit)
                    {
                        LocalIncomingDamage *= Hitbox->GetCriticalHitDamageMultiplier();
                    }
                }
            }
        }

        if (NLContext->KnockbackMagnitude > 0.f && NLContext->GetHitResult())
        {
            FHitResult* HitRes = NLContext->GetHitResult();
            const FVector KnockbackDirection = (HitRes->TraceEnd - HitRes->TraceStart).GetSafeNormal();
            const FVector KnockbackForce = KnockbackDirection * NLContext->KnockbackMagnitude;
            if (ACharacter* TargetCharacter = Cast<ACharacter>(Params.TargetAvatarActor))
            {
                TargetCharacter->GetCharacterMovement()->AddImpulse(KnockbackForce, true);
            }
        }

        SetHealth(FMath::Max(GetHealth() - LocalIncomingDamage, 0.f));

        if (Params.SourceAvatarActor != Params.TargetAvatarActor && SourceNLPC)
        {
            SourceNLPC->OnCausedDamage(LocalIncomingDamage, bIsCriticalHit, Params.TargetAvatarActor);
        }

        if (TargetNLPC)
        {
            TargetNLPC->OnTakenDamage(NLContext->GetHitResult(), DamageOrigin, bIsCriticalHit, DamageType);
        }
    }
}

void UNLAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldHealth) const
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNLAttributeSet, MaxHealth, OldHealth);
}

void UNLAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UNLAttributeSet, Health, OldHealth);
}

void UNLAttributeSet::SetEffectContextParams(const FGameplayEffectModCallbackData& Data, FEffectContextParams& OutParams) const
{
    OutParams = FEffectContextParams();

    OutParams.ContextHandle = Data.EffectSpec.GetContext();

    OutParams.SourceASC = OutParams.ContextHandle.GetOriginalInstigatorAbilitySystemComponent();
    if (OutParams.SourceASC)
    {
        OutParams.SourceAvatarActor = OutParams.SourceASC->GetAvatarActor();
        if (OutParams.SourceAvatarActor)
        {
            if (APawn* SourcePawn = Cast<APawn>(OutParams.SourceAvatarActor))
            {
                OutParams.SourceController = SourcePawn->GetController();
                if (OutParams.SourceController)
                {
                    OutParams.SourcePC = Cast<APlayerController>(OutParams.SourceController);
                }
            }
        }
    }

    if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
    {
        OutParams.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
        OutParams.TargetASC = Data.Target.AbilityActorInfo->AbilitySystemComponent.Get();
        if (APawn* TargetPawn = Cast<APawn>(OutParams.TargetAvatarActor))
        {
            OutParams.TargetController = TargetPawn->GetController();
            if (OutParams.TargetController)
            {
                OutParams.TargetPC = Cast<APlayerController>(OutParams.TargetController);
            }
        }
    }
}
