// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Item/ItemData.h"
#include "UW_ItemEntry.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UUW_ItemEntry : public UUserWidget
{
	GENERATED_BODY()

public:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    void SetupNode(const int32 NewItemCode, UTexture* NewIamge, const int32 NewPrice, const int32 NewCount = 0);
    void SetItemSelected(bool bIsSelected);
    void BindItemShopWidget(class UUW_ItemShop* Widget);

    void SetSize(const float NewSize = 70.f) const;

    void UpdateItemImage(UTexture* NewTexure);
    void UpdateItemPrice(const int32 Price);
    void UpdatePriceVisibility(ESlateVisibility InVisibility);
    void UpdateItemCount(const int32 InCount);
    void UpdateItemCountVisibility(ESlateVisibility InVisibility);
    void UpdateDisplaySubItems(bool DisplaySubItems) { bDisplaySubItems = DisplaySubItems; };
    void UpdateCanPurchaseItem(bool CanPurchaseItem) { bCanPurchaseItem = CanPurchaseItem; };

    void UpdateBorderColor(FLinearColor NewColor);
    void UpdateCooldownMaskOpacity(const float NewOpacity);
    void UpdateCooldownPercent(const float NewPercent);
    void UpdateCountPadding(FMargin NewMargin);
    void UpdateCountTextSize(const float NewSize);


    UFUNCTION()
    void OnButtonClicked();

    UFUNCTION()
    void OnMouseHovered();

    UFUNCTION()
    void OnMouseUnHovered();

protected:
    bool IsEmptyNode() const;
    void SetEmptyNode();
    void InitializeMaterial();
	
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item")
    TWeakObjectPtr<class UUW_ItemShop> ItemShop;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", Meta = (BindWidget))
    TObjectPtr<class UImage> ItemImage;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", Meta = (BindWidget))
    TObjectPtr<class UImage> ItemBorder;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", Meta = (BindWidget))
    TObjectPtr<class UTextBlock> ItemPrice;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", Meta = (BindWidget))
    TObjectPtr<class UTextBlock> ItemCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", Meta = (BindWidget))
    TObjectPtr<class UButton> ItemButton;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", Meta = (BindWidget))
    TObjectPtr<class USizeBox> RootSizeBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", Meta = (BindWidget))
    TObjectPtr<class UScaleBox> PriceScaleBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", Meta = (BindWidget))
    TObjectPtr<class UScaleBox> CountScaleBox;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
    UUW_ItemEntry* ParentNode;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
    TArray<UUW_ItemEntry*> ChildNodes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess))
    int32 ItemCode = 0;

private:
    UMaterialInstanceDynamic* MaterialRef = nullptr;

    bool bMouseHovered = false;
    bool bDisplaySubItems = false;
    bool bCanPurchaseItem = false;
};
