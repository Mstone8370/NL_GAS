// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Objects/ControlShake.h"
#include "GameplayTagContainer.h"
#include "ControlShakeManager.generated.h"

class UWeaponRecoilPattern;
struct FTaggedAimPunch;

USTRUCT()
struct FPooledControlShakes
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<UControlShake>> PooledShakes;
};

/**
* TODO:
* Shake 인스턴스에 필요한 정보: Shake 커브, 지속 시간, 각도, 게임플레이 태그, 루프 여부
* 재사용 되는 정보: Shake 커브, 지속 시간, 게임플레이 태그
* 재사용 되는 정보는 게임플레이 태그에 따라 결정됨
* 따라서 각 쉐이크에 필요한 정보는 하나의 데이터 에셋에 태그에 따라 분류
* 따라서 쉐이크 인스턴스를 처음 설정할때에만 데이터 에셋을 참조하도록
* 그 후엔 PoolMap에 태그에 따라 분류 & 저장되어 필요할때 재사용해 각도만 설정
* 반동 패턴은 별도의 데이터 에셋에 저장해서 사용
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
	void AddShake(float InDuration, UCurveVector* InCurve, FRotator InShakeMagnitude, bool bInLoop = false);

	void AddShake(const FGameplayTag& ShakeTag, FRotator InShakeMagnitude, bool bInLoop = false);

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
	TMap<FGameplayTag, FPooledControlShakes> ExpiredPoolMap;

	UPROPERTY()
	TObjectPtr<UControlShake> LoopingShake;

	UControlShake* ReclaimShakeFromExpiredPool();

	UControlShake* ReclaimShakeFromExpiredPoolMap(const FGameplayTag& ShakeTag);

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
