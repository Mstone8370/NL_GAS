// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Objects/ControlShake.h"
#include "GameplayTagContainer.h"
#include "ControlShakeManager.generated.h"

class UWeaponRecoilPattern;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NL_GAS_API UControlShakeManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	UControlShakeManager();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void UpdateShakes(float DeltaTime);

	void AddShake(float InDuration, UCurveVector* InCurve, FRotator InShakeMagnitude);

	UFUNCTION(BlueprintCallable)
	void AddShake(FControlShakeParams Params);

	UFUNCTION(BlueprintCallable)
	void WeaponFired(const FGameplayTag& WeaponTag);

	UPROPERTY(BlueprintReadOnly)
	FRotator TotalShake;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWeaponRecoilPattern> RecoilPatternData;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ACharacter> OwningCharacter;

	UPROPERTY()
	TArray<TObjectPtr<UControlShake>> ActiveShakes;

	UPROPERTY()
	TArray<TObjectPtr<UControlShake>> ExpiredPool;

	UControlShake* ReclaimShakeFromExpiredPool();

	ACharacter* GetOwningCharacter();

	TMap<FGameplayTag, int32> RecoilOffsetsMap;

	TMap<FGameplayTag, FTimerHandle> RecoilOffsetResetTimersMap;

	void ResetRecoilOffset(const FGameplayTag& WeaponTag);

public:
	int GetRecoilOffset(const FGameplayTag& WeaponTag) const;
};
