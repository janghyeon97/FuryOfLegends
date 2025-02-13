// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgetBase.h"
#include "UserWidgetBarBase.generated.h"


class UBorder;
class UTextBlock;
class UProgressBar;



/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UUserWidgetBarBase : public UUserWidgetBase
{
	GENERATED_BODY()
	
public:
    UUserWidgetBarBase(const FObjectInitializer& ObjectInitializer);

    void InitializeWidget(const float InCurrentFigure, const float InMaxFigure, const float InRegeneration);

    void SetMaxFigure(float InMaxFigure);
    void SetCurrentFigure(float InCurrentFigure);
    void SetBorderColor(FLinearColor InColor);
    void SetProgressBarColor(FLinearColor InColor);
    void SetTextVisibility(ESlateVisibility InVisibility);

    UFUNCTION()
    void OnMaxMaxFigureChanged(float PreviousMaxigure, float NewMaxigure);

    UFUNCTION()
    void OnCurrentFigureChanged(float PreviousCurrentFigure, float NewCurrentFigure);

    UFUNCTION()
    void OnRegenerationChanged(float PreviousFigure, float NewFigure);

protected:
    virtual void NativeConstruct() override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bar", Meta = (BindWidget))
    TObjectPtr<UProgressBar> Bar;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bar", Meta = (BindWidget))
    TObjectPtr<UBorder> Border;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bar", Meta = (BindWidget))
    TObjectPtr<UTextBlock> CurrentFigureText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bar", Meta = (BindWidget))
    TObjectPtr<UTextBlock> MaxFigureText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bar", Meta = (BindWidget))
    TObjectPtr<UTextBlock> RegenerationText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bar", Meta = (BindWidget))
    TObjectPtr<UTextBlock> SlashIcon;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bar")
    float MaxFigure;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Bar")
    float CurrentFigure;
};
