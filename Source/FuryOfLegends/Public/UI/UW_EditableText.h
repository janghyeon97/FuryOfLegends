// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_EditableText.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UUW_EditableText : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	FString GetText() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UUW_EditableText", Meta = (BindWidget))
	TObjectPtr<class UEditableText> EditText;
};
