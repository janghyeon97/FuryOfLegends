// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TitleLevelUI.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UTitleLevelUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UTitleLevelUI(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	UFUNCTION()
	void ProcessLogin();

	UFUNCTION()
	void CreateHostMenu();

	UFUNCTION()
	void CreateJoinMenu();

	UFUNCTION()
	void CreateQuitMenu();

	UFUNCTION()
	void OnLoginCompleted(bool Successful);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TitleLevel", Meta = (BindWidget))
	TObjectPtr<class UUW_Button> LoginButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TitleLevel", Meta = (BindWidget))
	TObjectPtr<class UUW_Button> HostGameButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TitleLevel", Meta = (BindWidget))
	TObjectPtr<class UUW_Button> JoinGameButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TitleLevel", Meta = (BindWidget))
	TObjectPtr<class UUW_Button> QuitButton;
};
