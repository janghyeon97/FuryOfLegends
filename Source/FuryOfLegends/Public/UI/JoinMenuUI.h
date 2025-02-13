// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Structs/SessionInfomation.h"
#include "JoinMenuUI.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UJoinMenuUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void FindSession();

	UFUNCTION()
	void AddEntryServerList(const TArray<USessionInfomation*>& SessionResults, bool Successful);

	UFUNCTION()
	void RemoveJoinMenu();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UJoinMenuUI", Meta = (BindWidget))
	TObjectPtr<class UListView> ServerList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UJoinMenuUI", Meta = (BindWidget))
	TObjectPtr<class UButton> RefreshButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UJoinMenuUI", Meta = (BindWidget))
	TObjectPtr<class UButton> CancelButton;
};
