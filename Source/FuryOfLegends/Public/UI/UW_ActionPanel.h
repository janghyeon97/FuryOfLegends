// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_ActionPanel.generated.h"


class UBorder;
class UTextBlock;
class UImage;
class UHorizontalBox;
class UHorizontalBoxSlot;
class UActionStatComponent;
class UTexture;

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UUW_ActionPanel : public UUserWidget
{
	GENERATED_BODY()	
	
public:
	virtual void NativeOnInitialized() override;

	void InitializeWidget(UActionStatComponent* InActionStatComponent);

	void SetActionSlotText(const FString& SlotText);
	void SetActionMaxLevel(const int32 InActionLevel);
	void SetActionCurrentLevel(const int32 InActionLevel);
	void SetCooldownTime(const float CurrentCooldownTime, const float MaxCooldownTime);
	void SetActionImage(UTexture* InActionImage);
	void SetLevelUpArrowVisibility(const bool bIsVisible);
	void SetActionActivation(const bool bIsVisible);


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActionPanel", Meta = (BindWidget))
	TObjectPtr<UImage> LevelUpArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActionPanel", Meta = (BindWidget))
	TObjectPtr<UImage> ActionImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActionPanel", Meta = (BindWidget))
	TObjectPtr<UTextBlock> CooldownTimeText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActionPanel", Meta = (BindWidget))
	TObjectPtr<UBorder> ActionIconShade;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActionPanel", Meta = (BindWidget))
	TObjectPtr<UHorizontalBox> ActionLevel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActionPanel", Meta = (BindWidget))
	TObjectPtr<UTextBlock> ActionSlotText;

private:
	TWeakObjectPtr<UActionStatComponent> ActionStatComponent;
};
