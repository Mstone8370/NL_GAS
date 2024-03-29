// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * 
 */

// Input Tag
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Move);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Look);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Jump);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_CrouchHold);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_CrouchToggle);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Melee);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_Interaction);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_WeaponSlot_1);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_WeaponSlot_2);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_WeaponSlot_3);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Weapon_PrimaryAction);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Weapon_SecondaryAction);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Weapon_Reload);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Block_Ability);

// Ability Tag
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Weapon_Primary);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Weapon_Secondary);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Weapon_Reload);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_WeaponChange);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Melee);

// Weapon Tag
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Unarmed);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Pistol_Glock);

// Weapon Status
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Weapon_Holstered);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Weapon_Drawn);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Weapon_Changing);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Weapon_Reloading);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Weapon_ADS);

