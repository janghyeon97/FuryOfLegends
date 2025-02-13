// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingScreenUI.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API ULoadingScreenUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	void RemoveLoadingScreen(bool Successful);
};
