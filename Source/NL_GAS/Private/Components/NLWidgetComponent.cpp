// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/NLWidgetComponent.h"

void UNLWidgetComponent::InitWidget()
{
    Super::InitWidget();

    bIsWidgetInitialized = true;

    OnWidgetInitialized.Broadcast();
}
