// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_ChatText.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UUW_ChatText : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	void SetRichText(const FString& NewText) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ChatText", Meta = (BindWidget))
	TObjectPtr<class URichTextBlock> RichText;
};
