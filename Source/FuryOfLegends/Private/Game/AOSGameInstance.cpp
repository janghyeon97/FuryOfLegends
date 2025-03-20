// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/AOSGameInstance.h"
#include "Structs/ActionData.h"
#include "Structs/MinionData.h"
#include "Structs//CharacterData.h"
#include "Structs//CharacterResources.h"
#include "Plugins/MultiplaySessionSubsystem.h"


UAOSGameInstance::UAOSGameInstance()
{
	NumberOfPlayer = -1;
}

void UAOSGameInstance::Init()
{
	Super::Init();

	if (IsDedicatedServerInstance())
	{
		UMultiplaySessionSubsystem* SubSystem = GetSubsystem<UMultiplaySessionSubsystem>();
		if (!SubSystem)
		{
			return;
		}

		SubSystem->CreateSession(true, false, "DedicatedServerSession", 10, "Aurora", "");
	}
}

void UAOSGameInstance::Shutdown()
{
	Super::Shutdown();
}

const UDataTable* UAOSGameInstance::GetChampionListTable() const
{
	return ::IsValid(ChampionList) ? ChampionList : nullptr;
}

const FCharacterAttributesRow* UAOSGameInstance::GetChampionListTableRow(const FName& RowName) const
{
	if (!::IsValid(ChampionList))
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] ChampionList is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return nullptr;
	}

	if (RowName.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] RowName is empty."), ANSI_TO_TCHAR(__FUNCTION__));
		return nullptr;
	}

	const FCharacterAttributesRow* ChampionListRow = ChampionList->FindRow<FCharacterAttributesRow>(RowName, TEXT(""));
	if (!ChampionListRow)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to find row in ChampionList for RowName: %s"), ANSI_TO_TCHAR(__FUNCTION__), *RowName.ToString());
		return nullptr;
	}

	return ChampionListRow;
}

const UDataTable* UAOSGameInstance::GetMinionsListTable() const
{
	return ::IsValid(MinionsList) ? MinionsList : nullptr;
}

const FMinionAttributesRow* UAOSGameInstance::GetMinionsListTableRow(const FName& RowName) const
{
	if (!::IsValid(MinionsList))
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] MinionsList is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return nullptr;
	}

	if (RowName.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] RowName is empty."), ANSI_TO_TCHAR(__FUNCTION__));
		return nullptr;
	}

	const FMinionAttributesRow* MinionDataTableRow = MinionsList->FindRow<FMinionAttributesRow>(RowName, TEXT(""));
	if (!MinionDataTableRow)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to find row in MinionsList for RowName: %s"), ANSI_TO_TCHAR(__FUNCTION__), *RowName.ToString());
		return nullptr;
	}

	return MinionDataTableRow;
}


const UDataTable* UAOSGameInstance::GetSharedGamePlayParticlesDataTable()
{
	return SharedGamePlayParticlesDataTable;
}

FSharedGameplay* UAOSGameInstance::GetSharedGamePlayParticles()
{
	if (!SharedGamePlayParticlesDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("GetSharedGamePlayParticles: SharedGamePlayParticlesDataTable is null."));
		return nullptr;
	}

	FSharedGameplay* SharedGameplayRow = SharedGamePlayParticlesDataTable->FindRow<FSharedGameplay>(FName(*FString::FromInt(1)), TEXT(""));
	if (!SharedGameplayRow)
	{
		UE_LOG(LogTemp, Error, TEXT("GetSharedGamePlayParticles: Failed to find row in SharedGamePlayParticlesDataTable."));
		return nullptr;
	}

	return SharedGameplayRow;
}