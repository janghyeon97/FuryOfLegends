// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UHUD.generated.h"

// Delegate declaration
DECLARE_MULTICAST_DELEGATE(FOnComponentsBindingCompletedDelegate);


class UStatComponent;
class UActionStatComponent;
class AArenaPlayerState;
class AArenaGameState;
class ACharacterBase;
class AAOSCharacterBase;
class UUW_ItemShop;
class UUW_ActionPanel;
class UHorizontalBox;
class UButton;
class UComboBoxString;
class UUW_ResourceBars;
class UUW_ChampionOverview;
class UUW_StatPanel;
class UUW_Inventory;
class UTextBlock;
class UCanvasPanel;
class USizeBox;


/**
 * User HUD class for displaying player information and stats
 */
UCLASS()
class FURYOFLEGENDS_API UUHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

public:
	// Functions for updating HUD elements
	UFUNCTION()
	void DecreaseHP();

	UFUNCTION()
	void DecreaseMP();

	UFUNCTION()
	void IncreaseEXP();

	UFUNCTION()
	void IncreaseLevel();

	UFUNCTION()
	void IncreaseCriticalChance();

	UFUNCTION()
	void IncreaseAttackSpeed();

	UFUNCTION()
	void ChangeTeamSide(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void ToggleHUD();


	UFUNCTION()
	void OnActionActivationChanged(EActionSlot SlotID, bool InVisibility);

	UFUNCTION()
	void OnActionUpgradeStateChanged(EActionSlot SlotID, bool InVisibility);

	UFUNCTION()
	void OnActionLevelChanged(EActionSlot SlotID, int InLevel);

	UFUNCTION()
	void OnActionCooldownTimeChanged(EActionSlot SlotID, float CurrentCooldownTime, float MaxCooldownTime);

	UFUNCTION()
	void OnInventoryChanged(const int32 InventortIndex, const int32 ItemCode, const int32 CurrentStack);

	UFUNCTION()
	void OnCurrencyChanged(const int32 NewCurrency);

	UFUNCTION()
	void OnAlertTextChanged(const FString& InString);

	UFUNCTION()
	void OnTimerUpdated(const uint32 UniqueCode, const int32 ConcurrentUses);

	UFUNCTION()
	void HandleTimerByCategory(const uint32 UniqueCode, const float RemainingTime, const float ElapsedTime);

	UFUNCTION()	
	void UpdateActionTimer(const uint32 UniqueCode, const float RemainingTime, const float ElapsedTime);

	UFUNCTION()
	void UpdateItemTimer(const uint32 UniqueCode, const float RemainingTime, const float ElapsedTime);

	UFUNCTION()
	void UpdateInventoryTimer(const uint32 UniqueCode, const float RemainingTime, const float ElapsedTime);

	UFUNCTION()
	void UpdateItemBuffTimer(const uint32 UniqueCode, const float RemainingTime, const float ElapsedTime);

public:
	void InitializeHUD(AAOSCharacterBase* OwningPlayer);

	// Getter and Setter for OwningActor
	ACharacterBase* GetOwningCharacter() const { return OwningCharacter; }
	void SetOwningActor(ACharacterBase* InOwningCharacter) { OwningCharacter = InOwningCharacter; }

	void BindItemShop(UUW_ItemShop* InItemShop);

private:
	void InitializeActions();
	void BindStatComponent(UStatComponent* InStatComponent);

public:
	// Delegate for initialization completion
	FOnComponentsBindingCompletedDelegate OnComponentsBindingCompleted;

public:
	// Weak pointers to components
	TWeakObjectPtr<UStatComponent> StatComponent;
	TWeakObjectPtr<UActionStatComponent> ActionStatComponent;
	TWeakObjectPtr<AArenaPlayerState> PlayerState;
	TWeakObjectPtr<AArenaGameState> GameState;
	TWeakObjectPtr<UUW_ItemShop> ItemShop;

	// -----------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD")
	TObjectPtr<ACharacterBase> OwningCharacter;

	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<USizeBox> HUD;

	// Buttons
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<UButton> DecreaseHPButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<UButton> DecreaseMPButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<UButton> IncreaseEXPButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<UButton> IncreaseLevelButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<UButton> IncreaseCriticalButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<UButton> IncreaseAttackSpeedButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<UComboBoxString> TeamSelectionComboBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<UButton> HUDToggleButton;

	// Text blocks for various stats
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<UTextBlock> DescriptionText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (AllowPrivateAccess))
	TMap<EActionSlot, UUW_ActionPanel*> Actions;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<UHorizontalBox> ActionPanels;

	// Progress bars
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<UUW_ResourceBars> ResourceBars;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<UUW_ChampionOverview> ChampionOverview;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<UUW_StatPanel> StatPanel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<UUW_Inventory> Inventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget))
	TObjectPtr<UHorizontalBox> BuffList;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UHUD", Meta = (AllowPrivateAccess))
	TObjectPtr<UClass> ActionPanelClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UHUD", Meta = (AllowPrivateAccess))
	TObjectPtr<UClass> ItemEntryClass;

	//TMap<FName, UTexture*> GamePlayTextures;
	TMap<uint32, class UUW_ItemEntry*> TimerWidgets;

	TArray<UMaterialInstanceDynamic*> MaterialRef;

	FName PlayerCharacterName;
	int32 PlayerIndex = 0;
};
