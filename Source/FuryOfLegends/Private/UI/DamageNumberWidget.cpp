// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/DamageNumberWidget.h"
#include "Components/TextBlock.h"


void UDamageNumberWidget::SetDamageAmount(const float DamageAmount, const FLinearColor InColor, const float TextScale)
{
	FString DamageString = FString::Printf(TEXT("%d"), FMath::CeilToInt(DamageAmount));
	DamageText->SetText(FText::FromString(DamageString));

	DamageText->SetColorAndOpacity(InColor);

	DamageText->SetRenderScale(FVector2D(TextScale));
}

UWidgetAnimation* UDamageNumberWidget::GetFadeOutAnimation() const
{
	return FadeOutAnimation;
}
