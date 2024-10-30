// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Objects/ControlShake.h"
#include "GameplayTagContainer.h"
#include "ControlShakeManager.generated.h"

class UWeaponRecoilPattern;
struct FTaggedAimPunch;
class UControlShakeData;
struct FTaggedControlShake;

USTRUCT()
struct FPooledControlShakes
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<UControlShake>> PooledShakes;

	FORCEINLINE int32 Num() const { return PooledShakes.Num(); }

	FORCEINLINE int32 Add(const TObjectPtr<UControlShake>& Item) { return PooledShakes.Add(Item); }

	FORCEINLINE TObjectPtr<UControlShake> Pop(bool bAllowShrinking = true) { return PooledShakes.Pop(bAllowShrinking); }
};

/**
* TODO:
* Shake �ν��Ͻ��� �ʿ��� ����: Shake Ŀ��, ���� �ð�, ����, �����÷��� �±�, ���� ����
* ���� �Ǵ� ����: Shake Ŀ��, ���� �ð�, �����÷��� �±�
* ���� �Ǵ� ������ �����÷��� �±׿� ���� ������
* ���� �� ����ũ�� �ʿ��� ������ �ϳ��� ������ ���¿� �±׿� ���� �з�
* ���� ����ũ �ν��Ͻ��� ó�� �����Ҷ����� ������ ������ �����ϵ���
* �� �Ŀ� PoolMap�� �±׿� ���� �з� & ����Ǿ� �ʿ��Ҷ� ������ ������ ����
* �ݵ� ������ ������ ������ ���¿� �����ؼ� ���
*/

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

	int32 MaxPoolSize;

public:
	UFUNCTION(meta = (DeprecatedFunction, DeprecationMessage = "[Deprecated function] UControlShakeManager::AddShake(float, UCurveVector*, FRotator, bool)"))
	void AddShake(float InDuration, UCurveVector* InCurve, FRotator InShakeMagnitude, bool bInLoop = false);

	void AddShake(const FGameplayTag& ShakeTag, FRotator InShakeMagnitude, bool bInLoop = false);

	UFUNCTION(BlueprintCallable)
	void WeaponFired(const FGameplayTag& WeaponTag);

	UFUNCTION(BlueprintCallable)
	void ClearLoopingShake();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWeaponRecoilPattern> RecoilPatternData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UControlShakeData> ControlShakeData;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ACharacter> OwningCharacter;

	UPROPERTY()
	TArray<TObjectPtr<UControlShake>> ActiveShakes;

	UPROPERTY()
	TMap<FGameplayTag, FPooledControlShakes> ExpiredPoolMap;

	UPROPERTY()
	TObjectPtr<UControlShake> LoopingShake;

	UControlShake* ReclaimShakeFromExpiredPoolMap(const FGameplayTag& InShakeTag);

	ACharacter* GetOwningCharacter();

	UPROPERTY()
	TMap<FGameplayTag, int32> RecoilOffsetsMap;

	UPROPERTY()
	TMap<FGameplayTag, FTimerHandle> RecoilOffsetResetTimersMap;

	void ResetRecoilOffset(const FGameplayTag& WeaponTag);

	const FTaggedControlShake* GetControlShakeData(const FGameplayTag& InShakeTag) const;

public:
	int GetRecoilOffset(const FGameplayTag& WeaponTag) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FRotator GetDeltaShake() const { return DeltaShake; }
};
