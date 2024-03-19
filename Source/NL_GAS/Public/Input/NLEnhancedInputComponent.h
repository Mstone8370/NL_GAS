// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "Input/Data/InputConfig.h"
#include "NLEnhancedInputComponent.generated.h"

/**
 * 
 */
UCLASS()
class NL_GAS_API UNLEnhancedInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
	
public:
	template<class UserClass, typename FuncType>
	void BindActionByTag(const UInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func)
	{					
		checkf(InputConfig, TEXT("InputConfig Data Asset in BP_NLPlayerController is not valid."));
		if (const UInputAction* IA = InputConfig->FindInputActionByTag(InputTag))
		{
			BindAction(IA, TriggerEvent, Object, Func);
		}
	}
};
