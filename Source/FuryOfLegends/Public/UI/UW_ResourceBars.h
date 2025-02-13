// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_ResourceBars.generated.h"

class UUserWidgetBarBase;
class UStatComponent;

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UUW_ResourceBars : public UUserWidget
{
	GENERATED_BODY()
	
	friend class UUHUD;

protected:
	virtual void NativeOnInitialized() override;
	
public:
	void InitializeWidget(UStatComponent* InStatComponent);
	
private:
	TWeakObjectPtr<UStatComponent> StatComponent;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ResourceBar", Meta = (BindWidget))
	TObjectPtr<UUserWidgetBarBase> HPBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ResourceBar", Meta = (BindWidget))
	TObjectPtr<UUserWidgetBarBase> MPBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ResourceBar", Meta = (BindWidget))
	TObjectPtr<UUserWidgetBarBase> EXPBar;
};
