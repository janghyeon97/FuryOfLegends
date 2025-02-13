// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_Button.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UUW_Button : public UUserWidget
{
	GENERATED_BODY()

public:
	UUW_Button(const FObjectInitializer &ObjectInitializer);

	virtual void NativeConstruct() override;

	UFUNCTION()
	void PlayHoverAnimation();

	UFUNCTION()
	void PlayUnHoverAnimation();

	UFUNCTION()
	void PlayConstructAnimation();

	UFUNCTION()
	void SetButtonText(FString InText);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UW_Button")
	FLinearColor BackgroundColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UW_Button")
	FString String;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "UW_Button", Meta = (BindWidget))
	TObjectPtr<class UButton> Button;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "UW_Button", Meta = (BindWidget))
	TObjectPtr<class UBorder> ButtonBackground;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "UW_Button", Meta = (BindWidget))
	TObjectPtr<class UImage> ButtonStroke;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "UW_Button", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> ButtonText;

	// User Widget Animation ---------------------------------
	UPROPERTY(VisibleDefaultsOnly, Transient, Category = "UW_Button", meta = (BindWidgetAnim))
	TObjectPtr<class UWidgetAnimation> HoverAnimation;

	UPROPERTY(VisibleDefaultsOnly, Transient, Category = "UW_Button", meta = (BindWidgetAnim))
	TObjectPtr<class UWidgetAnimation> ConstructAnimation;
};
