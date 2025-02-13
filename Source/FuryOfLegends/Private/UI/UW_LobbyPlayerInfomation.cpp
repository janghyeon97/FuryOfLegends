// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_LobbyPlayerInfomation.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UUW_LobbyPlayerInfomation::NativeConstruct()
{
	Super::NativeConstruct();


}

void UUW_LobbyPlayerInfomation::UpdatePlayerNameText(FString InName)
{
	PlayerNameText->SetText(FText::FromString(InName));
}

void UUW_LobbyPlayerInfomation::UpdatePlayerNameColor(FLinearColor InColor)
{
	PlayerNameText->SetColorAndOpacity(InColor);
}
