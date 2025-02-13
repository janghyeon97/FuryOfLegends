// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_AbilityLevel.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UUW_AbilityLevel : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UUW_AbilityLevel")
	TArray<class UWidgetSwitcher*> Switchers;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UUW_AbilityLevel", Meta = (BindWidget))
	TObjectPtr<class UStackBox> StackBox;
};
