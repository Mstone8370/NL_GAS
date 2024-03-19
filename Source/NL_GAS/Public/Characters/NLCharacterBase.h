// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NLCharacterBase.generated.h"

UCLASS()
class NL_GAS_API ANLCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	ANLCharacterBase();

protected:
	virtual void BeginPlay() override;

};
