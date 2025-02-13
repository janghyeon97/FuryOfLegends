// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_ChatWindow.h"
#include "UI/UW_ChatText.h"
#include "UI/UW_EditableText.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/EditableText.h"
#include "Components/RichTextBlock.h"
#include "Controllers/LobbyPlayerController.h"
#include "Kismet/GameplayStatics.h"

void UUW_ChatWindow::NativeConstruct()
{
	Super::NativeConstruct();

	UWorld* World = GetWorld();
	if (::IsValid(World) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[UUW_ChatWindow::NativeConstruct] WorldContext is not valid."));
		return;
	}

	LobbyPlayerController = Cast<ALobbyPlayerController>(UGameplayStatics::GetPlayerController(World, 0));
	LobbyPlayerController->BindChatWindow(this);

	EditableText->EditText->OnTextCommitted.AddDynamic(this, &UUW_ChatWindow::OnEditableTextCommitted);
}

void UUW_ChatWindow::UpdateMessageLog(const FString& NewText)
{
	UUW_ChatText* text = CreateWidget<UUW_ChatText>(ChatScrollBox, ChatTextClass);

	if (::IsValid(text))
	{
		text->SetRichText(NewText);
		ChatScrollBox->AddChild(text);
		ChatScrollBox->ScrollToEnd();
	}
}

void UUW_ChatWindow::OnEditableTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit)
{
	if (InTextCommit == ETextCommit::OnEnter)
	{
		LobbyPlayerController->UpdateChatLog_Server(NewText.ToString(), LobbyPlayerController.Get());

		FString str = "";
		EditableText->EditText->SetText(FText::FromString(str));

		EditableText->EditText->SetKeyboardFocus();
	}
}