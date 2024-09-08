// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/WeaponInfo.h"
#include "GameplayTagContainer.h"
#include "NLWidgetController.generated.h"

class APlayerController;
class APlayerState;
class UAbilitySystemComponent;
class UAttributeSet;
class ANLPlayerController;
class ANLPlayerState;
class UNLAbilitySystemComponent;
class UNLAttributeSet;
class UNLCharacterComponent;

USTRUCT(BlueprintType)
struct FWidgetControllerParams
{
	GENERATED_BODY()

	FWidgetControllerParams()
	{}

	FWidgetControllerParams(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS, UNLCharacterComponent* NLC)
		: PlayerController(PC)
		, PlayerState(PS)
		, AbilitySystemComponent(ASC)
		, AttributeSet(AS)
		, NLCharacterComponent(NLC)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerState> PlayerState = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAttributeSet> AttributeSet = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UNLCharacterComponent> NLCharacterComponent = nullptr;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerStatUpdatedSignature, const APlayerState*, Player, const FGameplayTag&, StatTag, int32, Value);

/**
 * 
 */
UCLASS()
class NL_GAS_API UNLWidgetController : public UObject
{
	GENERATED_BODY()
	
public:
	void Initialize(FWidgetControllerParams Params);

	virtual void BindEvents();

	UFUNCTION(BlueprintCallable)
	virtual void BroadcastInitialValues();

	ANLPlayerController* GetNLPC();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	ANLPlayerState* GetNLPS();
	UNLAbilitySystemComponent* GetNLASC();
	UNLAttributeSet* GetNLAS();
	UNLCharacterComponent* GetNLC();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UUITaggedWeaponInfo> UIWeaponData;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	const FUIWeaponInfo FindUIWeaponInfoByTag(const FGameplayTag WeaponTag) const;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<APlayerController> PlayerController;
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<APlayerState> PlayerState;
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<ANLPlayerController> NLPlayerController;
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<ANLPlayerState> NLPlayerState;
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UNLAbilitySystemComponent> NLAbilitySystemComponent;
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UNLAttributeSet> NLAttributeSet;
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UNLCharacterComponent> NLCharacterComponent;
};
