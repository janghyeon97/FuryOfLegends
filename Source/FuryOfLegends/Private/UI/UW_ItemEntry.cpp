#include "UI/UW_ItemEntry.h"
#include "UI/UW_ItemShop.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"
#include "Components/ScaleBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Game/ArenaPlayerState.h"
#include "Game/ArenaGameState.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

//
// 초기화 관련 함수
//

void UUW_ItemEntry::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	bDisplaySubItems = true;
	ParentNode = nullptr;
	ChildNodes = TArray<UUW_ItemEntry*>();

	MaterialRef = ItemImage->GetDynamicMaterial();
}

void UUW_ItemEntry::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ItemButton)
	{
		UE_LOG(LogTemp, Error, TEXT("[UUW_ItemEntry::NativeConstruct] ItemButton is nullptr! Click event will not bind."));
		return;
	}

	ItemButton->OnClicked.AddDynamic(this, &ThisClass::OnButtonClicked);
	ItemButton->OnHovered.AddDynamic(this, &ThisClass::OnMouseHovered);
	ItemButton->OnUnhovered.AddDynamic(this, &ThisClass::OnMouseUnHovered);
}


//
// 아이템 설정 및 초기화 관련 함수
//

void UUW_ItemEntry::SetupNode(const int32 NewItemCode, UTexture* NewImage, const int32 NewPrice, const int32 NewCount)
{
	ItemCode = NewItemCode;

	if (ItemCode <= 0 || !NewImage)
	{
		SetEmptyNode();
		return;
	}

	if (!MaterialRef)
	{
		InitializeMaterial();
	}
	else
	{
		MaterialRef->SetTextureParameterValue(FName("Texture"), NewImage);
	}

	UpdateItemPrice(NewPrice);
	UpdateItemCount(NewCount);
}

bool UUW_ItemEntry::IsEmptyNode() const
{
	return ItemCode == 0;
}

void UUW_ItemEntry::SetEmptyNode()
{
	ItemImage->SetVisibility(ESlateVisibility::Hidden);
	ItemBorder->SetVisibility(ESlateVisibility::Hidden);
	ItemPrice->SetVisibility(ESlateVisibility::Collapsed);
	ItemCount->SetVisibility(ESlateVisibility::Collapsed);
	ItemButton->SetIsEnabled(false);
}

void UUW_ItemEntry::InitializeMaterial()
{
	if (!::IsValid(ItemImage))
	{
		return;
	}

	MaterialRef = ItemImage->GetDynamicMaterial();
}

//
// UI 업데이트 관련 함수
//

void UUW_ItemEntry::UpdateItemImage(UTexture* NewTexture)
{
	if (!NewTexture)
	{
		return;
	}

	MaterialRef->SetTextureParameterValue(FName("Texture"), NewTexture);
}

