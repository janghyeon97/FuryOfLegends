// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

UENUM(BlueprintType)
enum class EMatchState : uint8
{
    None,
    Waiting,
    Picking,
    Playing,
    End
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSelectionTimerChangedDelegate, float, CurrentDraftTime, float, MaxDraftTime);
DECLARE_MULTICAST_DELEGATE(FOnConnectedPlayerChangedDelegate);

class ALobbyPlayerController;

/**
 *
 */
UCLASS()
class FURYOFLEGENDS_API ALobbyGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ALobbyGameMode();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    virtual void RegisterNewPlayer(APlayerController* NewPlayer);

public:
    TArray<ALobbyPlayerController*> GetBlueTeamPlayerControllers() const { return BlueTeamPlayerControllers; }
    TArray<ALobbyPlayerController*> GetRedTeamPlayerControllers() const { return RedTeamPlayerControllers; }
    FString GeneratePlayerUniqueID();

    void SavePlayerData();
    void SavePlayerStateData(APlayerController* PlayerController);
    void SaveGameData();
    void ChangeLevel();
    void StartBanPick();
    void UpdateBanPickTime();
    void EndBanPick();

    FOnConnectedPlayerChangedDelegate OnConnectedPlayerChanged;
    FOnSelectionTimerChangedDelegate OnSelectionTimerChanged;

public:
    EMatchState MatchState = EMatchState::Waiting;

    bool bIsFirstConnectedPlayer = true;

    uint8 NumberOfConnectedPlayers = 0;
    uint8 BanPickPhase = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BanPickTime = 60;
    float CurruentBanPickTime = 0;

protected:
    FTimerHandle BanPickTimer;
    FTimerHandle BanPickEndTimer;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
    TArray<TObjectPtr<ALobbyPlayerController>> BlueTeamPlayerControllers;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
    TArray<TObjectPtr<ALobbyPlayerController>> RedTeamPlayerControllers;
};
