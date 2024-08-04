// Fill out your copyright notice in the Description page of Project Settings.


#include "NLGameplayTags.h"

// Input Tag
UE_DEFINE_GAMEPLAY_TAG(Input_Default_Move, "Input.Default.Move");
UE_DEFINE_GAMEPLAY_TAG(Input_Default_Look, "Input.Default.Look");
UE_DEFINE_GAMEPLAY_TAG(Input_Default_Jump, "Input.Default.Jump");
UE_DEFINE_GAMEPLAY_TAG(Input_Default_CrouchHold, "Input.Default.Crouch");
UE_DEFINE_GAMEPLAY_TAG(Input_Default_CrouchToggle, "Input.Default.CrouchToggle");
UE_DEFINE_GAMEPLAY_TAG(Input_Default_Melee, "Input.Default.Melee");
UE_DEFINE_GAMEPLAY_TAG(Input_Default_Interaction, "Input.Default.Interaction");
UE_DEFINE_GAMEPLAY_TAG(Input_Default_WeaponSlot_1, "Input.Default.WeaponSlot.1");
UE_DEFINE_GAMEPLAY_TAG(Input_Default_WeaponSlot_2, "Input.Default.WeaponSlot.2");
UE_DEFINE_GAMEPLAY_TAG(Input_Default_WeaponSlot_3, "Input.Default.WeaponSlot.3");
UE_DEFINE_GAMEPLAY_TAG(Input_Default_WeaponSlot_Prev, "Input.Default.WeaponSlot.Prev");
UE_DEFINE_GAMEPLAY_TAG(Input_Default_WeaponSlot_Next, "Input.Default.WeaponSlot.Next");
UE_DEFINE_GAMEPLAY_TAG(Input_Weapon_PrimaryAction, "Input.Weapon.PrimaryAction");
UE_DEFINE_GAMEPLAY_TAG(Input_Weapon_SecondaryAction, "Input.Weapon.SecondaryAction");
UE_DEFINE_GAMEPLAY_TAG(Input_Weapon_Reload, "Input.Weapon.Reload");
UE_DEFINE_GAMEPLAY_TAG(Input_DeathCam_Respawn, "Input.DeathCam.Respawn");

UE_DEFINE_GAMEPLAY_TAG(Input_Block_Ability, "Input.Block.Ability");

// Ability Tag
UE_DEFINE_GAMEPLAY_TAG(Ability_Weapon_Primary, "Ability.Weapon.Primary");
UE_DEFINE_GAMEPLAY_TAG(Ability_Weapon_Secondary, "Ability.Weapon.Secondary");
UE_DEFINE_GAMEPLAY_TAG(Ability_Weapon_Reload, "Ability.Weapon.Reload");
UE_DEFINE_GAMEPLAY_TAG(Ability_WeaponChange_1, "Ability.WeaponChange.1");
UE_DEFINE_GAMEPLAY_TAG(Ability_WeaponChange_2, "Ability.WeaponChange.2");
UE_DEFINE_GAMEPLAY_TAG(Ability_WeaponChange_3, "Ability.WeaponChange.3");
UE_DEFINE_GAMEPLAY_TAG(Ability_WeaponChange_Prev, "Ability.WeaponChange.Prev");
UE_DEFINE_GAMEPLAY_TAG(Ability_WeaponChange_Next, "Ability.WeaponChange.Next");
UE_DEFINE_GAMEPLAY_TAG(Ability_Melee, "Ability.Melee");
UE_DEFINE_GAMEPLAY_TAG(Ability_Sprint, "Ability.Sprint");

// Ability Block Tag
UE_DEFINE_GAMEPLAY_TAG(Ability_Block_Weapon, "Ability.Block.Weapon");
UE_DEFINE_GAMEPLAY_TAG(Ability_Block_Sprint, "Ability.Block.Sprint");

// Weapon Tag
UE_DEFINE_GAMEPLAY_TAG(Weapon_Unarmed, "Weapon.Unarmed");
UE_DEFINE_GAMEPLAY_TAG(Weapon_Pistol_Glock, "Weapon.Pistol.Glock");
UE_DEFINE_GAMEPLAY_TAG(Weapon_Rifle_ACR, "Weapon.Rifle.ACR");

