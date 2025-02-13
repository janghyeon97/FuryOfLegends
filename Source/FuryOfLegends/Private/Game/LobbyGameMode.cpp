// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/LobbyGameMode.h"
#include "Game/AOSGameInstance.h"
#include "Game/LobbyGameState.h"
#include "Game/LobbyPlayerState.h"
#include "Game/PlayerStateSave.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Plugins/MultiplaySessionSubsystem.h"
#include "Controllers/LobbyPlayerController.h"
#include "Characters/AuroraCharacter.h"
#include "Kismet/GameplayStatics.h"

ALobbyGameMode::ALobbyGameMode()
{
	bIsFirstConnectedPlayer = true;

	NumberOfConnectedPlayers = 0;
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ALobbyGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(BanPickTimer);
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ALobbyPlayerController* NewPlayerController = Cast<ALobbyPlayerController>(NewPlayer);
	if (::IsValid(NewPlayerController) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] NewPlayer is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	RegisterNewPlayer(NewPlayer);

	ALobbyPlayerState* LobbyPlayerState = NewPlayerController->GetPlayerState<ALobbyPlayerState>();
	if (::IsValid(LobbyPlayerState) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] LobbyPlayerState is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	ALobbyGameState* LobbyGameState = Cast<ALobbyGameState>(GameState);
	if (::IsValid(LobbyGameState) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] LobbyGameState is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (bIsFirstConnectedPlayer)
	{
		bIsFirstConnectedPlayer = false;
		NewPlayerController->SetHostPlayer_Client(true);
	}

	if (BlueTeamPlayerControllers.Num() <= RedTeamPlayerControllers.Num())
	{
		LobbyPlayerState->TeamSide = ETeamSide::Blue;
		LobbyPlayerState->PlayerIndex = BlueTeamPlayerControllers.Num();
		LobbyPlayerState->PlayerUniqueID = LobbyPlayerState->GetPlayerUniqueId();

		BlueTeamPlayerControllers.AddUnique(NewPlayerController);
		LobbyGameState->BlueTeamPlayers.Add(LobbyPlayerState);
		UE_LOG(LogTemp, Warning, TEXT("[Server] Blue Side New Player Login : %s"), *LobbyPlayerState->GetPlayerName());
	}
	else
	{
		LobbyPlayerState->TeamSide = ETeamSide::Red;
		LobbyPlayerState->PlayerIndex = RedTeamPlayerControllers.Num() + 5;
		LobbyPlayerState->PlayerUniqueID = LobbyPlayerState->GetPlayerUniqueId();

		RedTeamPlayerControllers.AddUnique(NewPlayerController);
		LobbyGameState->RedTeamPlayers.Add(LobbyPlayerState);
		UE_LOG(LogTemp, Warning, TEXT("[Server] Red Side New Player Login : %s"), *LobbyPlayerState->GetPlayerName());
	}

	NumberOfConnectedPlayers++;
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ALobbyPlayerController* ExitingPlayerController = Cast<ALobbyPlayerController>(Exiting);
	if (::IsValid(ExitingPlayerController) == false)
	{
		return;
	}

	if (INDEX_NONE != BlueTeamPlayerControllers.Find(ExitingPlayerController) || INDEX_NONE != RedTeamPlayerControllers.Find(ExitingPlayerController))
	{
		BlueTeamPlayerControllers.Remove(ExitingPlayerController);
		RedTeamPlayerControllers.Remove(ExitingPlayerController);
	}

	ALobbyPlayerState* LobbyPlayerState = ExitingPlayerController->GetPlayerState<ALobbyPlayerState>();
	if (::IsValid(LobbyPlayerState))
	{
		ALobbyGameState* LobbyGameState = Cast<ALobbyGameState>(GameState);
		if (::IsValid(LobbyGameState))
		{
			if (INDEX_NONE != LobbyGameState->BlueTeamPlayers.Find(LobbyPlayerState) || INDEX_NONE != LobbyGameState->RedTeamPlayers.Find(LobbyPlayerState))
			{
				LobbyGameState->BlueTeamPlayers.Remove(LobbyPlayerState);
				LobbyGameState->BlueTeamPlayers.Remove(LobbyPlayerState);
			}
		}
	}


	NumberOfConnectedPlayers--;
}

void ALobbyGameMode::RegisterNewPlayer(APlayerController* NewPlayer)
{
	UWorld* World = GetWorld();
	if (::IsValid(World) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] World context is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(World);
	if (::IsValid(GameInstance) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] GameInstance is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	UMultiplaySessionSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiplaySessionSubsystem>();
	if (::IsValid(SessionSubsystem) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s]  SessionSubsystem is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	SessionSubsystem->RegisterPlayer(NewPlayer, false);
}

