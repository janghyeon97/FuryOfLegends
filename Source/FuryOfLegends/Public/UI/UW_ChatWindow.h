// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_ChatWindow.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UUW_ChatWindow : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	void UpdateMessageLog(const FString& NewText);

	UFUNCTION()
	void OnEditableTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ChatText", Meta = (AllowPrivateAccess))
	TSubclassOf<class UUserWidget> ChatTextClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ChatText")
	TWeakObjectPtr<class ALobbyPlayerController> LobbyPlayerController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ChatText", Meta = (BindWidget))
	TObjectPtr<class UScrollBox> ChatScrollBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ChatText", Meta = (BindWidget))
	TObjectPtr<class UUW_EditableText> EditableText;

	FLinearColor SenderColor = FLinearColor::White;

	FLinearColor MessageColor = FLinearColor::White;
};
