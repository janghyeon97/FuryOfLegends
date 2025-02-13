// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgetBase.h"
#include "TargetStatusWidget.generated.h"

class UUserWidgetBarBase;

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UTargetStatusWidget : public UUserWidgetBase
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void InitializeWidget(class UStatComponent* NewStatComponent);
	void ResetWidget();

protected:
	UPROPERTY()
	TWeakObjectPtr<class UStatComponent> StatComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TargetStatus", Meta = (BindWidget))
	TObjectPtr<class UTextBlock> ObjectName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TargetStatus", Meta = (BindWidget))
	TObjectPtr<class UUserWidgetBarBase> HPBar;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TargetStatus", Meta = (AllowPrivateAccess))
	float MaxHP;
};
