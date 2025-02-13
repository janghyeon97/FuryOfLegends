// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UIPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API AUIPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AUIPlayerController();

	virtual void BeginPlay() override;

	//FOnlineSessionSearchResult* GetSessionResult() const { return SessionResult; };

	//void SetSessionResult(FOnlineSessionSearchResult& InSessionResult) { SessionResult = &InSessionResult; };

	void JoinSession(FString InputPassword);

	void ShowMainMenu();

	void ShowHostMenu();

	void ShowJoinMenu();

	void ShowLoadingScreen();

	void ShowPasswordMenu();

	void ShowConfirmationQuitMenu();

	class USessionInfomation* SelectedSession = nullptr;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AUIPlayerController", Meta = (AllowPrivateAccess))
	TSubclassOf<class UUserWidget> MainMenuClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "AUIPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class UUserWidget> MainMenuInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AUIPlayerController", Meta = (AllowPrivateAccess))
	TSubclassOf<class UUserWidget> ConfirmationQuitClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "AUIPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class UUserWidget> ConfirmationQuitInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AUIPlayerController", Meta = (AllowPrivateAccess))
	TSubclassOf<class UUserWidget> HostMenuClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "AUIPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class UUserWidget> HostMenuInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AUIPlayerController", Meta = (AllowPrivateAccess))
	TSubclassOf<class UUserWidget> JoinMenuClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "AUIPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class UUserWidget> JoinMenuInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AUIPlayerController", Meta = (AllowPrivateAccess))
	TSubclassOf<class UUserWidget> LoadingScreenClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "AUIPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class UUserWidget> LoadingScreenInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AUIPlayerController", Meta = (AllowPrivateAccess))
	TSubclassOf<class UUserWidget> PasswordMenuClass;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "AUIPlayerController", Meta = (AllowPrivateAccess))
	TObjectPtr<class UPasswordMenuUI> PasswordMenuInstance;
};
