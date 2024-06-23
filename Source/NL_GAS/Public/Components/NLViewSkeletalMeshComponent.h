// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameplayTagContainer.h"
#include "NLViewSkeletalMeshComponent.generated.h"

USTRUCT()
struct FTaggedMaterialInstanceDynamic
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> MatInstDynamics;

	FTaggedMaterialInstanceDynamic() { Clear(); }

	~FTaggedMaterialInstanceDynamic() { Clear(); }

	void Add(UMaterialInstanceDynamic* InMID) { MatInstDynamics.Add(InMID); }

	void Clear() { MatInstDynamics.Empty(); }

	int32 Num() const { return MatInstDynamics.Num(); }
};


UCLASS()
class NL_GAS_API UNLViewSkeletalMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()
	
public:
	UNLViewSkeletalMeshComponent();

	void Initialize();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditDefaultsOnly, Category = "ViewMesh")
	float InterpSpeed;

	// Only affects during BeginPlay
	// Default Horizontal FOV when Viewport's aspect ratio is 16:9
	UPROPERTY(EditDefaultsOnly, Category = "ViewMesh")
	float DefaultHFOV;

protected:
	float TargetHFOV;

	float CurrentHFOV;

	float VFOV;

	bool bDoInterp;

	float CurrentInterpSpeed;

	void InterpFOV(float DeltaTime);

	/**
	* if AspectRatio = Width  / Height : Horizontal to  Vertical
	* if AspectRatio = Height /  Width :  Vertical  to Horizontal
	*/
	double ConvertFOVByAspectRatio(double BaseFOV, double AspectRatio);

	TObjectPtr<UGameViewportClient> ViewportClient;

	UGameViewportClient* GetViewportClient();

	UPROPERTY()
	TMap<FGameplayTag, FTaggedMaterialInstanceDynamic> MaterialMap;

public:
	// Update Mesh's FOV by CurrentHFOV
	void UpdateFOV();

	void SetTargetHFOV(float InTargetHFOV, float TransientInterpSpeed = -1.f);

	virtual void SetSkeletalMesh(class USkeletalMesh* NewMesh, bool bReinitPose = true) override;

	void SetSkeletalMesh(class USkeletalMesh* NewMesh, FGameplayTag Tag, bool bReinitPose = true);
};
