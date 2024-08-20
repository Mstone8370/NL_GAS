// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/NLProjectile.h"
#include "NLProjectile_Bullet.generated.h"

class UMaterialInterface;
class UFXSystemAsset;

/**
 * 
 */
UCLASS()
class NL_GAS_API ANLProjectile_Bullet : public ANLProjectile
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	TObjectPtr<UMaterialInterface> HitImpactDecalMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	FVector DecalSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	float DecalLifeSpan;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "FX")
	TObjectPtr<UFXSystemAsset> HitImpactFX;
	
protected:
	virtual void BeginPlay() override;

	virtual void OnProjectileHit(const FHitResult& HitResult) override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnProjectileHit_BP(const FHitResult& HitResult);

	virtual void HandleDestroy(AActor* DestroyedActor) override;

	virtual void ApplyDamage(const FHitResult& HitResult) override;

	UFUNCTION(BlueprintCallable)
	void HandleHitFX(const FHitResult& HitResult);

	bool bHit = false;
};
