// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_ChampionOverview.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/StatComponent.h"
#include "Characters/AOSCharacterBase.h"
#include "Game/ArenaGameState.h"
#include "Plugins/UniqueCodeGenerator.h"
#include "Kismet/GameplayStatics.h"


void UUW_ChampionOverview::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	OwnerPlayerIndex = -1;
}

void UUW_ChampionOverview::InitializeWidget(AArenaGameState* InGameState, UStatComponent* InStatComponent, const int32 InPlayerIndex, const FName& InCharacterName, UTexture* InProfile)
{
	if (::IsValid(InGameState))
	{
		GameState = InGameState;
		GameState->OnRespawnTimeChanged.AddDynamic(this, &ThisClass::UpdateRespawnTimer);
	}

	UMaterialInstanceDynamic* DynamicMaterial = ChampionImage->GetDynamicMaterial();
	if (::IsValid(ChampionImage))
	{
		DynamicMaterial->SetTextureParameterValue(FName("Texture"), InProfile);
	}

	OwnerPlayerIndex = InPlayerIndex;
	UpdateChampionName(InCharacterName);

	if (::IsValid(InStatComponent))
	{
		StatComponent = InStatComponent;
		StatComponent->OnCurrentLevelChanged.AddDynamic(this, &ThisClass::UpdateLevelText);
	}
}


void UUW_ChampionOverview::UpdateChampionName(const FName& NewName)
{
	if (NewName.IsNone())
	{
		return;
	}

	ChampionNameText->SetText(FText::FromName(NewName));
}


void UUW_ChampionOverview::UpdateLevelText(int32 PreviousLevel, int32 NewLevel)
{
	LevelText->SetText(FText::AsNumber(NewLevel));
}


void UUW_ChampionOverview::UpdateRespawnTimer(const uint32 UniqueCode, const float RemainingTime, const float ElapsedTime)
{
	int32 DecodePlayerIndex = static_cast<int32>(UUniqueCodeGenerator::DecodeSubField2(UniqueCode));

	if (OwnerPlayerIndex == DecodePlayerIndex)
	{
		if (RemainingTime <= 0.1f)
		{
			RespawnTimeImage->SetVisibility(ESlateVisibility::Hidden);
			RespawnTimeText->SetVisibility(ESlateVisibility::Hidden);
			return;
		}

		RespawnTimeImage->SetVisibility(ESlateVisibility::Visible);
		RespawnTimeText->SetVisibility(ESlateVisibility::Visible);

		FString RemainingTimeString = FString::Printf(TEXT("%d"), FMath::CeilToInt(RemainingTime));
		RespawnTimeText->SetText(FText::FromString(RemainingTimeString));
	}
}
