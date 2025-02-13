// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/TargetStatusWidget.h"
#include "UI/UserWidgetBarBase.h"
#include "Components/StatComponent.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UTargetStatusWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UTargetStatusWidget::InitializeWidget(UStatComponent* NewStatComponent)
{
    if (!::IsValid(NewStatComponent))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UUW_StateBar::InitializeStateBar] Invalid StatComponent"));
        return;
    }

    StatComponent = NewStatComponent;

    if (StatComponent.IsValid())
    {
        StatComponent->OnMaxHPChanged.AddDynamic(HPBar, &UUserWidgetBarBase::OnMaxMaxFigureChanged);
        StatComponent->OnCurrentHPChanged.AddDynamic(HPBar, &UUserWidgetBarBase::OnCurrentFigureChanged);

        HPBar->InitializeWidget(StatComponent->GetMaxHP(), StatComponent->GetCurrentHP(), 0);
        HPBar->SetTextVisibility(ESlateVisibility::Hidden);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to bind StatComponent: StatComponent is invalid."), ANSI_TO_TCHAR(__FUNCTION__));
    }
}

void UTargetStatusWidget::ResetWidget()
{
    if (StatComponent.IsValid())
    {
        StatComponent->OnMaxHPChanged.RemoveDynamic(HPBar, &UUserWidgetBarBase::OnMaxMaxFigureChanged);
        StatComponent->OnCurrentHPChanged.RemoveDynamic(HPBar, &UUserWidgetBarBase::OnCurrentFigureChanged);

        StatComponent = nullptr;

        HPBar->InitializeWidget(0.0f, 0.0f, 0);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] StatComponent was not valid during reset."), ANSI_TO_TCHAR(__FUNCTION__));
    }
}