// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_ChatText.h"
#include "Components/RichTextBlock.h"

void UUW_ChatText::NativeConstruct()
{
	Super::NativeConstruct();
}

void UUW_ChatText::SetRichText(const FString& NewText) const
{
	if (RichText)
	{
		RichText->SetText(FText::FromString(NewText));
	}
}