void ALobbyGameMode::StartBanPick()
{
	if (MatchState == EMatchState::Picking)
	{
		CurruentBanPickTime = BanPickTime;

		if (OnSelectionTimerChanged.IsBound())
		{
			OnSelectionTimerChanged.Broadcast(CurruentBanPickTime, BanPickTime);
		}

		uint8 NumberOfPlayers = BlueTeamPlayerControllers.Num() + RedTeamPlayerControllers.Num();
		UE_LOG(LogTemp, Log, TEXT("[Server] ALobbyGameMode - Start BanPick | NumberOfPlayers: %d"), NumberOfPlayers);

		GetWorld()->GetTimerManager().SetTimer(BanPickTimer, this, &ALobbyGameMode::UpdateBanPickTime, 0.5f, true, 0.0f);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Server] ALobbyGameMode - Can not Start Ban Pick, MatchState is not Picking State."));
	}
}

void ALobbyGameMode::UpdateBanPickTime()
{
	CurruentBanPickTime -= 0.5f;

	if (CurruentBanPickTime <= 60 && OnSelectionTimerChanged.IsBound())
	{
		OnSelectionTimerChanged.Broadcast(CurruentBanPickTime, BanPickTime);
	}

	if (CurruentBanPickTime <= -1)
	{
		GetWorld()->GetTimerManager().ClearTimer(BanPickTimer);
		GetWorld()->GetTimerManager().SetTimer(BanPickEndTimer, this, &ALobbyGameMode::EndBanPick, 2.0f, false);
	}
}

void ALobbyGameMode::EndBanPick()
{
	SavePlayerData();
	SaveGameData();
}

void ALobbyGameMode::SavePlayerData()
{
	for (APlayerController* PlayerController : BlueTeamPlayerControllers)
	{
		SavePlayerStateData(PlayerController);
	}

	for (APlayerController* PlayerController : RedTeamPlayerControllers)
	{
		SavePlayerStateData(PlayerController);
	}
}

void ALobbyGameMode::SavePlayerStateData(APlayerController* PlayerController)
{
	if (!::IsValid(PlayerController))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid PlayerController."));
		return;
	}

	ALobbyPlayerState* LobbyPlayerState = Cast<ALobbyPlayerState>(PlayerController->PlayerState);
	if (!::IsValid(LobbyPlayerState))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid PlayerState."));
		return;
	}

	UPlayerStateSave* PlayerStateSave = NewObject<UPlayerStateSave>();
	PlayerStateSave->TeamSide = LobbyPlayerState->TeamSide;
	PlayerStateSave->ChosenChampionName = LobbyPlayerState->ChosenChampionName;
	PlayerStateSave->PlayerIndex = LobbyPlayerState->PlayerIndex;
	PlayerStateSave->PlayerUniqueID = LobbyPlayerState->PlayerUniqueID;

	FString SafeSaveSlotName = LobbyPlayerState->GetPlayerUniqueId().Replace(TEXT("|"), TEXT("_"));
	UE_LOG(LogTemp, Warning, TEXT("Saving PlayerData: Slot: %s, PlayerIndex: %d, CharacterName: %s"), *SafeSaveSlotName, PlayerStateSave->PlayerIndex, *PlayerStateSave->ChosenChampionName.ToString());

	if (UGameplayStatics::SaveGameToSlot(PlayerStateSave, SafeSaveSlotName, 0))
	{
		UE_LOG(LogTemp, Log, TEXT("Save successful."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save player data to slot: %s"), *SafeSaveSlotName);
	}
}


void ALobbyGameMode::SaveGameData()
{
	UAOSGameInstance* AOSGameInstance = Cast<UAOSGameInstance>(GetGameInstance());
	if (::IsValid(AOSGameInstance))
	{
		AOSGameInstance->NumberOfPlayer = BlueTeamPlayerControllers.Num() + RedTeamPlayerControllers.Num();
		UE_LOG(LogTemp, Warning, TEXT("[ALobbyGameMode::SaveGameData] Number of player %d"), NumberOfConnectedPlayers);
	}

	ChangeLevel();
}

void ALobbyGameMode::ChangeLevel()
{
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindWeakLambda(this, [this]()
		{
			if (!::IsValid(this))
			{
				UE_LOG(LogTemp, Error, TEXT("[%s] GameMode instance is invalid. Aborting map change."), ANSI_TO_TCHAR(__FUNCTION__));
				return;
			}

			UWorld* WorldContext = GetWorld();
			if (::IsValid(WorldContext) == false)
			{
				UE_LOG(LogTemp, Error, TEXT("[%s] WorldContext is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
				return;
			}

			WorldContext->ServerTravel("/Game/FuryOfLegends/Level/Arena?listen", false, false);
			GetWorld()->GetTimerManager().ClearTimer(BanPickTimer);
		});

	GetWorld()->GetTimerManager().SetTimer(BanPickTimer, TimerDelegate, 1.0f, false);

}

FString ALobbyGameMode::GeneratePlayerUniqueID()
{
	FGuid NewGUID = FGuid::NewGuid();
	return NewGUID.ToString(EGuidFormats::Digits);
}