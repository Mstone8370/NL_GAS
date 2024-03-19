// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NLCharacterBase.h"

ANLCharacterBase::ANLCharacterBase()
{
 	PrimaryActorTick.bCanEverTick = false;
}

void ANLCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}
