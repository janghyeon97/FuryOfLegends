// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/UIPlayerController.h"
#include "Plugins/MultiplaySessionSubsystem.h"
#include "Structs/SessionInfomation.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UW_ConfirmationQuit.h"
#include "UI/PasswordMenuUI.h"

AUIPlayerController::AUIPlayerController()
{

}

void AUIPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		if (::IsValid(MainMenuClass) == false)
		{
			return;
		}

		MainMenuInstance = CreateWidget<UUserWidget>(this, MainMenuClass);
		if (::IsValid(MainMenuInstance))
		{
			MainMenuInstance->AddToViewport();
			// AddToViewport()가 호출 될 때 UIWidgetInstance->NativeConstruct() 함수가 호출됨.

			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(MainMenuInstance->GetCachedWidget());
			SetInputMode(Mode);

			bShowMouseCursor = true;
		}
	}
}

void AUIPlayerController::JoinSession(FString InputPassword)
{
	if (SelectedSession == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AUIPlayerController - SelectedSession is nullptr !!"));
		return;
	}

	UMultiplaySessionSubsystem* SessionSubsystem = GetGameInstance()->GetSubsystem<UMultiplaySessionSubsystem>();
	if (!::IsValid(SessionSubsystem))
	{
		UE_LOG(LogTemp, Warning, TEXT("AUIPlayerController - SessionSubsystem is not vaild !!"));
		return;
	}

	SessionSubsystem->JoinSession(SelectedSession->GetIndex(), InputPassword);
}

void AUIPlayerController::ShowMainMenu()
{
	if (!::IsValid(MainMenuInstance))
	{
		MainMenuInstance = CreateWidget<UUserWidget>(this, MainMenuClass);
		if (::IsValid(MainMenuInstance))
		{
			MainMenuInstance->AddToViewport();
		}
	}
	else
	{
		MainMenuInstance->AddToViewport();
	}
}


void AUIPlayerController::ShowHostMenu()
{
	if (::IsValid(HostMenuInstance))
	{
		HostMenuInstance->AddToViewport();
	}
	else
	{
		HostMenuInstance = CreateWidget<UUserWidget>(this, HostMenuClass);
		if (::IsValid(HostMenuInstance))
		{
			HostMenuInstance->AddToViewport();
		}
	}
}

void AUIPlayerController::ShowJoinMenu()
{
	JoinMenuInstance = CreateWidget<UUserWidget>(this, JoinMenuClass);
	if (::IsValid(JoinMenuInstance))
	{
		JoinMenuInstance->AddToViewport();
	}
}

void AUIPlayerController::ShowLoadingScreen()
{
	if (::IsValid(LoadingScreenInstance))
	{
		LoadingScreenInstance->AddToViewport();
	}
	else
	{
		LoadingScreenInstance = CreateWidget<UUserWidget>(this, LoadingScreenClass);
		if (::IsValid(LoadingScreenInstance))
		{
			LoadingScreenInstance->AddToViewport();
		}
	}
}

void AUIPlayerController::ShowPasswordMenu()
{
	PasswordMenuInstance = CreateWidget<UPasswordMenuUI>(this, PasswordMenuClass);
	if (::IsValid(PasswordMenuInstance))
	{
		PasswordMenuInstance->AddToViewport();
	}
}

void AUIPlayerController::ShowConfirmationQuitMenu()
{
	if (::IsValid(ConfirmationQuitInstance))
	{
		ConfirmationQuitInstance->RemoveFromParent();
		ConfirmationQuitInstance = nullptr;
	}

	ConfirmationQuitInstance = CreateWidget<UUserWidget>(this, ConfirmationQuitClass);
	if (::IsValid(ConfirmationQuitInstance))
	{
		ConfirmationQuitInstance->AddToViewport();
	}
}

