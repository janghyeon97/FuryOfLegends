#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Structs/CharacterData.h"
#include "ChampionSelectionUI.generated.h"

class UUW_ChampionSelection;

UCLASS()
class FURYOFLEGENDS_API UChampionSelectionUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	class UWrapBox* GetChampionListBox() { return ChampionListBox; };

	void InitializePlayerList();
	void InitializeChampionList();
	void AddChampionListEntry(int32 Index, const FName& InChampionName, UTexture* Texture);

	void AddPlayerSelection(ETeamSide Team, const FName& InPlayerName);
	void AddPlayerSelectionUI(TArray<TObjectPtr<UUW_ChampionSelection>>& TeamPlayers, TArray<TObjectPtr<UWidget>>& TeamWidgets, uint8& CurrentIndex, const FName& InPlayerName);

	void UpdatePlayerSelection(ETeamSide Team, const int32 PlayerIndex, const FName& InPlayerName, UTexture* Texture, const FName& InChampionName, const FName& InChampionPosition, FLinearColor Color, bool bShowChampionDetails);
	void UpdatePlayerSelectionUI(TArray<TObjectPtr<UUW_ChampionSelection>>& TeamPlayers, const int32 PlayerIndex, const FName& InPlayerName, UTexture* Texture, const FName& InChampionName, const FName& InChampionPosition, FLinearColor Color, bool bShowChampionDetails);

	void UpdateInfomationText(const FString& String);
	void OnBanPickTimeChanged(float CurrentTime, float MaxTime);

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TWeakObjectPtr<class ALobbyGameState> LobbyGameState;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TWeakObjectPtr<class UAOSGameInstance> AOSGameInstance;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UChampionSectionUI", Meta = (AllowPrivateAccess))
	TSubclassOf<class UUserWidget> ChampionListEntryClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UChampionSectionUI", Meta = (AllowPrivateAccess))
	TSubclassOf<class UUserWidget> ChampionSelectionClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UChampionSectionUI", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> InfomationText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UChampionSectionUI", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> TimerText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UChampionSectionUI", Meta = (BindWidget))
	TObjectPtr<class UProgressBar> BlueProgressBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UChampionSectionUI", Meta = (BindWidget))
	TObjectPtr<class UProgressBar> RedProgressBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UChampionSectionUI", Meta = (BindWidget))
	TObjectPtr<class UWrapBox> RedTeamSelection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UChampionSectionUI", Meta = (BindWidget))
	TObjectPtr<class UWrapBox> BlueTeamSelection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UChampionSectionUI", Meta = (BindWidget))
	TObjectPtr<class UWrapBox> ChampionListBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UChampionSectionUI", Meta = (BindWidget))
	TObjectPtr<class UUW_ChatWindow> ChatWindow;

	TArray<TObjectPtr<UUW_ChampionSelection>> BlueTeamPlayers;
	TArray<TObjectPtr<UUW_ChampionSelection>> RedTeamPlayers;
	TArray<TObjectPtr<class UWidget>> BlueTeamWidgets;
	TArray<TObjectPtr<class UWidget>> RedTeamWidgets;

	uint8 BlueTeamCurrentIndex = 0;
	uint8 RedTeamCurrentIndex = 0;
};
