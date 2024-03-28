// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/WeaponInfo.h"
#include "WeaponActor.generated.h"

UCLASS(Blueprintable, BlueprintType)
class NL_GAS_API AWeaponActor : public AActor
{
	GENERATED_BODY()
	
public:
	AWeaponActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	TObjectPtr<UTaggedWeaponInfo> StartupWeaponInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> ViewWeaponMesh;

	// TArray<TObjectPtr<UMaterialInstanceDynamic>> ViewMaterials;

public:
	void InitalizeWeapon(const FWeaponInfo* Info);

};
