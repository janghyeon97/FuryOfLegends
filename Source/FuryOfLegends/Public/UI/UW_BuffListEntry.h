// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_BuffListEntry.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UUW_BuffListEntry : public UUserWidget
{
	GENERATED_BODY()
	
public:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

    void UpdateImage(UTexture* NewTexure);
    void UpdateBorderColor(FLinearColor NewColor);
    void UpdateCooldownMaskOpacity(const float NewOpacity);
    void UpdateCooldownPercent(const float NewPercent);

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", Meta = (BindWidget))
    TObjectPtr<class UImage> Image;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", Meta = (BindWidget))
    TObjectPtr<class UImage> Border;

private:
    UMaterialInstanceDynamic* MaterialRef = nullptr;
};
