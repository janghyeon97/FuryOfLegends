// Fill out your copyright notice in the Description page of Project Settings.

#include "Controllers/LobbyPlayerController.h"
#include "Game/AOSGameInstance.h"
#include "Game/LobbyPlayerState.h"
#include "Game/LobbyGameState.h"
#include "Game/LobbyGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "UI/LobbyUI.h"
#include "UI/UW_ChatWindow.h"
#include "UI/UW_Button.h"
#include "UI/ChampionSelectionUI.h"
#include "UI/UW_ChampionListEntry.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

ALobbyPlayerController::ALobbyPlayerController()
{
	ChatWindow = nullptr;
	bIsHostPlayer = false;
}

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] World is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	AOSGameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (!AOSGameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] AOSGameInstance is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	LobbyGameState = Cast<ALobbyGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (!LobbyGameState)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] LobbyGameState is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (HasAuthority())
	{
		LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		if (!LobbyGameMode)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] LobbyGameMode is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
			return;
		}

		LobbyGameMode->OnSelectionTimerChanged.AddDynamic(this, &ThisClass::UpdateBanPickTime_Client);
	}

	if (IsLocalPlayerController())
	{
		if (!LobbyUIClass)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] LobbyUIClass is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
			return;
		}

		LobbyUIInstance = CreateWidget<ULobbyUI>(this, LobbyUIClass);
		if (!LobbyUIInstance)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] Failed to create LobbyUIInstance."), ANSI_TO_TCHAR(__FUNCTION__));
			return;
		}

		LobbyUIInstance->AddToViewport();
		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(LobbyUIInstance->GetCachedWidget());
		SetInputMode(Mode);

		bShowMouseCursor = true;
		UE_LOG(LogTemp, Log, TEXT("[%s] LobbyUIInstance successfully created."), ANSI_TO_TCHAR(__FUNCTION__));
	}
}


void ALobbyPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bChangedDraftTime)
	{
		if (DraftTime <= 0)
		{
			bChangedDraftTime = false;
			return;
		}

		DraftTime = FMath::FInterpTo(DraftTime, CurrentDraftTime, DeltaSeconds, 15.f);
		if (ChampionSelectUIInstance)
		{
			ChampionSelectUIInstance->OnBanPickTimeChanged(DraftTime, MaxDraftTime);
		}
	}
}

void ALobbyPlayerController::BindChatWindow(UUW_ChatWindow* Widget)
{
	ChatWindow = Widget;
}

void ALobbyPlayerController::ShowLobbyUI()
{
	if (LobbyUIInstance)
	{
		LobbyUIInstance->AddToViewport();
	}
	else
	{
		LobbyUIInstance = CreateWidget<ULobbyUI>(this, LobbyUIClass);
		if (LobbyUIInstance)
		{
			LobbyUIInstance->AddToViewport();
		}
	}
}


void ALobbyPlayerController::ShowLoadingScreen_Client_Implementation()
{
	if (LoadingScreenInstance)
	{
		LoadingScreenInstance->AddToViewport();
	}
	else
	{
		LoadingScreenInstance = CreateWidget<UUserWidget>(this, LoadingScreenClass);
		if (LoadingScreenInstance)
		{
			LoadingScreenInstance->AddToViewport();
		}
	}
}

void ALobbyPlayerController::ShowChampionSelectUI_Server_Implementation()
{
	if (!LobbyGameState)
	{
		LoadGameState();
		return;
	}

	const TArray<TObjectPtr<APlayerState>>& AllPlayers = LobbyGameState->PlayerArray;
	for (auto& CurrentPlayer : AllPlayers)
	{
		ALobbyPlayerController* LobbyPlayerController = Cast<ALobbyPlayerController>(CurrentPlayer->GetPlayerController());
		if (::IsValid(LobbyPlayerController))
		{
			LobbyPlayerController->ShowChampionSelectUI_Client();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[ALobbyPlayerController::ShowChampionSelectUI_Server] Invalid LobbyPlayerController for player %s."), *CurrentPlayer->GetPlayerName());
		}
	}

	if (::IsValid(LobbyGameMode))
	{
		LobbyGameMode->MatchState = EMatchState::Picking;
		LobbyGameMode->StartBanPick();
	}
	else
	{
		LoadGameMode();
		if (::IsValid(LobbyGameMode))
		{
			LobbyGameMode->MatchState = EMatchState::Picking;
			LobbyGameMode->StartBanPick();
		}
	}
}

