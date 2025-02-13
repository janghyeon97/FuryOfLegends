// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/LobbyGameState.h"
#include "Game/LobbyGameMode.h"
#include "Game/ArenaPlayerState.h"
#include "Controllers/LobbyPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

ALobbyGameState::ALobbyGameState()
{
	
}

void ALobbyGameState::BeginPlay()
{
	
}

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, BlueTeamPlayers);
	DOREPLIFETIME(ThisClass, RedTeamPlayers);
}

void ALobbyGameState::UpdatePlayers()
{
	/*if (::IsValid(LobbyGameMode))
	{
		TArray<ALobbyPlayerController*> ConnectedBlueTeamPlayers = LobbyGameMode->GetBlueTeamPlayerControllers();
		TArray<ALobbyPlayerController*> ConnectedRedTeamPlayers = LobbyGameMode->GetRedTeamPlayerControllers();

		NumberOfPlayers = LobbyGameMode->NumberOfConnectedPlayers;

		BlueTeamPlayers.Empty();

		if (ConnectedBlueTeamPlayers.Num() > 0 && ConnectedBlueTeamPlayers.IsValidIndex(0))
		{
			UE_LOG(LogTemp, Log, TEXT("------------ BlueTeam Player List -------------"));

			int Index = 0;

			for (auto& Player : ConnectedBlueTeamPlayers)
			{				
				AArenaPlayerState* PlayerState = Player->GetPlayerState<AArenaPlayerState>();

				if (::IsValid(PlayerState))
				{
					FPlayerInfomaion NewPlayer;

					NewPlayer.PlayerName = PlayerState->GetPlayerName();
					NewPlayer.PlayerController = Player;

					BlueTeamPlayers.Add(NewPlayer);

					UE_LOG(LogTemp, Log, TEXT("			%d. %s"), Index++, *NewPlayer.PlayerName);
				}
			}
		}
		

		RedTeamPlayers.Empty();

		if (ConnectedRedTeamPlayers.Num() > 0 && ConnectedRedTeamPlayers.IsValidIndex(0))
		{
			UE_LOG(LogTemp, Log, TEXT("------------ RedTeam Player List -------------"));

			int Index = 0;

			for (auto& Player : ConnectedRedTeamPlayers)
			{
				AArenaPlayerState* PlayerState = Player->GetPlayerState<AArenaPlayerState>();
				if (::IsValid(PlayerState))
				{
					FPlayerInfomaion NewPlayer;

					NewPlayer.PlayerName = PlayerState->GetPlayerName();
					NewPlayer.PlayerController = Player;

					RedTeamPlayers.Add(NewPlayer);

					UE_LOG(LogTemp, Log, TEXT("			%d. %s"), Index++, *NewPlayer.PlayerName);
				}
			}
		}
	
		if (OnConnectedPlayerReplicated.IsBound())
		{
			OnConnectedPlayerReplicated.Broadcast();
		}
	}*/
}

void ALobbyGameState::OnRep_ConnectedPlayerChanged()
{
	if (OnConnectedPlayerReplicated.IsBound())
	{
		OnConnectedPlayerReplicated.Broadcast();
	}
}
