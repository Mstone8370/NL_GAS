// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NLDataTableRows.generated.h"

class UCurveVector;

USTRUCT(BlueprintType)
struct FFOVModifyValue : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    float CameraFOVMultiplier = 1.f;

    UPROPERTY(EditAnywhere)
    float CameraFOVAdditive = 0.f;

    UPROPERTY(EditAnywhere)
    float ViewModelHorizontalFOV = 80.f;

    UPROPERTY(EditAnywhere)
    float LookSensitivityMultiplier = 1.f;

    UPROPERTY(EditAnywhere)
    TObjectPtr<UCurveVector> LoopingControlShakeCurve = nullptr;
};

USTRUCT(BlueprintType)
struct FHitboxInfoRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    FName BoneName = NAME_None;

    UPROPERTY(EditAnywhere)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere)
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere)
    FVector Extend = FVector::ZeroVector;

    UPROPERTY(EditAnywhere)
    bool bIsWeakHitbox = false;
};
