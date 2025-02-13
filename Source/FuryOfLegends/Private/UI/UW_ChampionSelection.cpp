// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_ChampionSelection.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UUW_ChampionSelection::NativeConstruct()
{
	Super::NativeConstruct();
}

void UUW_ChampionSelection::InitializeListEntry()
{
	MaterialRef = ChampionImage->GetDynamicMaterial();
}

void UUW_ChampionSelection::UpdateChampionNameText(const FName& InString)
{
	ChampionNameText->SetText(FText::FromName(InString));
}

void UUW_ChampionSelection::UpdateChampionNameColor(FLinearColor InColor)
{
	ChampionNameText->SetColorAndOpacity(InColor);
}

void UUW_ChampionSelection::UpdateChampionPositionText(const FName& InString)
{
	ChampionPositionText->SetText(FText::FromName(InString));
}

void UUW_ChampionSelection::UpdateChampionPositionColor(FLinearColor InColor)
{
	ChampionPositionText->SetColorAndOpacity(InColor);
}

void UUW_ChampionSelection::UpdatePlayerNameText(const FName& InString)
{
	PlayerNameText->SetText(FText::FromName(InString));
}

void UUW_ChampionSelection::UpdatePlayerNameColor(FLinearColor InColor)
{
	PlayerNameText->SetColorAndOpacity(InColor);
}

void UUW_ChampionSelection::UpdateBorderImageColor(FLinearColor InColor)
{
	BorderImage->SetColorAndOpacity(InColor);
}

void UUW_ChampionSelection::SetChampionInfoVisibility(bool bVisibility)
{
	if (bVisibility)
	{
		ChampionNameText->SetVisibility(ESlateVisibility::HitTestInvisible);
		ChampionPositionText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		ChampionNameText->SetVisibility(ESlateVisibility::Collapsed);
		ChampionPositionText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UUW_ChampionSelection::UpdateCampionImage(UTexture* InTexture)
{
	MaterialRef = ChampionImage->GetDynamicMaterial();
	MaterialRef->SetTextureParameterValue(FName("Texture"), InTexture);
}
