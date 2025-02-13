// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LoadingPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API ALoadingPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Loading", Meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> LoadingScreenClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Loading", Meta = (AllowPrivateAccess))
	TObjectPtr<UUserWidget> LoadingScreenWidget;
};