void ALobbyPlayerController::ShowChampionSelectUI_Client_Implementation()
{
	if (::IsValid(ChampionSelectUIInstance))
	{
		ChampionSelectUIInstance->AddToViewport();
	}
	else
	{
		if (ChampionSelectUIClass == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("[ALobbyPlayerController::ShowChampionSelectUI_Client] ChampionSelectUIClass is not set."));
			return;
		}

		ChampionSelectUIInstance = CreateWidget<UChampionSelectionUI>(this, ChampionSelectUIClass);
		if (::IsValid(ChampionSelectUIInstance))
		{
			ChampionSelectUIInstance->AddToViewport();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[ALobbyPlayerController::ShowChampionSelectUI_Client] Failed to create ChampionSelectUIInstance."));
		}
	}
}

void ALobbyPlayerController::SetHostPlayer_Client_Implementation(bool bIsHost)
{
	bIsHostPlayer = bIsHost;
	if (LobbyUIInstance)
	{
		UUW_Button* GameStartButton = LobbyUIInstance->GetGameStartButton();
		if (GameStartButton)
		{
			GameStartButton->SetIsEnabled(bIsHost);
		}
	}
}

void ALobbyPlayerController::UpdateChatLog_Server_Implementation(const FString& Message, AController* EventInstigator)
{
	if (::IsValid(LobbyGameMode) == false)
	{
		LoadGameMode();
		return;
	}

	if (::IsValid(LobbyGameState) == false)
	{
		LoadGameState();
		return;
	}

	ALobbyPlayerState* SenderPlayerState = EventInstigator->GetPlayerState<ALobbyPlayerState>();
	if (!SenderPlayerState)
	{
		return;
	}

	FString SenderName = SenderPlayerState->GetPlayerName();

	TArray<TObjectPtr<ALobbyPlayerState>> Players;
	if (LobbyGameMode->MatchState == EMatchState::Waiting)
	{
		for (APlayerState* CurrentPlayer  : LobbyGameState->PlayerArray)
		{
			if (ALobbyPlayerState* NewPlayerState = Cast<ALobbyPlayerState>(CurrentPlayer))
			{
				Players.Add(NewPlayerState);
			}
		}
	}
	else if (LobbyGameMode->MatchState == EMatchState::Picking)
	{
		if (SenderPlayerState->TeamSide == ETeamSide::Blue)
		{
			Players = LobbyGameState->BlueTeamPlayers;
		}
		else if (SenderPlayerState->TeamSide == ETeamSide::Red)
		{
			Players = LobbyGameState->RedTeamPlayers;
		}
	}

	for (APlayerState* CurrentPlayer  : Players)
	{
		ALobbyPlayerController* LobbyPlayerController = Cast<ALobbyPlayerController>(CurrentPlayer->GetPlayerController());
		if (LobbyPlayerController)
		{
			FString Result = (EventInstigator == LobbyPlayerController) ? FString::Printf(TEXT("<Yellow>%s</>: %s"), *SenderName, *Message) : FString::Printf(TEXT("<White>%s</>: %s"), *SenderName, *Message);
			LobbyPlayerController->UpdateChatLog_Client(Result);
		}
	}
}

void ALobbyPlayerController::UpdateChatLog_Client_Implementation(const FString& Text)
{
	if (::IsValid(ChatWindow) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Client] ALobbyPlayerController::UpdateChatLog_Client ChatWindow is not valid!"));
		return;
	}


	UE_LOG(LogTemp, Log, TEXT("[Client] %s received message: %s"), *PlayerState->GetPlayerName(), *Text);
	ChatWindow->UpdateMessageLog(Text);
}

