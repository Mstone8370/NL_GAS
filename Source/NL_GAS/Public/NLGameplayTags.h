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
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_WeaponSlot_Prev);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Default_WeaponSlot_Next);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Weapon_PrimaryAction);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Weapon_SecondaryAction);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Weapon_Reload);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_DeathCam_Respawn);

UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Block_Ability);

// Ability Tag
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Weapon_Primary);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Weapon_Secondary);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Weapon_Reload);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_WeaponChange_1);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_WeaponChange_2);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_WeaponChange_3);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_WeaponChange_Prev);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_WeaponChange_Next);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Melee);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Sprint);

// Ability Block Tag
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Block_Weapon);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Block_Sprint);

// Weapon Tag
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Unarmed);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Pistol_Glock);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Rifle_ACR);

// Weapon Status
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Weapon_Holstered);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Weapon_Drawn);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Weapon_Changing);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Weapon_Reloading);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Weapon_ADS);

// Anim Montage Tag
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Montage_Weapon_Draw);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Montage_Weapon_DrawFirst);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Montage_Weapon_Holster);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Montage_Weapon_Primary);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Montage_Weapon_LastFire);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Montage_Weapon_Secondary);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Montage_Weapon_ReloadLong);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Montage_Weapon_ReloadShort);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Montage_Weapon_Lower);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Montage_Weapon_Raise);

// Event Tag
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Reload_MagOut);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Reload_MagIn);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Reload_Finished);

// FOV Tag
UE_DECLARE_GAMEPLAY_TAG_EXTERN(FOV_Default);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(FOV_ADS_None);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(FOV_ADS_1x);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(FOV_ADS_2x);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(FOV_ADS_3x);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(FOV_ADS_4x);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(FOV_Movement_Slide);

// Attribute Tag
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Meta_IncomingDamage);

// DamageType Tag
UE_DECLARE_GAMEPLAY_TAG_EXTERN(DamageType_Environment);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(DamageType_Weapon_Rifle_ACR);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(DamageType_Weapon_Pistol_Glock);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(DamageType_Weapon_Sniper_TEMP);

// Particle Tag
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Particle_Weapon_Impact_Default);

// Projectile Tag
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Projectile_Bullet_Default);
