// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_AbilityLevel.h"
#include "Components/WidgetSwitcher.h"
#include "Components/StackBox.h"

void UUW_AbilityLevel::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	TArray<UWidget*> widgets;
	widgets = StackBox->GetAllChildren();

	for (auto& widget : widgets)
	{
		UWidgetSwitcher* Switcher = Cast<UWidgetSwitcher>(widget);
		if (::IsValid(Switcher))
		{
			Switchers.AddUnique(Switcher);
		}
	}
}

void UUW_AbilityLevel::NativeConstruct()
{
	Super::NativeConstruct();

}
