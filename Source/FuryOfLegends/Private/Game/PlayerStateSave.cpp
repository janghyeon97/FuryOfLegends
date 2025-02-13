// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PlayerStateSave.h"

UPlayerStateSave::UPlayerStateSave()
{
	TeamSide = ETeamSide::None;

	PlayerUniqueID = FString();
	PlayerIndex = -1;
	ChosenChampionName = NAME_None;
}
