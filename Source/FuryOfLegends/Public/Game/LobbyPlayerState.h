// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Structs/CharacterData.h"
#include "LobbyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API ALobbyPlayerState : public APlayerState
{
	GENERATED_BODY()


public:
	ALobbyPlayerState();

	FString GetPlayerUniqueId() const;

	void SetSelectedChampionIndex(int32 Index) { SelectedChampionIndex = Index; };

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void UpdateSelectedChampion_Server(const int32 Index, const FName& InName);

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AArenaPlayerState", Meta = (AllowPrivateAccess))
	TObjectPtr<class ALobbyGameState> LobbyGameState;

public:
	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	ETeamSide TeamSide;

	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	FString PlayerUniqueID;

	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	int32 PlayerIndex;

	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	int32 SelectedChampionIndex;

	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	FName ChosenChampionName;
};
