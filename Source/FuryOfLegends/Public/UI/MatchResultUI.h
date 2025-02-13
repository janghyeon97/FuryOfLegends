// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MatchResultUI.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UMatchResultUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;

	void UpdateResultText(const FString& NewText);

	UFUNCTION()
	void OnButtonClicked();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget, AllowPrivateAccess))
	TObjectPtr<class UTextBlock> MatchResultText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHUD", Meta = (BindWidget, AllowPrivateAccess))
	TObjectPtr<class UUW_Button> ContinueButton;
};
