// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/NLAbilitySystemComponent.h"

#include "NLGameplayTags.h"
#include "AbilitySystem/Abilities/NLGameplayAbility.h"

void UNLAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
    if (!InputTag.IsValid())
    {
        return;
    }

    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        bool bIsInputTagAbility = AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag);
        if (AbilitySpec.Ability && bIsInputTagAbility)
        {
            InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
            InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
        }
    }
}

void UNLAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
    if (!InputTag.IsValid())
    {
        return;
    }

    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        bool bIsInputTagAbility = AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag);
        if (AbilitySpec.Ability && bIsInputTagAbility)
        {
            InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
            InputHeldSpecHandles.Remove(AbilitySpec.Handle);
        }
    }
}

void UNLAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	if (HasMatchingGameplayTag(Input_Block_Ability))
	{
		ClearAbilityInput();
		return;
	}

	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();

	//@TODO: See if we can use FScopedServerAbilityRPCBatcher ScopedRPCBatcher in some of these loops

	//
	// Process all abilities that activate when the input is held.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputHeldSpecHandles)
	{
		if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				bool bIsWeaponHolstered = AbilitySpec->DynamicAbilityTags.HasTagExact(Status_Weapon_Holstered);
				if (!bIsWeaponHolstered)
				{
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				}
			}
		}
	}

	//
	// Process all abilities that had their input pressed this frame.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputPressedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputPressed(*AbilitySpec);

					InvokeReplicatedEvent(
						EAbilityGenericReplicatedEvent::InputPressed,
						AbilitySpec->Handle,
						AbilitySpec->ActivationInfo.GetActivationPredictionKey()
					);
				}
				else
				{
					bool bIsWeaponHolstered = AbilitySpec->DynamicAbilityTags.HasTagExact(Status_Weapon_Holstered);
					if (!bIsWeaponHolstered)
					{
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
				}
			}
		}
	}

	//
	// Try to activate all the abilities that are from presses and holds.
	// We do it all at once so that held inputs don't activate the ability
	// and then also send a input event to the ability because of the press.
	//
	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}

	//
	// Process all abilities that had their input released this frame.
	//
	for (const FGameplayAbilitySpecHandle& SpecHandle : InputReleasedSpecHandles)
	{
		if (FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputReleased(*AbilitySpec);

					InvokeReplicatedEvent(
						EAbilityGenericReplicatedEvent::InputReleased,
						AbilitySpec->Handle,
						AbilitySpec->ActivationInfo.GetActivationPredictionKey()
					);
				}
			}
		}
	}

	//
	// Clear the cached ability handles.
	//
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UNLAbilitySystemComponent::ClearAbilityInput()
{
    InputPressedSpecHandles.Reset();
    InputReleasedSpecHandles.Reset();
    InputHeldSpecHandles.Reset();
}

void UNLAbilitySystemComponent::AddAbilities(const TMap<FGameplayTag, TSubclassOf<UGameplayAbility>>& Abilities)
{
	// On Server

	for (const TPair<FGameplayTag, TSubclassOf<UGameplayAbility>>& AbilityInfo : Abilities)
	{
		const FGameplayTag& InputTag = AbilityInfo.Key;
		const TSubclassOf<UGameplayAbility> AbilityClass = AbilityInfo.Value;
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		AbilitySpec.DynamicAbilityTags.AddTag(InputTag);
		GiveAbility(AbilitySpec);
	}
}

void UNLAbilitySystemComponent::TryChangeWeaponSlot(int32 NewWeaponSlot)
{
}

void UNLAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();
}
