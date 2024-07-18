// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidget/KillNotifyWidget_Slot.h"

#include "UI/UserWidget/KillNotifyWidget_Item.h"

void UKillNotifyWidget_Slot::NativeConstruct()
{
    Super::NativeConstruct();

    Items.Empty();
    
    check(ItemWidgetClass);

    for (int32 i = 0; i < SlotNum; i++)
    {
        UKillNotifyWidget_Item* Item = CreateWidget<UKillNotifyWidget_Item>(this, ItemWidgetClass);
        Items.Add(Item);
    }

    OnInitializedItems();
}

void UKillNotifyWidget_Slot::DisplayItem(const FString& Name)
{
    for (UKillNotifyWidget_Item* Item : Items)
    {
        if (!Item->IsDisplaying())
        {
            Item->StartDisplay(Name, ItemDisplayTime);
            return;
        }
    }
}
