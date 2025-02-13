// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LobbyUI.h"
#include "UI/UW_Button.h"
#include "UI/UW_EditableText.h"
#include "UI/UW_LobbyPlayerInfomation.h"
#include "Kismet/GameplayStatics.h"
#include "Game/LobbyGameState.h"
#include "Game/LobbyPlayerState.h"
#include "Controllers/LobbyPlayerController.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/StackBox.h"
#include "Components/Button.h"


void ULobbyUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	TArray<UWidget*> widgets;
	
	widgets = BlueTeamStackBox->GetAllChildren();
	for (auto& widget : widgets)
	{
		UWidgetSwitcher* PlayerInfo = Cast<UWidgetSwitcher>(widget);
		if (::IsValid(PlayerInfo))
		{
			BlueTeamSwitcher.AddUnique(PlayerInfo);
		}
	}

	widgets = RedTeamStackBox->GetAllChildren();
	for (auto& widget : widgets)
	{
		UWidgetSwitcher* PlayerInfo = Cast<UWidgetSwitcher>(widget);
		if (::IsValid(PlayerInfo))
		{
			RedTeamSwitcher.AddUnique(PlayerInfo);
		}
	}
}

void ULobbyUI::NativeConstruct()
{
	Super::NativeConstruct();

	UWorld* WorldContext = GetWorld();
	if (::IsValid(WorldContext) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] WorldContext is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	LobbyGameState = Cast<ALobbyGameState>(UGameplayStatics::GetGameState(WorldContext));
	if (::IsValid(LobbyGameState))
	{
		LobbyGameState->OnConnectedPlayerReplicated.AddUObject(this, &ThisClass::UpdateLobby);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] LobbyGameState is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
	}

	LobbyPlayerController = Cast<ALobbyPlayerController>(UGameplayStatics::GetPlayerController(WorldContext, 0));
	if (LobbyPlayerController.IsValid())
	{
		GameStartButton->Button->OnClicked.AddDynamic(this, &ULobbyUI::OnGameStartButtonClicked);

		// 호스트 플레이어인지 확인
		if (LobbyPlayerController->GetIsHostPlayer())
		{
			GameStartButton->SetIsEnabled(true);
		}
		else
		{
			GameStartButton->SetIsEnabled(false);
		}
	}
	else
	{
		GameStartButton->SetIsEnabled(false); // 컨트롤러가 없으면 버튼 비활성화
	}

	// 로비 업데이트 호출
	UpdateLobby();
}


void ULobbyUI::InitializeSwitcher()
{
	// 블루팀 스위처 초기화
	for (auto& switcher : BlueTeamSwitcher)
	{
		if (::IsValid(switcher))
		{
			switcher->SetActiveWidgetIndex(0);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid BlueTeamSwitcher detected."), ANSI_TO_TCHAR(__FUNCTION__));
		}
	}

	// 레드팀 스위처 초기화
	for (auto& switcher : RedTeamSwitcher)
	{
		if (::IsValid(switcher))
		{
			switcher->SetActiveWidgetIndex(0);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid RedTeamSwitcher detected."), ANSI_TO_TCHAR(__FUNCTION__));
		}
	}
}


void ULobbyUI::UpdateLobby()
{
	// 팀 스위처 초기화
	InitializeSwitcher();

	// 로비 게임 상태에서 플레이어 배열 가져오기
	TArray<TObjectPtr<APlayerState>> Players = LobbyGameState->PlayerArray;

	if (Players.Num() == 0)
	{
		return;  // 플레이어가 없는 경우 바로 리턴
	}

	// 각 플레이어 상태를 확인하여 팀별로 위젯 업데이트
	for (auto& Player : Players)
	{
		ALobbyPlayerState* LobbyPlayerState = Cast<ALobbyPlayerState>(Player);
		if (!::IsValid(LobbyPlayerState))
		{
			continue; 
		}

		// 팀에 따라 인덱스 계산
		const bool bIsBlueTeam = (LobbyPlayerState->TeamSide == ETeamSide::Blue);
		int32 PlayerIndex = bIsBlueTeam ? LobbyPlayerState->PlayerIndex : LobbyPlayerState->PlayerIndex - 5;

		// 팀 스위처 배열 설정
		TArray<UWidgetSwitcher*>& TeamSwitcher = bIsBlueTeam ? BlueTeamSwitcher : RedTeamSwitcher;

		// 유효한 인덱스 확인
		if (!TeamSwitcher.IsValidIndex(PlayerIndex))
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] Invalid index %d for team %s."), ANSI_TO_TCHAR(__FUNCTION__), PlayerIndex, bIsBlueTeam ? TEXT("Blue") : TEXT("Red"));
			continue;
		}

		// 자식 개수 확인
		if (TeamSwitcher[PlayerIndex]->GetChildrenCount() <= 1)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] Insufficient children for team %s at index %d."), ANSI_TO_TCHAR(__FUNCTION__), bIsBlueTeam ? TEXT("Blue") : TEXT("Red"), PlayerIndex);
			continue;
		}


		UUW_LobbyPlayerInfomation* Widget = Cast<UUW_LobbyPlayerInfomation>(TeamSwitcher[PlayerIndex]->GetChildAt(1));
		if (::IsValid(Widget) == false)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] Invalid widget at index %d for team %s."), ANSI_TO_TCHAR(__FUNCTION__), PlayerIndex, bIsBlueTeam ? TEXT("Blue") : TEXT("Red"));
			continue;  
		}

		// 위젯 업데이트
		Widget->UpdatePlayerNameText(Player->GetPlayerName());
		TeamSwitcher[PlayerIndex]->SetActiveWidgetIndex(1);
	}
}


void ULobbyUI::OnGameStartButtonClicked()
{	
	LobbyPlayerController->ShowChampionSelectUI_Server();
}