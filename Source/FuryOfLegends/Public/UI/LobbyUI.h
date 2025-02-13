// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyUI.generated.h"

class UUW_ChatWindow;
class UUW_Button;

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API ULobbyUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	UUW_Button* GetGameStartButton() { return GameStartButton; };

	void InitializeSwitcher();

	void UpdateLobby();

	UFUNCTION()
	void OnGameStartButtonClicked();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ULobbyUI")
	TArray<class UWidgetSwitcher*> BlueTeamSwitcher;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ULobbyUI")
	TArray<class UWidgetSwitcher*> RedTeamSwitcher;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TObjectPtr<class ALobbyGameState> LobbyGameState;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TWeakObjectPtr<class ALobbyPlayerController> LobbyPlayerController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ULobbyUI", Meta = (BindWidget))
	TObjectPtr<class UUW_ChatWindow> ChatWindow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ULobbyUI", Meta = (BindWidget))
	TObjectPtr<class UImage> BackgroundImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ULobbyUI", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> CustomNameText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ULobbyUI", Meta = (BindWidget))
	TObjectPtr<class UStackBox> BlueTeamStackBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ULobbyUI", Meta = (BindWidget))
	TObjectPtr<class UStackBox> RedTeamStackBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ULobbyUI", Meta = (BindWidget))
	TObjectPtr<class UUW_Button> GameStartButton;
};
