// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/NLGameMode.h"
#include "NLGameMode_FiringRange.generated.h"

/**
 * 
 */
UCLASS()
class NL_GAS_API ANLGameMode_FiringRange : public ANLGameMode
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AActor> TargetActorClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 PoolSize = 10;

protected:
	UPROPERTY()
	TArray<TObjectPtr<AActor>> TargetActorPool;

	UFUNCTION(BlueprintCallable)
	AActor* ReclaimTargetActor();

	UFUNCTION(BlueprintCallable)
	void ReturnTargetActor(AActor* TargetActor);

	void PrintPoolStatus() const;
};