// Weapon Status
UE_DEFINE_GAMEPLAY_TAG(Status_Weapon_Holstered, "Status.Weapon.Holstered");
UE_DEFINE_GAMEPLAY_TAG(Status_Weapon_Drawn, "Status.Weapon.Drawn");
UE_DEFINE_GAMEPLAY_TAG(Status_Weapon_Changing, "Status.Weapon.Changing");
UE_DEFINE_GAMEPLAY_TAG(Status_Weapon_Reloading, "Status.Weapon.Reloading");
UE_DEFINE_GAMEPLAY_TAG(Status_Weapon_ADS, "Status.Weapon.ADS");

// Anim Montage Tag
UE_DEFINE_GAMEPLAY_TAG(Montage_Weapon_Draw, "Montage.Weapon.Draw");
UE_DEFINE_GAMEPLAY_TAG(Montage_Weapon_DrawFirst, "Montage.Weapon.DrawFirst");
UE_DEFINE_GAMEPLAY_TAG(Montage_Weapon_Holster, "Montage.Weapon.Holster");
UE_DEFINE_GAMEPLAY_TAG(Montage_Weapon_Primary, "Montage.Weapon.Primary");
UE_DEFINE_GAMEPLAY_TAG(Montage_Weapon_LastFire, "Montage.Weapon.LastFire");
UE_DEFINE_GAMEPLAY_TAG(Montage_Weapon_Secondary, "Montage.Weapon.Secondary");
UE_DEFINE_GAMEPLAY_TAG(Montage_Weapon_ReloadLong, "Montage.Weapon.ReloadLong");
UE_DEFINE_GAMEPLAY_TAG(Montage_Weapon_ReloadShort, "Montage.Weapon.ReloadShort");
UE_DEFINE_GAMEPLAY_TAG(Montage_Weapon_Lower, "Montage.Weapon.Lower");
UE_DEFINE_GAMEPLAY_TAG(Montage_Weapon_Raise, "Montage.Weapon.Raise");

// Event Tag
UE_DEFINE_GAMEPLAY_TAG(Event_Reload_MagOut, "Event.Reload.MagOut");
UE_DEFINE_GAMEPLAY_TAG(Event_Reload_MagIn, "Event.Reload.MagIn");
UE_DEFINE_GAMEPLAY_TAG(Event_Reload_Finished, "Event.Reload.Finished");

// FOV Tag
UE_DEFINE_GAMEPLAY_TAG(FOV_Default, "FOV.Default");
UE_DEFINE_GAMEPLAY_TAG(FOV_ADS_None, "FOV.ADS.None");
UE_DEFINE_GAMEPLAY_TAG(FOV_ADS_1x, "FOV.ADS.1x");
UE_DEFINE_GAMEPLAY_TAG(FOV_ADS_2x, "FOV.ADS.2x");
UE_DEFINE_GAMEPLAY_TAG(FOV_ADS_3x, "FOV.ADS.3x");
UE_DEFINE_GAMEPLAY_TAG(FOV_ADS_4x, "FOV.ADS.4x");
UE_DEFINE_GAMEPLAY_TAG(FOV_Movement_Slide, "FOV.Movement.Slide");

// Attribute Tag
UE_DEFINE_GAMEPLAY_TAG(Attribute_Meta_IncomingDamage, "Attribute.Meta.IncomingDamage");

// DamageType Tag
UE_DEFINE_GAMEPLAY_TAG(DamageType_Environment, "DamageType.Environment");
UE_DEFINE_GAMEPLAY_TAG(DamageType_Weapon_Rifle_ACR, "DamageType.Weapon.Rifle.ACR");
UE_DEFINE_GAMEPLAY_TAG(DamageType_Weapon_Pistol_Glock, "DamageType.Weapon.Pistol.Glock");
UE_DEFINE_GAMEPLAY_TAG(DamageType_Weapon_Sniper_TEMP, "DamageType.Weapon.Sniper.TEMP");