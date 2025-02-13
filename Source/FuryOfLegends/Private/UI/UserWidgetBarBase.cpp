// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UserWidgetBarBase.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"

UUserWidgetBarBase::UUserWidgetBarBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer) 
{
    MaxFigure = 0.f;
    CurrentFigure = 0.f;
}


void UUserWidgetBarBase::InitializeWidget(const float InCurrentFigure, const float InMaxFigure, const float InRegeneration)
{
    SetMaxFigure(InMaxFigure);
    SetCurrentFigure(InCurrentFigure);
    OnRegenerationChanged(0, InRegeneration);

    if (::IsValid(Bar) == false)
    {
        return;
    }

    if (KINDA_SMALL_NUMBER < MaxFigure)
    {
        Bar->SetPercent(CurrentFigure / MaxFigure);
    }
    else
    {
        Bar->SetPercent(0.f);
    }
}


void UUserWidgetBarBase::SetMaxFigure(float InMaxFigure)
{
    if (InMaxFigure < KINDA_SMALL_NUMBER)
    {
        MaxFigure = 0.f;
        return;
    }

    MaxFigure = InMaxFigure;
}

void UUserWidgetBarBase::SetCurrentFigure(float InCurrentFigure)
{
    if (InCurrentFigure < KINDA_SMALL_NUMBER)
    {
        CurrentFigure = 0.f;
        return;
    }

    CurrentFigure = InCurrentFigure;
}


void UUserWidgetBarBase::OnMaxMaxFigureChanged(float PreviousMaxigure, float NewMaxigure)
{
    SetMaxFigure(NewMaxigure);

    if (::IsValid(Bar) == false)
    {
        return;
    }

    if (KINDA_SMALL_NUMBER < MaxFigure)
    {
        Bar->SetPercent(CurrentFigure / MaxFigure);
    }
    else
    {
        Bar->SetPercent(0.f);
    }

    FString StatString = FString::Printf(TEXT("%d"), FMath::CeilToInt(NewMaxigure));
    MaxFigureText->SetText(FText::FromString(StatString));
}

void UUserWidgetBarBase::OnCurrentFigureChanged(float PreviousCurrentFigure, float NewCurrentFigure)
{
    SetCurrentFigure(NewCurrentFigure);

    if (::IsValid(Bar) == false)
    {
        return;
    }

    if (KINDA_SMALL_NUMBER < MaxFigure)
    {
        Bar->SetPercent(CurrentFigure / MaxFigure);
    }
    else
    {
        Bar->SetPercent(0.f);
    }

    FString StatString = FString::Printf(TEXT("%d"), FMath::CeilToInt(NewCurrentFigure));
    CurrentFigureText->SetText(FText::FromString(StatString));
}


void UUserWidgetBarBase::OnRegenerationChanged(float PreviousFigure, float NewFigure)
{
    FString StatString = NewFigure > 0 ?
        FString::Printf(TEXT("+%d"), FMath::CeilToInt(NewFigure)) :
        FString::Printf(TEXT("-%d"), FMath::CeilToInt(NewFigure));

    RegenerationText->SetText(FText::FromString(StatString));
}


void UUserWidgetBarBase::SetBorderColor(FLinearColor InColor)
{
    if (!Border)
    {
        UE_LOG(LogTemp, Error, TEXT("Border widget is null in %s"), *GetName());
        return;
    }

    // 현재 색상과 새로 설정할 색상이 다를 경우에만 업데이트
    if (Border->GetBrushColor() != InColor)
    {
        Border->SetBrushColor(InColor);
    }
}

void UUserWidgetBarBase::SetProgressBarColor(FLinearColor InColor)
{
    if (!Bar)
    {
        UE_LOG(LogTemp, Error, TEXT("Progress bar widget is null in %s"), *GetName());
        return;
    }

    // 현재 색상과 새로 설정할 색상이 다를 경우에만 업데이트
    if (Bar->GetFillColorAndOpacity() != InColor)
    {
        Bar->SetFillColorAndOpacity(InColor);
    }
}

void UUserWidgetBarBase::SetTextVisibility(ESlateVisibility InVisibility)
{
    if (!CurrentFigureText || !MaxFigureText || !RegenerationText)
    {
        return;
    }

    CurrentFigureText->SetVisibility(InVisibility);
    MaxFigureText->SetVisibility(InVisibility);
    RegenerationText->SetVisibility(InVisibility);
    SlashIcon->SetVisibility(InVisibility);
}


void UUserWidgetBarBase::NativeConstruct()
{
    Super::NativeConstruct();

}
