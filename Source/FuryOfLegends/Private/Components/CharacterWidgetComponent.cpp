// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CharacterWidgetComponent.h"
#include "UI/UserWidgetBase.h"

void UCharacterWidgetComponent::InitWidget()
{
    Super::InitWidget();

    UUserWidgetBase* UserWidget = Cast<UUserWidgetBase>(GetWidget());
    if (::IsValid(UserWidget) == false)
    {
        return;
    }

    AActor* OwnerActor = GetOwner();
    if (::IsValid(OwnerActor))
    {
        UserWidget->SetOwningActor(OwnerActor);
    }
}
