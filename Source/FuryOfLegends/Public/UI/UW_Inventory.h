#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_Inventory.generated.h"

/**
 *
 */
UCLASS()
class FURYOFLEGENDS_API UUW_Inventory : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

    void UpdateItem(int32 Index, int32 ItemCode);
    void UpdateItemImage(UTexture* NewTexture, int32 Index);
    void UpdateItemCountText(int32 Count, int32 Index);
    void UpdateCurrencyText(FText NewText);
    void UpdateCooldownRatio(const float NewRatio, int32 Index);

    TMap<int32, int32> ItemCodeToIndexMap; // ItemCode, Index

protected:
    void InitializeWidgets();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 InventorySize = 6;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Image")
    TArray<TObjectPtr<class UImage>> ItemImages;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Button")
    TArray<TObjectPtr<class UButton>> ItemButtons;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Count")
    TArray<TObjectPtr<class UTextBlock>> ItemCounts;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Count")
    TArray<TObjectPtr<class UUW_ItemEntry>> ItemEnties;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Text", Meta = (BindWidget))
    TObjectPtr<class UTextBlock> CurrencyText;

    UPROPERTY()
    TArray<UMaterialInstanceDynamic*> MaterialRef;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    UTexture* DefaultTexture;
};
