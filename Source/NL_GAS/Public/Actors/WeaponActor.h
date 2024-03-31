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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	TObjectPtr<UTaggedWeaponInfo> StartupWeaponInfo;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> ViewWeaponMesh;

public:
	void InitalizeWeapon(const FWeaponInfo* Info);

};
