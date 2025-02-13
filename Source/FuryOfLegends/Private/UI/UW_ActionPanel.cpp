// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_ActionPanel.h"
#include "Components/ActionStatComponent.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Blueprint/WidgetTree.h"

void UUW_ActionPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (::IsValid(LevelUpArrow))
	{
		LevelUpArrow->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UUW_ActionPanel::InitializeWidget(UActionStatComponent* InActionStatComponent)
{
	if (::IsValid(InActionStatComponent) == false)
	{
		return;
	}

}

void UUW_ActionPanel::SetActionSlotText(const FString& SlotText)
{
	if (::IsValid(ActionSlotText))
	{
		ActionSlotText->SetText(FText::FromString(SlotText));
	}
}

void UUW_ActionPanel::SetActionMaxLevel(const int32 InActionLevel)
{
	if (::IsValid(ActionLevel) == false)
	{
		return;
	}

	for (int32 i = 0; i < InActionLevel; ++i)
	{
		UBorder* Border = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), FName(*FString::Printf(TEXT("Border_%d"), i)));
		if (Border)
		{
			Border->SetPadding(FMargin(2));
			Border->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
			Border->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
			Border->SetBrushColor(FLinearColor::Gray);
		}
		else
		{
			continue;
		}

		UHorizontalBoxSlot* HorizontalBoxSlot = ActionLevel->AddChildToHorizontalBox(Border);
		if (HorizontalBoxSlot)
		{
			HorizontalBoxSlot->SetSize(ESlateSizeRule::Fill);
			HorizontalBoxSlot->SetPadding(FMargin(0));
			HorizontalBoxSlot->SetHorizontalAlignment(HAlign_Center);
			HorizontalBoxSlot->SetVerticalAlignment(VAlign_Center);
		}
	}
}

void UUW_ActionPanel::SetActionCurrentLevel(const int32 InActionLevel)
{
	if (::IsValid(ActionLevel) == false)
	{
		return;
	}

	if (ActionLevel->GetChildrenCount() < InActionLevel)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid InActionLevel."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	for (int32 i = 0; i < InActionLevel; ++i)
	{
		UBorder* Border = Cast<UBorder>(ActionLevel->GetChildAt(i));
		if (Border)
		{
			Border->SetVisibility(ESlateVisibility::Visible);
			Border->SetBrushColor(FLinearColor::Yellow);
		}
	}
}

void UUW_ActionPanel::SetCooldownTime(float CurrentCooldownTime, float MaxCooldownTime)
{
	if (!IsValid(CooldownTimeText))
	{
		return;
	}

	UMaterialInstanceDynamic* DynamicMaterial = ActionImage->GetDynamicMaterial();
	if (!DynamicMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] DynamicMaterial is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	FString CooldownTimeString = (CurrentCooldownTime <= 1.0f)
			? FString::Printf(TEXT("%.1f"), CurrentCooldownTime)
			: FString::Printf(TEXT("%d"), static_cast<int32>(CurrentCooldownTime));

	ESlateVisibility TextVisibility = (CurrentCooldownTime >= 0.1f) ? ESlateVisibility::Visible : ESlateVisibility::Hidden;
	const float Ratio = (CurrentCooldownTime > 0) ? (1 - CurrentCooldownTime / MaxCooldownTime) : 1;

	CooldownTimeText->SetVisibility(TextVisibility);
	CooldownTimeText->SetText(FText::FromString(CooldownTimeString));
	DynamicMaterial->SetScalarParameterValue(FName("Percent"), Ratio);
}


void UUW_ActionPanel::SetActionImage(UTexture* InActionImage)
{
	if (::IsValid(InActionImage) == false || ::IsValid(ActionImage) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid ActionImage or InActionImage."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	UMaterialInstanceDynamic* DynamicMaterial = ActionImage->GetDynamicMaterial();
	if (DynamicMaterial == nullptr)
	{
		return;
	}

	DynamicMaterial->SetTextureParameterValue(FName("Texture"), InActionImage);
}

void UUW_ActionPanel::SetLevelUpArrowVisibility(const bool bIsVisible)
{
	if (::IsValid(LevelUpArrow))
	{
		LevelUpArrow->SetVisibility(bIsVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UUW_ActionPanel::SetActionActivation(const bool bIsVisible)
{
	if (::IsValid(ActionIconShade))
	{
		ActionIconShade->SetVisibility(bIsVisible ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
	}
}