void ALobbyPlayerController::UpdatePlayerSelection_Server_Implementation(ETeamSide Team, int32 PlayerIndex, const FName& InPlayerName, const FName& InChampionName, FLinearColor Color, bool bShowChampionDetails)
{
	if (::IsValid(LobbyGameMode) == false)
	{
		LoadGameMode();
		return;
	}

	if (::IsValid(LobbyGameState) == false)
	{
		LoadGameState();
		return;
	}

	if (Team != ETeamSide::Blue && Team != ETeamSide::Red)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid team: %d. Only Blue or Red are allowed."), ANSI_TO_TCHAR(__FUNCTION__), static_cast<int32>(Team));
		return;
	}


	const TArray<TObjectPtr<APlayerState>>& AllPlayers = LobbyGameState->PlayerArray;
	for (auto& CurrentPlayer : AllPlayers)
	{
		ALobbyPlayerController* LobbyPlayerController = Cast<ALobbyPlayerController>(CurrentPlayer->GetPlayerController());
		if (LobbyPlayerController)
		{
			LobbyPlayerController->UpdatePlayerSelection_Client(Team, PlayerIndex, InPlayerName, InChampionName, Color, bShowChampionDetails);
		}
	}
}

void ALobbyPlayerController::UpdatePlayerSelection_Client_Implementation(ETeamSide Team, int32 PlayerIndex, const FName& InPlayerName, const FName& InChampionName, FLinearColor Color, bool bShowChampionDetails)
{
	if (::IsValid(AOSGameInstance) == false)
	{
		LoadGameInstance();
		return;
	}

	if (::IsValid(ChampionSelectUIInstance) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] %s Player's ChampionSelectUIInstance is not valid."), ANSI_TO_TCHAR(__FUNCTION__), *PlayerState->GetPlayerName());
		return;
	}

	const FCharacterAttributesRow* ChampionsListRow = AOSGameInstance->GetChampionListTableRow(InChampionName);
	if (!ChampionsListRow)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] %s Failed to find ChampionsListRow for PlayerName: %s"), ANSI_TO_TCHAR(__FUNCTION__), *PlayerState->GetPlayerName(), *InChampionName.ToString());
		return;
	}

	UTexture* ChampionImageTexture = ChampionsListRow->ChampionImage;
	FName ChampionPositionString = *StaticEnum<ELaneType>()->GetNameStringByValue(static_cast<int64>(ChampionsListRow->Position));

	ChampionSelectUIInstance->UpdatePlayerSelection(Team, PlayerIndex, InPlayerName, ChampionImageTexture, InChampionName, ChampionPositionString, Color, bShowChampionDetails);
}

void ALobbyPlayerController::UpdateBanPickTime_Client_Implementation(float InCurrentDraftTime, float InMaxDraftTime)
{
	if (::IsValid(ChampionSelectUIInstance) == false)
	{
		return;
	}

	if (InCurrentDraftTime >= InMaxDraftTime)
	{
		DraftTime = InCurrentDraftTime;
	}
	else
	{
		bChangedDraftTime = true;
	}

	CurrentDraftTime = InCurrentDraftTime;
	MaxDraftTime = InMaxDraftTime;
}



void ALobbyPlayerController::LoadGameInstance()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	AOSGameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(World));
}

void ALobbyPlayerController::LoadGameMode()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	LobbyGameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(World));
}

void ALobbyPlayerController::LoadGameState()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	LobbyGameState = Cast<ALobbyGameState>(UGameplayStatics::GetGameState(World));
}

void ALobbyPlayerController::LoadPlayerState()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;

	}

	if (::IsValid(PlayerState))
	{
		LobbyPlayerState = Cast<ALobbyPlayerState>(PlayerState);
	}
	else
	{
		LobbyPlayerState = Cast<ALobbyPlayerState>(UGameplayStatics::GetPlayerState(World, 0));
	}
}