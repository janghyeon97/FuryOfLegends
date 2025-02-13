// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/LobbyPlayerState.h"
#include "Game/AOSGameInstance.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

ALobbyPlayerState::ALobbyPlayerState()
{
	TeamSide = ETeamSide::None;
	PlayerIndex = -1;
	SelectedChampionIndex = -1;
	ChosenChampionName = NAME_None;
	PlayerUniqueID = FString();
}

void ALobbyPlayerState::BeginPlay()
{
	Super::BeginPlay();

}

void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, TeamSide);
	DOREPLIFETIME(ThisClass, PlayerIndex);
	DOREPLIFETIME(ThisClass, SelectedChampionIndex);
	DOREPLIFETIME(ThisClass, PlayerUniqueID); 
	DOREPLIFETIME(ThisClass, ChosenChampionName);
}

void ALobbyPlayerState::UpdateSelectedChampion_Server_Implementation(const int32 Index, const FName& InName)
{
	SelectedChampionIndex = Index;
	ChosenChampionName = InName;
}

FString ALobbyPlayerState::GetPlayerUniqueId() const
{
#if WITH_EDITOR
	// PIE ȯ�� Ȯ��
	if (GIsEditor && GetWorld()->IsPlayInEditor())
	{
		// PIE �ν��Ͻ� ID ��������
		return FString::Printf(TEXT("PIE_Player_%s"), *GetPlayerName());
	}
#endif

	// ���� ���� ȯ�濡���� ���� ID ��ȯ
	const FUniqueNetIdRepl UniqueNetId = GetUniqueId();
	if (UniqueNetId.IsValid())
	{
		return UniqueNetId->ToString();
	}

	UAOSGameInstance* GameInstance = Cast<UAOSGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] GetPlayerUniqueId failed: return %s"), ANSI_TO_TCHAR(__FUNCTION__), *GameInstance->GameInstanceID);
		return GameInstance->GameInstanceID;
	}

	return FString();
}

