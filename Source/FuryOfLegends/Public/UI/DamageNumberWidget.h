// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgetBase.h"
#include "DamageNumberWidget.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UDamageNumberWidget : public UUserWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(Category = "Damage")
	void SetDamageAmount(const float DamageAmount, const FLinearColor InColor, const float TextScale);

	// FadeOut 애니메이션 반환 함수
	UFUNCTION(BlueprintCallable, Category = "Damage")
	UWidgetAnimation* GetFadeOutAnimation() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Damage", meta = (BindWidget))
	TObjectPtr<class UTextBlock> DamageText;

	UPROPERTY(Transient, EditAnywhere, BlueprintReadOnly, Category = "Damage", meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> FadeOutAnimation;
};
