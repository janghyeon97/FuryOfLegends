// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_ChampionListEntry.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UUW_ChampionListEntry : public UUserWidget
{
	GENERATED_BODY()
	
public:
    virtual void NativeConstruct() override;

    void InitializeListEntry();
    void PlaySelectSound();

    void UpdateChampionIndex(int32 Index);
    void UpdateChampionNameText(const FName& InString);
    void UpdateChampionNameColor(FLinearColor InColor);
    void UpdateBorderImageColor(FLinearColor InColor);
    void UpdateChampionImage(UTexture* InTexture);

    UMaterialInstanceDynamic* MaterialRef = nullptr;

    UFUNCTION(BlueprintCallable)
    int32 GetChampionIndex() const { return ChampionIndex; };

    UFUNCTION(BlueprintCallable)
    FName GetChampionName() const { return ChampionName; };

    UFUNCTION()
    void OnButtonClicked();

public:
    

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ChampionListEntry", Meta = (BindWidget))
    TObjectPtr<class UTextBlock> ChampionNameText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ChampionListEntry", Meta = (BindWidget))
    TObjectPtr<class UImage> ChampionImage;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ChampionListEntry", Meta = (BindWidget))
    TObjectPtr<class UImage> BorderImage;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ChampionListEntry", Meta = (BindWidget))
    TObjectPtr<class UButton> Button;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* ChampionSelectSound;

    FName ChampionName;
    int32 ChampionIndex;
};
