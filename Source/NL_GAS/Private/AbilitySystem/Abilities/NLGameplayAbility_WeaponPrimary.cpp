// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/NLGameplayAbility_WeaponPrimary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "NLGameplayTags.h"

void UNLGameplayAbility_WeaponPrimary::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    /**
    * Sprint를 막는 게임플레이 태그를 제거해야 다시 달릴 수 있음.
    * 이 어빌리티를 짧은 시간 내에 재사용하는 경우를 고려해서 태그 제거 전에 약간의 딜레이를 줌.
    */
    FTimerDelegate Dele;
    Dele.BindLambda(
        [Actor = GetAvatarActorFromActorInfo()]()
        {
            FGameplayTagContainer TagContainer(Ability_Block_Sprint);
            UAbilitySystemBlueprintLibrary::RemoveLooseGameplayTags(
                Actor,
                TagContainer
            );
        }
    );
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, Dele, .2f, false);
}
