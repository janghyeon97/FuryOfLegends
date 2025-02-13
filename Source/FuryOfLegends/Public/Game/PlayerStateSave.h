// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Game/ArenaPlayerState.h"
#include "PlayerStateSave.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UPlayerStateSave : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPlayerStateSave();

public:
	UPROPERTY()
	FString PlayerUniqueID;

	UPROPERTY()
	ETeamSide TeamSide;

	UPROPERTY()
	int32 PlayerIndex;

	UPROPERTY()
	FName ChosenChampionName;
};
