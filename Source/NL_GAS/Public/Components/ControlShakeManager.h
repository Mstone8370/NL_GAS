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

protected:
	void UpdateShakes(float DeltaTime);

	void ApplyShake(float DeltaTime);

	FRotator ShakeSum;

	FRotator ShakeSumPrev;

	FRotator DeltaShake;

public:
	void AddShake(float InDuration, UCurveVector* InCurve, FRotator InShakeMagnitude, bool bInLoop = false);

	UFUNCTION(BlueprintCallable)
	void AddShake(FControlShakeParams Params, bool bInLoop = false);

	UFUNCTION(BlueprintCallable)
	void WeaponFired(const FGameplayTag& WeaponTag);

	UFUNCTION(BlueprintCallable)
	void ClearLoopingShake();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWeaponRecoilPattern> RecoilPatternData;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ACharacter> OwningCharacter;

	UPROPERTY()
	TArray<TObjectPtr<UControlShake>> ActiveShakes;

	UPROPERTY()
	TArray<TObjectPtr<UControlShake>> ExpiredPool;

	UPROPERTY()
	TObjectPtr<UControlShake> LoopingShake;

	UControlShake* ReclaimShakeFromExpiredPool();

	ACharacter* GetOwningCharacter();

	UPROPERTY()
	TMap<FGameplayTag, int32> RecoilOffsetsMap;

	UPROPERTY()
	TMap<FGameplayTag, FTimerHandle> RecoilOffsetResetTimersMap;

	void ResetRecoilOffset(const FGameplayTag& WeaponTag);

public:
	int GetRecoilOffset(const FGameplayTag& WeaponTag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FRotator GetDeltaShake() const { return DeltaShake; }
};
