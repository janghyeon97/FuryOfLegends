// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_ConfirmationQuit.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UUW_ConfirmationQuit : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UUW_ConfirmationQuit(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnClickQuitButton();

	UFUNCTION(Client, Unreliable)
	void OnExecuteQuit_Client();

	UFUNCTION()
	void OnExecuteCancel();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ConfirmationQuit", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> DescriptionText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ConfirmationQuit", Meta = (BindWidget))
	TObjectPtr<class UUW_Button> QuitButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ConfirmationQuit", Meta = (BindWidget))
	TObjectPtr<class UUW_Button> CancelButton;
};
