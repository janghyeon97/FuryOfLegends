// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ArenaGameState.h"
#include "Game/ArenaGameMode.h"
#include "Characters/AOSCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Item/ItemData.h"
#include "Engine/Engine.h"

AArenaGameState::AArenaGameState()
{
	StartTime = 0.0f;
	ElapsedTime = 0.0f;
}

void AArenaGameState::BeginPlay()
{
	Super::BeginPlay();

	ArenaGameMode = Cast<AArenaGameMode>(UGameplayStatics::GetGameMode(this));
	if (::IsValid(ArenaGameMode))
	{
		LoadedItems = ArenaGameMode->GetLoadedItems();
		UE_LOG(LogTemp, Log, TEXT("[%s] LoadedItems count: %d"), ANSI_TO_TCHAR(__FUNCTION__), LoadedItems.Num());
	}
}

void AArenaGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsGameStarted)
	{
		ElapsedTime = GetWorld()->GetTimeSeconds() - StartTime;
	}
}

void AArenaGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, BlueTeamPlayers);
	DOREPLIFETIME(ThisClass, RedTeamPlayers);
	DOREPLIFETIME(ThisClass, LoadedItems);
	DOREPLIFETIME(ThisClass, ElapsedTime);
}

const TArray<AAOSCharacterBase*> AArenaGameState::GetPlayers(ETeamSide Team) const
{
	switch (Team)
	{
	case ETeamSide::Blue:
		return BlueTeamPlayers;
	case ETeamSide::Red:
		return RedTeamPlayers;
	default:
		return TArray<AAOSCharacterBase*>();
	}
}

FItemTableRow* AArenaGameState::GetItemInfoByID(int32 ItemCode)
{
	for (FItemTableRow& Item : LoadedItems)
	{
		if (Item.ItemCode == ItemCode)
		{
			return &Item;
		}
	}
	return nullptr;
}

void AArenaGameState::StartGame()
{
	StartTime = GetWorld()->GetTimeSeconds();
	bIsGameStarted = true;
}

void AArenaGameState::AddPlayerCharacter(AAOSCharacterBase* Character, ETeamSide TeamSide)
{
	switch (TeamSide)
	{
	case ETeamSide::Blue:
		UE_LOG(LogTemp, Log, TEXT("[AArenaGameState::AddCharacter] Successfully added character %s to Blue Team."), *Character->GetName());
		BlueTeamPlayers.Add(Character);
		break;
	case ETeamSide::Red:
		UE_LOG(LogTemp, Log, TEXT("[AArenaGameState::AddCharacter] Successfully added character %s to Red Team."), *Character->GetName());
		RedTeamPlayers.Add(Character);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("[AArenaGameState::AddCharacter] Failed to add character, team assignment must be either Blue or Red team."));
		break;
	}
}

void AArenaGameState::RemovePlayerCharacter(AAOSCharacterBase* Character)
{
	if (!Character)
	{
		UE_LOG(LogTemp, Error, TEXT("[AArenaGameState::RemovePlayerCharacter] Character is null."));
		return;
	}

	if ((INDEX_NONE != BlueTeamPlayers.Find(Character) || INDEX_NONE != RedTeamPlayers.Find(Character)))
	{
		BlueTeamPlayers.Remove(Character);
		RedTeamPlayers.Remove(Character);
	}
}

void AArenaGameState::MulticastBroadcastRespawnTime_Implementation(const uint32 UniqueCode, const float InRemainingTime, const float InElapsedTime)
{
	if (OnRespawnTimeChanged.IsBound())
	{
		OnRespawnTimeChanged.Broadcast(UniqueCode, InRemainingTime, InElapsedTime);
	}
}

