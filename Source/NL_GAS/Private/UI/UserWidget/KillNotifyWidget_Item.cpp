// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidget/KillNotifyWidget_Item.h"

bool UKillNotifyWidget_Item::IsDisplaying() const
{
    return GetWorld()->GetTimerManager().IsTimerActive(DisplayTimerHandle);
}

void UKillNotifyWidget_Item::StartDisplay_Implementation(const FString& Name, float DisplayTime)
{
    GetWorld()->GetTimerManager().SetTimer(DisplayTimerHandle, DisplayTime, false);
}
