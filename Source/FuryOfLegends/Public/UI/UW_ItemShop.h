#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgetBase.h"
#include "Item/ItemData.h"
#include "UW_ItemShop.generated.h"

class AArenaGameState;
class AArenaPlayerState;
class UStatComponent;
class UUW_ItemEntry;
class UUniformGridPanel;
class UStackBox;
class UImage;
class UTextBlock;
class URichTextBlock;
class UStackBoxSlot;
class UVerticalBox;
class UHorizontalBox;
class UHorizontalBoxSlot;

/**
 *
 */
UCLASS()
class FURYOFLEGENDS_API UUW_ItemShop : public UUserWidgetBase
{
	GENERATED_BODY()

public:
	// Constructor
	UUW_ItemShop(const FObjectInitializer& ObjectInitializer);

	// Overridden functions
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// Public functions
	void InitializeItemList();
	void BindPlayerState(AArenaPlayerState* InPlayerState);
	void PurchaseItem(const int32 ItemCode);
	void DisplayItemWithSubItems(int32 ItemCode);
	void DisplayItemDescription(int32 ItemCode);
	void SetSelectedItem(UUW_ItemEntry* NewSelectedItem);
	void PlaySound(USoundBase* Sound);
	//void AdjustAndDisplayItemPrice(UUW_ItemEntry* Entry, FItemTableRow* ItemInfo);
	//void ApplyDiscount(FItemTableRow* ItemInfo, int32& FinalPrice, TArray<int32>& PlayerInventoryItemCodes);

	TWeakObjectPtr<AArenaGameState> GetGameState() { return GameState; }

	UFUNCTION()
	void OnItemPurchased(int32 ItemCode, bool bSucessful);

	// Private functions
	UHorizontalBox* CreateRootHorizontalBox();
	UUniformGridPanel* GetOrCreateClassificationPanel(const FString& ClassificationString);

	void SetupStackBoxSlot(UStackBoxSlot* NewSlot, ESlateSizeRule::Type SizeRule, EHorizontalAlignment HorizontalAlignment, EVerticalAlignment VerticalAlignment);
	void SetupHorizontalBoxSlot(UHorizontalBoxSlot* NewSlot, ESlateSizeRule::Type SizeRule, EHorizontalAlignment HorizontalAlignment, EVerticalAlignment VerticalAlignment);

	void AddClassifiedItemEntry(const FItemTableRow& Item);
	void AddNodesBreadthFirst(UUW_ItemEntry* RootNode);
	void AddChildNodesRecursively(UUW_ItemEntry* ParentNode, UHorizontalBox* ParentBox, int32 Depth);
	void AddItemDescription(const FItemTableRow& ItemInfo, UStatComponent* StatComponent);
	void AddTopLevelBoxToItemHierarchy(UHorizontalBox* TopLevelBox);
	void AddNodeToStackBox(UStackBox* ParentBox, UWidget* Node, ESlateSizeRule::Type SizeRule, EHorizontalAlignment HorizontalAlignment, EVerticalAlignment VerticalAlignment);
	void AddNodeToHorizontalBox(UHorizontalBox* ParentBox, UWidget* Node, ESlateSizeRule::Type SizeRule, EHorizontalAlignment HorizontalAlignment, EVerticalAlignment VerticalAlignment);

	UUW_ItemEntry* CreateNodeHierarchy(const FItemTableRow& NodeInfo);
	UUW_ItemEntry* FindLoadedItemByID(int32 ItemCode) const;
	UUW_ItemEntry* CreateCachedNode(const FItemTableRow& NodeInfo);
	UUW_ItemEntry* GetOrCreateItemHierarchy(const FItemTableRow& ItemInfo);
	FItemTableRow* GetItemInfoByID(int32 ItemCode);

protected:
	// Properties
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ItemShop", Meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> ItemListEntryClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ItemShop", Meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> TreeNodeWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ItemShop", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> ItemDescriptionLineClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemShop", Meta = (BindWidget))
	TObjectPtr<UStackBox> ItemList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemShop", Meta = (BindWidget))
	TObjectPtr<UStackBox> ItemHierarchyBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemShop", Meta = (BindWidget))
	TObjectPtr<UStackBox> ItemDescriptionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemShop", Meta = (BindWidget))
	TObjectPtr<UImage> ItemImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemShop", Meta = (BindWidget))
	TObjectPtr<UImage> CoinImage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemShop", Meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemShop", Meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemPrice;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ItemShop")
	UDataTable* RichTextStyleSet;

private:
	// Private member variables
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "ItemShop", meta = (AllowPrivateAccess))
	TArray<TObjectPtr<UUW_ItemEntry>> LoadedItems;

	TWeakObjectPtr<AArenaGameState> GameState;
	TWeakObjectPtr<AArenaPlayerState> PlayerState;
	TWeakObjectPtr<UUW_ItemEntry> SelectedItem;

	TMap<FString, UUniformGridPanel*> ClassificationPanels;
	TMap<FString, int32> ClassificationRows;
	TMap<FString, int32> ClassificationColumns;

	TMap<int32, TArray<TWeakObjectPtr<UUW_ItemEntry>>> ItemHierarchyCache;
	TMap<int32, TArray<class UUW_ItemDescriptionLine*>> ItemDescriptionCache;

	UUW_ItemEntry* DefaultEmptyNode = nullptr;
	UMaterialInstanceDynamic* ItemImageRef = nullptr;

	const int32 MaxColumn = 10;
	int32 BoxIndex = 0;
	int32 NodeIDCounter;
};
