// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/NLGameplayAbility_WeaponPrimary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "NLGameplayTags.h"

void UNLGameplayAbility_WeaponPrimary::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    /**
    * Sprint�� ���� �����÷��� �±׸� �����ؾ� �ٽ� �޸� �� ����.
    * �� �����Ƽ�� ª�� �ð� ���� �����ϴ� ��츦 ����ؼ� �±� ���� ���� �ణ�� �����̸� ��.
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
