// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "NLAbilitySystemGlobals.generated.h"

/**
 * 
 */
UCLASS()
class NL_GAS_API UNLAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

	// AbilitySystemComponent에서 MakeEffectContext 함수의 내부에선 아래의 함수로 FGameplayEffectContext 생성함.
	// 이 함수를 override해서 FGameplayEffectContext 상속받은 FAuraGameplayEffectContext를 리턴하도록 변경하면 모든 곳에 적용됨.
	// Config/DefaultGame.ini 파일 수정으로 이 UAuraAbilitySystemGlobals를 사용하게 설정 가능
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
