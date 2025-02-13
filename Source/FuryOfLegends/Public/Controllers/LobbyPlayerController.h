// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

class UUW_ChatWindow;

/**
 *
 */
UCLASS()
class FURYOFLEGENDS_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ALobbyPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	// UI 관련 함수
	void BindChatWindow(UUW_ChatWindow* Widget);
	void ShowLobbyUI();

private:
	void LoadGameInstance();
	void LoadGameMode();
	void LoadGameState();
	void LoadPlayerState();

public:

	UFUNCTION(Client, Reliable)
	void ShowLoadingScreen_Client();

	UFUNCTION(Server, Reliable)
	void ShowChampionSelectUI_Server();

	UFUNCTION(Client, Reliable)
	void ShowChampionSelectUI_Client();

	UFUNCTION(Client, Reliable)
	void SetHostPlayer_Client(bool bIsHost);

	// 채팅 관련 함수
	UFUNCTION(Server, Reliable)
	void UpdateChatLog_Server(const FString& Message, AController* EventInstigator);

	UFUNCTION(Client, Reliable)
	void UpdateChatLog_Client(const FString& Text);

	// 캐릭터 선택 정보 업데이트 함수
	UFUNCTION(Server, Reliable)
	void UpdatePlayerSelection_Server(ETeamSide Team, int32 PlayerIndex, const FName& InPlayerName, const FName& ChampionName, FLinearColor Color, bool bShowCharacterDetails);

	UFUNCTION(Client, Reliable)
	void UpdatePlayerSelection_Client(ETeamSide Team, int32 PlayerIndex, const FName& InPlayerName, const FName& ChampionName, FLinearColor Color, bool bShowCharacterDetails);

	// 밴픽 시간 업데이트 함수
	UFUNCTION(Client, Reliable)
	void UpdateBanPickTime_Client(float InCurrentDraftTime, float InMaxDraftTime);

	// 호스트 플레이어 여부 확인 함수
	bool GetIsHostPlayer() const { return bIsHostPlayer; }

protected:
	// 게임 인스턴스, 게임 모드, 게임 상태, 플레이어 상태
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALobbyPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class UAOSGameInstance> AOSGameInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALobbyPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class ALobbyGameMode> LobbyGameMode;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALobbyPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class ALobbyGameState> LobbyGameState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ALobbyPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class ALobbyPlayerState> LobbyPlayerState;

	// UI 클래스와 인스턴스
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALobbyPlayerController", Meta = (AllowPrivateAccess))
	TSubclassOf<class ULobbyUI> LobbyUIClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "ALobbyPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class ULobbyUI> LobbyUIInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALobbyPlayerController", Meta = (AllowPrivateAccess))
	TSubclassOf<class UChampionSelectionUI> ChampionSelectUIClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "ALobbyPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class UChampionSelectionUI>ChampionSelectUIInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ALobbyPlayerController", Meta = (AllowPrivateAccess))
	TSubclassOf<class UUserWidget> LoadingScreenClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "ALobbyPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class UUserWidget> LoadingScreenInstance;

	// 채팅 윈도우
	UPROPERTY(Meta = (AllowPrivateAccess))
	TObjectPtr<class UUW_ChatWindow> ChatWindow;

	// 호스트 플레이어 여부
	bool bIsHostPlayer;

	// 드래프트 타임 관련 변수
	bool bChangedDraftTime = false;
	float DraftTime = 0.f;
	float CurrentDraftTime = 0.f;
	float MaxDraftTime = 0.f;
};

