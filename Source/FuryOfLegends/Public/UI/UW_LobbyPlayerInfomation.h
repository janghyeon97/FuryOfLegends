// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_LobbyPlayerInfomation.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UUW_LobbyPlayerInfomation : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	void UpdatePlayerNameText(FString InName);

	void UpdatePlayerNameColor(FLinearColor InColor);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LobbyPlayerInfomation", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> PlayerNameText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LobbyPlayerInfomation", Meta = (BindWidget))
	TObjectPtr<class UButton> TeamChangeButton;
};
