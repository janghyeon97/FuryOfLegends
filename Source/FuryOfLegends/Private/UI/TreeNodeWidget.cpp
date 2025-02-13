
// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TreeNodeWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"


void UTreeNodeWidget::NativeOnInitialized()
{
	MaterialRef = ItemImage->GetDynamicMaterial();

	ItemInfo = FItemTableRow();
	ParentNode = nullptr;
	ChildNodes = TArray<UTreeNodeWidget*>();
}

void UTreeNodeWidget::SetupNode(const FItemTableRow& NodeInfo)
{
	ItemInfo = NodeInfo;

	if (IsEmptyNode())
	{
		ItemImage->SetVisibility(ESlateVisibility::Hidden);
		ItemBorder->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		ItemImage->SetVisibility(ESlateVisibility::Visible);
		ItemBorder->SetVisibility(ESlateVisibility::Visible);

		MaterialRef->SetTextureParameterValue("Texture", NodeInfo.Icon);
	}
}

bool UTreeNodeWidget::IsEmptyNode() const
{
	return ItemInfo.Name.IsEmpty();
}