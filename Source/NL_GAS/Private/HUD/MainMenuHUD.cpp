// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/MainMenuHUD.h"

#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

void AMainMenuHUD::TEST()
{
    if (IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get())
    {
        IOnlineSession* SessionInterface = OnlineSubsystem->GetSessionInterface().Get();

        FOnlineSessionSettings Settings;

        SessionInterface->CreateSession(0, FName("TEST"), Settings);
    }
}
