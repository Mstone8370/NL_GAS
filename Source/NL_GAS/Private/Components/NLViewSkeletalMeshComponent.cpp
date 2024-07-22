// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/NLViewSkeletalMeshComponent.h"

#include "Engine/SkinnedAssetCommon.h"
#include "Kismet/KismetMathLibrary.h"

UNLViewSkeletalMeshComponent::UNLViewSkeletalMeshComponent()
    : InterpSpeed(10.f)
    , DefaultHFOV(80.f)
{}

void UNLViewSkeletalMeshComponent::Initialize()
{
    // Create dyamic materials
    for (uint8 i = 0; i < GetNumMaterials(); i++)
    {
        UMaterialInstanceDynamic* MatInstDynamic = CreateAndSetMaterialInstanceDynamic(i);
        MatInstDynamic->SetScalarParameterValue(FName("FOV"), DefaultHFOV);
    }

    TargetHFOV = DefaultHFOV;
    CurrentHFOV = DefaultHFOV;

    UpdateFOV();
}

void UNLViewSkeletalMeshComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    InterpFOV(DeltaTime);
}

void UNLViewSkeletalMeshComponent::InterpFOV(float DeltaTime)
{
    if (!bDoInterp)
    {
        return;
    }

    CurrentHFOV = FMath::FInterpTo(CurrentHFOV, TargetHFOV, DeltaTime, CurrentInterpSpeed);
    if (FMath::IsNearlyEqual(CurrentHFOV, TargetHFOV))
    {
        CurrentHFOV = TargetHFOV;
        bDoInterp = false;
        CurrentInterpSpeed = InterpSpeed;
    }
    UpdateFOV();
}

double UNLViewSkeletalMeshComponent::ConvertFOVByAspectRatio(double BaseFOV, double AspectRatio)
{
    return UKismetMathLibrary::DegAtan(UKismetMathLibrary::DegTan(BaseFOV / 2) * AspectRatio) * 2;
}

void UNLViewSkeletalMeshComponent::UpdateFOV()
{
    float FinalHFOV = CurrentHFOV;

    if (GetViewportClient())
    {
        VFOV = ConvertFOVByAspectRatio(CurrentHFOV, 9 / 16.f);

        FVector2D ViewportSize;
        GetViewportClient()->GetViewportSize(ViewportSize);
        FinalHFOV = ConvertFOVByAspectRatio(VFOV, ViewportSize.X / ViewportSize.Y);
    }

    // Set FOV value
    for (UMaterialInterface* MaterialInstance : GetMaterials())
    {
        if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(MaterialInstance))
        {
            MID->SetScalarParameterValue(FName("FOV"), FinalHFOV);
        }
    }
}

UGameViewportClient* UNLViewSkeletalMeshComponent::GetViewportClient()
{
    if (!ViewportClient)
    {
        ViewportClient = GEngine ? GEngine->GameViewport : nullptr;
    }
    return ViewportClient;
}

void UNLViewSkeletalMeshComponent::SetTargetHFOV(float InTargetHFOV, float TransientInterpSpeed)
{
    if (!FMath::IsNearlyEqual(TargetHFOV, InTargetHFOV))
    {
        TargetHFOV = InTargetHFOV;
        bDoInterp = true;
        CurrentInterpSpeed = TransientInterpSpeed > 0.f ? TransientInterpSpeed : InterpSpeed;
    }
}

void UNLViewSkeletalMeshComponent::SetSkeletalMesh(USkeletalMesh* NewMesh, bool bReinitPose)
{
    Super::SetSkeletalMesh(NewMesh, bReinitPose);
}

void UNLViewSkeletalMeshComponent::SetSkeletalMesh(USkeletalMesh* NewMesh, FGameplayTag Tag, bool bReinitPose)
{
    Super::SetSkeletalMesh(NewMesh, bReinitPose);

    if (Tag.IsValid())
    {
        /**
        * 메시만 변경하면 이전에 만든 다른 메시의 다이나믹 머티리얼이 존재해서
        * 그걸 그대로 사용하므로 메시를 변경할때마다 머티리얼을 새로 지정해줌.
        * 머티리얼 인스턴스를 저장해둬서 메시를 변경할때마다 새로 생성하지 않고 이전에 쓰던걸 다시 사용함.
        */
        if (MaterialMap.Contains(Tag))
        {
            FTaggedMaterialInstanceDynamic& TaggedMID = MaterialMap[Tag];
            for (uint8 i = 0; i < GetNumMaterials(); i++)
            {
                UMaterialInstanceDynamic* MID = TaggedMID.MatInstDynamics[i];
                if (!MID)
                {
                    // Could happen?
                    UE_LOG(LogTemp, Error, TEXT("Material Instance Dynamic [%d] of weapon [%s] is not valid."), i, *Tag.GetTagName().ToString());

                    // Create again
                    FSkeletalMaterial& Mat = NewMesh->GetMaterials()[i];
                    SetMaterial(i, Mat.MaterialInterface);

                    MID = CreateAndSetMaterialInstanceDynamic(i);
                }

                SetMaterial(i, MID);
            }
        }
        else
        {
            FTaggedMaterialInstanceDynamic TaggedMID;
            for (uint8 i = 0; i < GetNumMaterials(); i++)
            {
                // Set WeaponMesh's material
                FSkeletalMaterial& Mat = NewMesh->GetMaterials()[i];
                SetMaterial(i, Mat.MaterialInterface);

                // Create Weapon Material Instance Dynamic
                UMaterialInstanceDynamic* MatInstDynamic = CreateAndSetMaterialInstanceDynamic(i);
                SetMaterial(i, MatInstDynamic);
                TaggedMID.Add(MatInstDynamic);
            }
            MaterialMap.Add(Tag, TaggedMID);
        }

        UpdateFOV();
    }
}

