// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_Inventory.h"
#include "UI/UW_ItemEntry.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"


void UUW_Inventory::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ItemImages.SetNum(InventorySize);
	ItemButtons.SetNum(InventorySize);
	ItemCounts.SetNum(InventorySize);
	ItemEnties.SetNum(InventorySize);

	if (!DefaultTexture)
	{
		DefaultTexture = Cast<UTexture>(StaticLoadObject(UTexture::StaticClass(), NULL,
			TEXT("/Game/FuryOfLegends/UI/Texture/Image_Transparent.Image_Transparent")));
	}
}

void UUW_Inventory::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeWidgets();
}

void UUW_Inventory::InitializeWidgets()
{
	FString EntryName;
	for (int32 i = 0; i < InventorySize; ++i)
	{
		EntryName = FString::Printf(TEXT("Entry_%d"), i + 1);
		UUW_ItemEntry* FoundEntry = Cast<UUW_ItemEntry>(GetWidgetFromName(FName(*EntryName)));
		if (!FoundEntry)
		{
			UE_LOG(LogTemp, Warning, TEXT("Widget %s not found!"), *EntryName);
			continue;
		}

		if (!ItemEnties.IsValidIndex(i))
		{
			UE_LOG(LogTemp, Error, TEXT("ItemEnties index %d is out of bounds!"), i);
			continue;
		}

		ItemEnties[i] = FoundEntry;
		if (ItemEnties[i])
		{
			ItemEnties[i]->UpdateDisplaySubItems(false);
			ItemEnties[i]->UpdateCanPurchaseItem(false);
			ItemEnties[i]->UpdatePriceVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UUW_Inventory::UpdateItem(int32 Index, int32 ItemCode)
{
	if (ItemCode == 0 && ItemCodeToIndexMap.Remove(ItemCode) > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Removed ItemCode %d"), ItemCode);
		return;
	}

	int32* ExistingIndex = ItemCodeToIndexMap.Find(ItemCode);
	if (ExistingIndex)
	{
		if (*ExistingIndex != Index)
		{
			*ExistingIndex = Index;
			UE_LOG(LogTemp, Log, TEXT("Updated ItemCode %d from Index %d to %d"), ItemCode, *ExistingIndex, Index);
		}
		return;
	}

	ItemCodeToIndexMap.Add(ItemCode, Index);
	UE_LOG(LogTemp, Log, TEXT("Added ItemCode %d at Index %d"), ItemCode, Index);
}


void UUW_Inventory::UpdateItemImage(UTexture* NewTexture, int32 Index)
{
	if (!ItemEnties.IsValidIndex(Index) || !ItemEnties[Index])
	{
		return;
	}

	// NewTexture가 nullptr이면 DefaultTexture 사용
	UTexture* TextureToApply = NewTexture ? NewTexture : DefaultTexture;
	ItemEnties[Index]->UpdateItemImage(TextureToApply);
}


void UUW_Inventory::UpdateItemCountText(int32 Count, int32 Index)
{
	if (!ItemEnties.IsValidIndex(Index) || !ItemEnties[Index])
	{
		return;
	}

	ItemEnties[Index]->UpdateItemCount(Count);
}

void UUW_Inventory::UpdateCurrencyText(FText NewText)
{
	if (!CurrencyText)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] CurrencyText widget is nullptr! Cannot update currency display."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (NewText.IsEmptyOrWhitespace())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Received empty or whitespace currency text. Update skipped."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	CurrencyText->SetText(NewText);
}

void UUW_Inventory::UpdateCooldownRatio(const float NewRatio, int32 Index)
{
	if (!ItemEnties.IsValidIndex(Index) || !ItemEnties[Index])
	{
		return;
	}

	ItemEnties[Index]->UpdateCooldownPercent(NewRatio);
}


