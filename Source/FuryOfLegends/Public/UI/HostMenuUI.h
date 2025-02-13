// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HostMenuUI.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UHostMenuUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UHostMenuUI(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnConfirmButtonClicked();

	UFUNCTION()
	void RemoveHostMenu();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UHostMenuUI")
	FColor FailureColor = FColor::White;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHostMenuUI", Meta = (BindWidget))
	TObjectPtr<class UUW_EditableText> EditableNameText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UHostMenuUI", Meta = (BindWidget))
	TObjectPtr<class UUW_EditableText> EditablePasswordText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TitleLevel", Meta = (BindWidget))
	TObjectPtr<class UUW_Button> ConfirmButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TitleLevel", Meta = (BindWidget))
	TObjectPtr<class UUW_Button> CancelButton;
};
