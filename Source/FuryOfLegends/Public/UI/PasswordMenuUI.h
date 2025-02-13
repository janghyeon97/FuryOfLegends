// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PasswordMenuUI.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UPasswordMenuUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void ConfirmPassword();

	UFUNCTION()
	void RemovePasswordMenu();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UPasswordMenuUI", Meta = (BindWidget))
	TObjectPtr<class UUW_EditableText> EditablePasswordText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UPasswordMenuUI", Meta = (BindWidget))
	TObjectPtr<class UUW_Button> ConfirmButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UPasswordMenuUI", Meta = (BindWidget))
	TObjectPtr<class UUW_Button> CancelButton;
};