void UUW_ItemEntry::UpdateItemPrice(const int32 Price)
{
	if (Price <= 0)
	{
		PriceScaleBox->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	ItemPrice->SetText(FText::AsNumber(Price));
	PriceScaleBox->SetVisibility(ESlateVisibility::Visible);
}

void UUW_ItemEntry::UpdateItemCount(const int32 InCount)
{
	if (InCount <= 0)
	{
		ItemCount->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	ItemCount->SetText(FText::AsNumber(InCount));
	ItemCount->SetVisibility(ESlateVisibility::Visible);
}

void UUW_ItemEntry::UpdateItemCountVisibility(ESlateVisibility InVisibility)
{
	if (ItemCount)
	{
		ItemCount->SetVisibility(InVisibility);
	}
}

void UUW_ItemEntry::UpdatePriceVisibility(ESlateVisibility InVisibility)
{
	if (::IsValid(PriceScaleBox))
	{
		PriceScaleBox->SetVisibility(InVisibility);
	}
}

//
// UI 스타일 및 크기 조정 관련 함수
//

void UUW_ItemEntry::SetItemSelected(bool bIsSelected)
{
	if (ItemBorder)
	{
		ItemBorder->SetVisibility(bIsSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UUW_ItemEntry::SetSize(const float NewSize) const
{
	if (!::IsValid(RootSizeBox))
	{
		return;
	}

	RootSizeBox->SetMinDesiredHeight(NewSize);
	RootSizeBox->SetMinDesiredWidth(NewSize);
	RootSizeBox->SetHeightOverride(NewSize);
	RootSizeBox->SetWidthOverride(NewSize);
}

void UUW_ItemEntry::UpdateBorderColor(FLinearColor NewColor)
{
	if (!ItemBorder)
	{
		UE_LOG(LogTemp, Error, TEXT("Border widget is null in %s"), *GetName());
		return;
	}

	if (ItemBorder->GetColorAndOpacity() != NewColor)
	{
		ItemBorder->SetColorAndOpacity(NewColor);
	}
}

void UUW_ItemEntry::UpdateCountPadding(FMargin NewMargin)
{
	if (!::IsValid(CountScaleBox))
	{
		return;
	}

	if (UVerticalBoxSlot* VerticalSlot = Cast<UVerticalBoxSlot>(CountScaleBox->Slot))
	{
		VerticalSlot->SetPadding(NewMargin);
	}
}

void UUW_ItemEntry::UpdateCountTextSize(const float NewSize)
{
	if (!::IsValid(CountScaleBox))
	{
		return;
	}

	if (UVerticalBoxSlot* VerticalSlot = Cast<UVerticalBoxSlot>(CountScaleBox->Slot))
	{
		FSlateChildSize SlateChildSize;
		SlateChildSize.SizeRule = ESlateSizeRule::Fill;
		SlateChildSize.Value = NewSize;

		VerticalSlot->SetSize(SlateChildSize);
	}
}

//
// 인터랙션 및 이벤트 핸들링 관련 함수
//

void UUW_ItemEntry::OnButtonClicked()
{
	if (!ItemButton)
	{
		UE_LOG(LogTemp, Error, TEXT("[UUW_ItemEntry::OnButtonClicked] ItemButton is nullptr!"));
		return;
	}

	if (!ItemButton->IsVisible())
	{
		UE_LOG(LogTemp, Warning, TEXT("[UUW_ItemEntry::OnButtonClicked] ItemButton is not visible!"));
	}

	if (!ItemShop.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[UUW_ItemEntry::OnButtonClicked] ItemShop reference is invalid"));
		return;
	}

	if (ItemCode <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[UUW_ItemEntry::OnButtonClicked] Invalid ItemCode: %d"), ItemCode);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[UUW_ItemEntry::OnButtonClicked] Button Clicked! ItemCode: %d"), ItemCode);
	ItemShop->DisplayItemDescription(ItemCode);

	if (bDisplaySubItems)
	{
		UE_LOG(LogTemp, Log, TEXT("[UUW_ItemEntry::OnButtonClicked] Displaying item with sub-items."));
		ItemShop->DisplayItemWithSubItems(ItemCode);
		ItemShop->SetSelectedItem(this);
	}
}


void UUW_ItemEntry::OnMouseHovered()
{
	bMouseHovered = true;
}

void UUW_ItemEntry::OnMouseUnHovered()
{
	bMouseHovered = false;
}

FReply UUW_ItemEntry::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FVector2D LocalMousePosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		ItemShop->DisplayItemDescription(ItemCode);

		if (bDisplaySubItems)
		{
			UE_LOG(LogTemp, Log, TEXT("[%s] Displaying item with sub-items."), *GetName());
			ItemShop->DisplayItemWithSubItems(ItemCode);
			ItemShop->SetSelectedItem(this);
		}
		return FReply::Handled();
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton && bCanPurchaseItem)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Purchasing item: ItemCode=%d"), *GetName(), ItemCode);
		ItemShop->PurchaseItem(ItemCode);
		return FReply::Handled();
	}

	return FReply::Handled();
}


//
// 외부 바인딩 관련 함수
//

void UUW_ItemEntry::BindItemShopWidget(UUW_ItemShop* Widget)
{
	if (!Widget)
	{
		return;
	}

	ItemShop = Widget;
}

//
// 머티리얼 관련 업데이트 함수
//

void UUW_ItemEntry::UpdateCooldownMaskOpacity(const float NewOpacity)
{
	if (MaterialRef)
	{
		MaterialRef->SetScalarParameterValue(FName("Param"), NewOpacity);
	}
}

void UUW_ItemEntry::UpdateCooldownPercent(const float NewPercent)
{
	if (!MaterialRef)
	{
		UE_LOG(LogTemp, Error, TEXT("[UUW_ItemEntry] MaterialRef is null in %s"), *GetName());
		return;
	}

	MaterialRef->SetScalarParameterValue(FName("Percent"), NewPercent);
}
