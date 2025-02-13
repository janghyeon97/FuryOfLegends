// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Structs/CharacterData.h"
#include "ArenaGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRespawnTimeChangedDelegate, uint32, UniqueCode, float, InRemainingTime, float, InElapsedTime);


class AAOSCharacterBase;
struct FItemTableRow;

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API AArenaGameState : public AGameStateBase
{
	GENERATED_BODY()

public:	
	friend class AArenaGameMode;

	AArenaGameState();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	const TArray<AAOSCharacterBase*> GetPlayers(ETeamSide Team) const;

	void SetLoadedItems(const TArray<FItemTableRow>& Items) { LoadedItems = Items; };
	const TArray<FItemTableRow>& GetLoadedItems() { return LoadedItems; };

	float GetElapsedTime() const { return ElapsedTime; };
	FItemTableRow* GetItemInfoByID(int32 ItemCode);

	void StartGame();
	void AddPlayerCharacter(AAOSCharacterBase* Character, ETeamSide TeamSide);
	void RemovePlayerCharacter(AAOSCharacterBase* Character);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBroadcastRespawnTime(const uint32 UniqueCode, const float InRemainingTime, const float InElapsedTime);

public:
	FOnRespawnTimeChangedDelegate OnRespawnTimeChanged;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ArenaGameState", Meta = (AllowPrivateAccess))
	TObjectPtr<class AArenaGameMode> ArenaGameMode;

	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, Category = "ArenaGameState", Meta = (AllowPrivateAccess))
	TArray<AAOSCharacterBase*> BlueTeamPlayers;

	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, Category = "ArenaGameState", Meta = (AllowPrivateAccess))
	TArray<AAOSCharacterBase*> RedTeamPlayers;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	TArray<FItemTableRow> LoadedItems;

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ArenaGameState", Meta = (AllowPrivateAccess))
	float StartTime = 0.f;

	UPROPERTY(Replicated, VisibleDefaultsOnly, BlueprintReadOnly, Category = "ArenaGameState", Meta = (AllowPrivateAccess))
	float ElapsedTime = 0.f;

	TMap<AAOSCharacterBase*, int32> RespawnTime;

	bool bIsGameStarted = false;
};