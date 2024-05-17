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
    float CameraFOVMultiplier;

    UPROPERTY(EditAnywhere)
    float ViewModelHorizontalFOV;

    UPROPERTY(EditAnywhere)
    float LookSensitivityMultiplier;

    UPROPERTY(EditAnywhere)
    TObjectPtr<UCurveVector> LoopingControlShakeCurve;
};

USTRUCT(BlueprintType)
struct FHitboxInfoRow : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere)
    FName BoneName;

    UPROPERTY(EditAnywhere)
    FVector Location;

    UPROPERTY(EditAnywhere)
    FRotator Rotation;

    UPROPERTY(EditAnywhere)
    FVector Extend;

    UPROPERTY(EditAnywhere)
    bool IsWeakHitbox;
};
