// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_ChampionSelection.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UUW_ChampionSelection : public UUserWidget
{
	GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    void InitializeListEntry();

    void UpdateChampionNameText(const FName& InString);

    void UpdateChampionNameColor(FLinearColor InColor);

    void UpdateChampionPositionText(const FName& InString);

    void UpdateChampionPositionColor(FLinearColor InColor);
    
    void UpdatePlayerNameText(const FName& InString);

    void UpdatePlayerNameColor(FLinearColor InColor);

    void UpdateBorderImageColor(FLinearColor InColor);

    void SetChampionInfoVisibility(bool bVisibility);

    UFUNCTION(BlueprintCallable)
    void UpdateCampionImage(UTexture* InTexture);

    UMaterialInstanceDynamic* MaterialRef = nullptr;

protected:  
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ChampionSelection", Meta = (BindWidget))
    TObjectPtr<class UTextBlock> ChampionNameText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ChampionSelection", Meta = (BindWidget))
    TObjectPtr<class UTextBlock> ChampionPositionText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ChampionSelection", Meta = (BindWidget))
    TObjectPtr<class UTextBlock> PlayerNameText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ChampionSelection", Meta = (BindWidget))
    TObjectPtr<class UImage> ChampionImage;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ChampionSelection", Meta = (BindWidget))
    TObjectPtr<class UImage> BorderImage;
};
