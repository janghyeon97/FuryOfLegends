// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/LoadingPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "UI/LoadingScreenUI.h"

void ALoadingPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		if (!LoadingScreenClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] LoadingScreenClass is not set."), ANSI_TO_TCHAR(__FUNCTION__));
			return;
		}

		LoadingScreenWidget = CreateWidget<UUserWidget>(this, LoadingScreenClass);
		if (::IsValid(LoadingScreenWidget) == false)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] Failed to create LoadingScreenWidget."), ANSI_TO_TCHAR(__FUNCTION__));
			return;
		}

		LoadingScreenWidget->AddToViewport(9999);
	}


	AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this);
	if (::IsValid(GameMode) == false)
	{
		return;
	}

	FString NextLevelString = UGameplayStatics::ParseOption(GameMode->OptionsString, FString(TEXT("NextLevel")));
	if (NextLevelString.IsEmpty())
	{
		NextLevelString = TEXT("Title");
	}

	UGameplayStatics::OpenLevel(GameMode, *NextLevelString, false);
}