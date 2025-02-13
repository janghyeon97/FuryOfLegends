#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgetBase.h"
#include "Item/ItemData.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "TreeNodeWidget.generated.h"

/**
 *
 */
UCLASS()
class FURYOFLEGENDS_API UTreeNodeWidget : public UUserWidgetBase
{
    GENERATED_BODY()

public:
    virtual void NativeOnInitialized() override;

    void SetupNode(const FItemTableRow& NodeInfo);
    FItemTableRow GetNode() const { return ItemInfo; };

public:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UImage> ItemImage;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UImage> ItemBorder;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UButton> ItemButton;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tree")
    FItemTableRow ItemInfo;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tree")
    UTreeNodeWidget* ParentNode;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Tree")
    TArray<UTreeNodeWidget*> ChildNodes;

private:
    UMaterialInstanceDynamic* MaterialRef = nullptr;

    bool IsEmptyNode() const;
};
