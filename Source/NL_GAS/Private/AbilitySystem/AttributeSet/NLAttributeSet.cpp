// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AttributeSet/NLAttributeSet.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/NLAbilitySystemTypes.h"
#include "Player/NLPlayerController.h"
#include "AbilitySystem/NLAbilitySystemTypes.h"

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
        UE_LOG(LogTemp, Warning, TEXT("[PreAttributeChange] Health"));
        NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
    }
}

void UNLAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    if (Attribute == GetHealthAttribute())
    {
        UE_LOG(LogTemp, Warning, TEXT("[PostAttributeChange] Health"));
    }
}

void UNLAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    FNLGameplayEffectContext* Context = static_cast<FNLGameplayEffectContext*>(Data.EffectSpec.GetContext().Get());
    UAbilitySystemComponent* SourceASC = Data.EffectSpec.GetContext().GetOriginalInstigatorAbilitySystemComponent();
    AActor* SourceAvatarActor = nullptr;
    AController* SourceController = nullptr;
    APlayerController* SourcePC = nullptr;
    ANLPlayerController* SourceNLPC = nullptr;
    if (SourceASC)
    {
        SourceAvatarActor = SourceASC->GetAvatarActor();
        if (SourceAvatarActor)
        {
            if (APawn* SourcePawn = Cast<APawn>(SourceAvatarActor))
            {
                SourceController = SourcePawn->GetController();
                if (SourceController)
                {
                    SourcePC = Cast<APlayerController>(SourceController);
                    SourceNLPC = Cast<ANLPlayerController>(SourceController);
                }
            }
        }
    }

    AActor* TargetAvatarActor = nullptr;
    if (Data.Target.AbilityActorInfo)
    {
       TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
    }

    if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
    {
        float LocalIncomingDamage = GetIncomingDamage();
        SetIncomingDamage(0.f);
        LocalIncomingDamage = FMath::Floor(LocalIncomingDamage);

        // TODO: Check if it's critical hit.
        bool bIsCriticalHit = false;
        if (Context->bCanCriticalHit)
        {
            Context->GetHitResult()->GetComponent();
        }

        SetHealth(FMath::Max(GetHealth() - LocalIncomingDamage, 0.f));

        if (SourceAvatarActor != TargetAvatarActor && SourceNLPC)
        {
            SourceNLPC->OnCausedDamage(LocalIncomingDamage, bIsCriticalHit, TargetAvatarActor);
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
