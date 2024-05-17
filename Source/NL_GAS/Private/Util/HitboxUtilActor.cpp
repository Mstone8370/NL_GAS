// Fill out your copyright notice in the Description page of Project Settings.


#include "Util/HitboxUtilActor.h"

#include "Components/HitboxComponent.h"

// Sets default values
AHitboxUtilActor::AHitboxUtilActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    HitboxComp = CreateDefaultSubobject<UHitboxComponent>(FName("Hitbox"));
    SetRootComponent(HitboxComp);
    HitboxComp->SetLineThickness(1.f);
}
