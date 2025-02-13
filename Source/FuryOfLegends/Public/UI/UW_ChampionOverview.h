// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_ChampionOverview.generated.h"


class UImage;
class UTextBlock;
class AArenaGameState;
class UStatComponent;
class AAOSCharacterBase;

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UUW_ChampionOverview : public UUserWidget
{
	GENERATED_BODY()

	friend class UUHUD;
	
public:
	virtual void NativeOnInitialized() override;
	
	void InitializeWidget(AArenaGameState* InGameState, UStatComponent* InStatComponent, const int32 InPlayerIndex, const FName& InCharacterName, UTexture* InProfile);

	UFUNCTION()
	void UpdateChampionName(const FName& NewName);

	UFUNCTION()
	void UpdateLevelText(int32 PreviousLevel, int32 NewLevel);

	UFUNCTION()
	void UpdateRespawnTimer(const uint32 UniqueCode, const float RemainingTime, const float ElapsedTime);

private:
	TWeakObjectPtr<AArenaGameState> GameState;
	TWeakObjectPtr<UStatComponent> StatComponent;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Overview", Meta = (BindWidget))
	TObjectPtr<UImage> ChampionImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Overview", Meta = (BindWidget))
	TObjectPtr<UImage> ChampionImageBorder;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Overview", Meta = (BindWidget))
	TObjectPtr<UTextBlock> ChampionNameText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Overview", Meta = (BindWidget))
	TObjectPtr<UImage> LevelBackgroundImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Overview", Meta = (BindWidget))
	TObjectPtr<UTextBlock> LevelText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Overview", Meta = (BindWidget))
	TObjectPtr<UImage> RespawnTimeImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Overview", Meta = (BindWidget))
	TObjectPtr<UTextBlock> RespawnTimeText;

private:
	int32 OwnerPlayerIndex;
};
