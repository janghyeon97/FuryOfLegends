// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UW_ItemDescriptionLine.h"
#include "Components/RichTextBlock.h"

UUW_ItemDescriptionLine::UUW_ItemDescriptionLine(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    static ConstructorHelpers::FObjectFinder<UDataTable> DataTable(TEXT("/Game/FuryOfLegends/UI/DT_RichTextStyles.DT_RichTextStyles"));
    if (DataTable.Succeeded())
    {
        RichTextStyleSet = DataTable.Object;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[UUW_ItemDescriptionLine::UUW_ItemDescriptionLine] Failed to load data table: /Game/FuryOfLegends/UI/DT_RichTextStyles.DT_RichTextStyles"));
    }
}

void UUW_ItemDescriptionLine::NativeConstruct()
{
    Super::NativeConstruct();

    if (RichTextStyleSet && DescriptionRichTextBlock)
    {
        DescriptionRichTextBlock->SetTextStyleSet(RichTextStyleSet);
    }
}

void UUW_ItemDescriptionLine::SetDescriptionText(const FString& Text)
{
    if (DescriptionRichTextBlock)
    {
        DescriptionRichTextBlock->SetText(FText::FromString(Text));
        DescriptionRichTextBlock->SetAutoWrapText(true);
        DescriptionRichTextBlock->SetWrapTextAt(0);
        DescriptionRichTextBlock->SetWrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping);

        if (RichTextStyleSet)
        {
            DescriptionRichTextBlock->SetTextStyleSet(RichTextStyleSet);
        }

    }
}
