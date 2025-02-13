// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_ChampionListEntry.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Controllers/LobbyPlayerController.h"
#include "Game/LobbyPlayerState.h"
#include "Game/AOSGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UUW_ChampionListEntry::NativeConstruct()
{
	Super::NativeConstruct();

	Button->OnClicked.AddDynamic(this, &UUW_ChampionListEntry::OnButtonClicked);
}

void UUW_ChampionListEntry::InitializeListEntry()
{
	MaterialRef = ChampionImage->GetDynamicMaterial();
}

void UUW_ChampionListEntry::UpdateChampionIndex(int32 Index)
{
	ChampionIndex = Index;
}

void UUW_ChampionListEntry::UpdateChampionNameText(const FName& InString)
{
	ChampionName = InString;
	ChampionNameText->SetText(FText::FromName(InString));
}

void UUW_ChampionListEntry::UpdateChampionNameColor(FLinearColor InColor)
{
	ChampionNameText->SetColorAndOpacity(InColor);
}

void UUW_ChampionListEntry::UpdateBorderImageColor(FLinearColor InColor)
{
	BorderImage->SetColorAndOpacity(InColor);
}

void UUW_ChampionListEntry::UpdateChampionImage(UTexture* InTexture)
{
	if (::IsValid(InTexture))
	{
		MaterialRef = ChampionImage->GetDynamicMaterial();
		MaterialRef->SetTextureParameterValue(FName("Texture"), InTexture);
	}
}

void UUW_ChampionListEntry::OnButtonClicked()
{
	UAOSGameInstance* GameInstance = Cast<UAOSGameInstance>(GetGameInstance());
	if (::IsValid(GameInstance) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[UUW_ChampionListEntry::OnButtonClicked] GameInstance is not valid."));
		return;
	}

	ALobbyPlayerController* PlayerController = Cast<ALobbyPlayerController>(GetOwningPlayer());
	if (!::IsValid(PlayerController))
	{
		UE_LOG(LogTemp, Error, TEXT("[UUW_ChampionListEntry::OnButtonClicked] PlayerController is not valid."));
		return;
	}

	ALobbyPlayerState* PlayerState = Cast<ALobbyPlayerState>(PlayerController->PlayerState);
	if (!::IsValid(PlayerState))
	{
		UE_LOG(LogTemp, Error, TEXT("[UUW_ChampionListEntry::OnButtonClicked] PlayerState is not valid."));
		return;
	}

	FName PlayerName = FName(*PlayerState->GetPlayerName());
	int32 PlayerIndex = PlayerState->PlayerIndex;

	PlaySelectSound();
	PlayerState->UpdateSelectedChampion_Server(ChampionIndex, ChampionName);
	PlayerController->UpdatePlayerSelection_Server(PlayerState->TeamSide, PlayerIndex, PlayerName, ChampionName, FLinearColor::Blue, true);
}

void UUW_ChampionListEntry::PlaySelectSound()
{
	// 사운드 파일 경로 생성
	int32 RandomInt = FMath::RandRange(1, 3);
	FString SoundPath = FString::Printf(TEXT("/Game/Paragon%s/Audio/Wavs/%s_DraftSelect_0%d0.%s_DraftSelect_0%d0"), *ChampionName.ToString(), *ChampionName.ToString(), RandomInt, *ChampionName.ToString(), RandomInt);

	// 사운드 로드
	USoundBase* SelectSound = Cast<USoundBase>(StaticLoadObject(USoundBase::StaticClass(), NULL, *SoundPath));

	// 사운드 로드 성공 여부 확인 및 로그 출력
	if (SelectSound)
	{
		// 사운드 재생
		UGameplayStatics::PlaySound2D(GetWorld(), SelectSound, 1.0f, 1.0f, 0.0f);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load sound: %s"), *SoundPath);
	}
}