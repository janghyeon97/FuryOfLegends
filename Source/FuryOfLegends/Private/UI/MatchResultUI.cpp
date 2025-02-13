// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MatchResultUI.h"
#include "UI/UW_Button.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Game/ArenaGameMode.h"
#include "Kismet/GameplayStatics.h"

void UMatchResultUI::NativeConstruct()
{
	Super::NativeConstruct();

	ContinueButton->Button->OnClicked.AddDynamic(this, &ThisClass::OnButtonClicked);
}

void UMatchResultUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();

}

void UMatchResultUI::UpdateResultText(const FString& NewText)
{
	if (NewText.IsEmpty())
	{
		return;
	}

	if (::IsValid(MatchResultText))
	{
		MatchResultText->SetText(FText::FromString(NewText));
	}
}

void UMatchResultUI::OnButtonClicked()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to get World context"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
	if (::IsValid(PlayerController) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to get PlayerController from World context"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	PlayerController->ClientTravel(TEXT("Title"), ETravelType::TRAVEL_Absolute);
}
