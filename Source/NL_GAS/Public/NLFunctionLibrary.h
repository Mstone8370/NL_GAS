// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/NLAbilitySystemTypes.h"
#include "NLFunctionLibrary.generated.h"

struct FWeaponInfo;
struct FTaggedAnimMontageInfo;
struct FUIWeaponInfo;
struct FParticleInfo;
struct FParticleSpawnInfo;
struct FProjectileInfo;
struct FProjectileSpawnInfo;
class ANLProjectile;

/**
 * 
 */
UCLASS()
class NL_GAS_API UNLFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static const FWeaponInfo* GetWeaponInfoByTag(const UObject* WorldContextObject, const FGameplayTag& WeaponTag);

	static const FTaggedAnimMontageInfo* GetAnimMontageByTag(const UObject* WorldContextObject, const FGameplayTag& WeaponTag, const FGameplayTag& MontageTag);

	UFUNCTION(BlueprintCallable, Category = "NLFunctionLibrary")
	static void ApplyDamageEffect(const FDamageEffectParams& Params);

	UFUNCTION(BlueprintCallable, Category = "NLFunctionLibrary")
	static FString MakeHitboxInfoDataTablePath(const USkeletalMeshComponent* SkeletalMeshComponent);

	UFUNCTION(BlueprintCallable, Category = "NLFunctionLibrary")
	static void LoadHitboxComponents(USkeletalMeshComponent* SkeletalMeshComponent);

	UFUNCTION(BlueprintCallable, Category = "NLFunctionLibrary")
	static bool AssetExists(FString FullPath);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "NLFunctionLibrary")
	static FString GetPlayerName(AActor* Actor);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "NLFunctionLibrary")
	static AActor* GetAbilitySystemAvatarActor(UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "NLFunctionLibrary")
	static AController* GetAbilitySystemController(UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "NLFunctionLibrary")
	static APlayerController* GetAbilitySystemPlayerController(UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "NLFunctionLibrary")
	static APlayerState* GetPlayerStateFromActor(AActor* PawnOrController);

	UFUNCTION(BlueprintCallable, Category = "NLFunctionLibrary|Particles")
	static void SpawnSingleParticleByParticleInfo(const UObject* WorldContextObject, const FParticleInfo& ParticleInfo, const FParticleSpawnInfo& SpawnInfo);
	
	UFUNCTION(BlueprintCallable, Category = "NLFunctionLibrary|Particles")
	static void SpawnSingleParticleByTag(const UObject* WorldContextObject, const FGameplayTag& ParticleTag, const FParticleSpawnInfo& SpawnInfo);

	UFUNCTION(BlueprintCallable, Category = "NLFunctionLibrary|Particles")
	static void SpawnMultipleParticleByParticleInfo(const UObject* WorldContextObject, const FParticleInfo& ParticleInfo, const TArray<FParticleSpawnInfo>& SpawnInfos);

	UFUNCTION(BlueprintCallable, Category = "NLFunctionLibrary|Particles")
	static void SpawnMultipleParticleByTag(const UObject* WorldContextObject, const FGameplayTag& ParticleTag, const TArray<FParticleSpawnInfo>& SpawnInfos);

	UFUNCTION(BlueprintCallable, Category = "NLFunctionLibrary|Projectiles")
	static ANLProjectile* SpawnSingleProjectileByProjectileInfo(const UObject* WorldContextObject, const FProjectileInfo& ProjectileInfo, const FProjectileSpawnInfo& SpawnInfo, APawn* Instigator);

	UFUNCTION(BlueprintCallable, Category = "NLFunctionLibrary|Projectiles")
	static ANLProjectile* SpawnSingleProjectileByTag(const UObject* WorldContextObject, const FGameplayTag& ProjectileTag, const FProjectileSpawnInfo& SpawnInfo, APawn* Instigator);

	UFUNCTION(BlueprintCallable, Category = "NLFunctionLibrary|Projectiles")
	static void SpawnMultipleProjectileByProjectileInfo(const UObject* WorldContextObject, const FProjectileInfo& ProjectileInfo, const TArray<FProjectileSpawnInfo>& SpawnInfos, APawn* Instigator, TArray<ANLProjectile*>& OutProjectiles);

	UFUNCTION(BlueprintCallable, Category = "NLFunctionLibrary|Projectiles")
	static void SpawnMultipleProjectileByTag(const UObject* WorldContextObject, const FGameplayTag& ProjectileTag, const TArray<FProjectileSpawnInfo>& SpawnInfos, APawn* Instigator, TArray<ANLProjectile*>& OutProjectiles);
};
