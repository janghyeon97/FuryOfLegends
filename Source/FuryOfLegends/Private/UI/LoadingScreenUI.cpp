// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LoadingScreenUI.h"
#include "Plugins/MultiplaySessionSubsystem.h"

void ULoadingScreenUI::NativeConstruct()
{
    Super::NativeConstruct(); 

    UGameInstance* GameInstance = GetGameInstance();
    if (!::IsValid(GameInstance))
    {
        UE_LOG(LogTemp, Error, TEXT("ULoadingScreenUI::NativeConstruct - Invalid GameInstance."));
        return;
    }

    UMultiplaySessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiplaySessionSubsystem>();
    if (!::IsValid(SessionSubsystem))
    {
        UE_LOG(LogTemp, Error, TEXT("ULoadingScreenUI::NativeConstruct - Invalid SessionSubsystem."));
        return;
    }

    SessionSubsystem->OnCreateSessionCompleteEvent.AddDynamic(this, &ULoadingScreenUI::RemoveLoadingScreen);
}


void ULoadingScreenUI::RemoveLoadingScreen(bool Successful)
{
	if(Successful) RemoveFromParent();
}
