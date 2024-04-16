// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidget/NLUserWidget.h"

void UNLUserWidget::SetWidgetController(UNLWidgetController* InWidgetController)
{
    WidgetController = InWidgetController;

    OnWidgetControllerSet();
}
