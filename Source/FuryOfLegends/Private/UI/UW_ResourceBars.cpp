// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_ResourceBars.h"
#include "UI/UserWidgetBarBase.h"
#include "Components/StatComponent.h"


void UUW_ResourceBars::NativeOnInitialized()
{
	Super::NativeOnInitialized();


}


void UUW_ResourceBars::InitializeWidget(UStatComponent* InStatComponent)
{
	if (::IsValid(InStatComponent) == false)
	{
		return;
	}

	StatComponent = InStatComponent;
	StatComponent->OnCurrentHPChanged.AddDynamic(HPBar, &UUserWidgetBarBase::OnCurrentFigureChanged);
	StatComponent->OnMaxHPChanged.AddDynamic(HPBar, &UUserWidgetBarBase::OnMaxMaxFigureChanged);

	StatComponent->OnCurrentMPChanged.AddDynamic(MPBar, &UUserWidgetBarBase::OnCurrentFigureChanged);
	StatComponent->OnMaxMPChanged.AddDynamic(MPBar, &UUserWidgetBarBase::OnMaxMaxFigureChanged);

	StatComponent->OnCurrentEXPChanged.AddDynamic(EXPBar, &UUserWidgetBarBase::OnCurrentFigureChanged);
	StatComponent->OnMaxEXPChanged.AddDynamic(EXPBar, &UUserWidgetBarBase::OnMaxMaxFigureChanged);


	float MaxFigure = 0.f;
	float CurrentFigure = 0.f;
	float Regeneration = 0.f;

	MaxFigure = StatComponent->GetMaxHP();
	CurrentFigure = StatComponent->GetCurrentHP();
	Regeneration = StatComponent->GetHealthRegeneration();

	HPBar->InitializeWidget(MaxFigure, CurrentFigure, Regeneration);
	HPBar->SetTextVisibility(ESlateVisibility::HitTestInvisible);

	MaxFigure = StatComponent->GetMaxMP();
	CurrentFigure = StatComponent->GetCurrentMP();
	Regeneration = StatComponent->GetManaRegeneration();

	MPBar->InitializeWidget(MaxFigure, CurrentFigure, Regeneration);
	MPBar->SetTextVisibility(ESlateVisibility::HitTestInvisible);

	EXPBar->InitializeWidget(0, 0, 0);
	EXPBar->SetTextVisibility(ESlateVisibility::Collapsed);
}