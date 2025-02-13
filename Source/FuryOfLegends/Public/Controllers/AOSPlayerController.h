// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AOSPlayerController.generated.h"


class UUHUD;
class UTargetStatusWidget;
class UUW_ItemShop;
class UUW_StateBar;
class UUserWidget;
class UStatComponent;
class UCameraComponent;
class UMatchResultUI;

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API AAOSPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AAOSPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void PreInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_PlayerState() override;
	virtual void OnPossess(APawn* InPawn) override;

public:
	UUHUD* GetHUDWidget() const { return HUDWidget; }
	UTargetStatusWidget* GetTargetStatusWidget() const { return TargetStatusWidget; }

	void DisplayMouseCursor(bool bShowCursor);

private:
	void CreateHUD();
	void CreateTargetStatusWidget();

public:
	UFUNCTION(Client, Reliable)
	void ClientTravelToTitle();

	UFUNCTION(Client, Reliable)
	void ClientHandleGameEnd(ANexus* Nexus, bool bIsVictorious);

	UFUNCTION(Client, Reliable)
	void ClientBindTargetStatusWidget(UStatComponent* ObjectStatComponent);

	UFUNCTION(Client, Reliable)
	void ClientRemoveTargetStatusWidget();

	UFUNCTION(Client, Reliable)
	void DisplayCrosshair();

	UFUNCTION(Client, Reliable)
	void ShowLoadingScreen();

	UFUNCTION(Client, Reliable)
	void RemoveLoadingScreen();

	UFUNCTION(Client, Reliable)
	void InitializeHUD(const FName& InChampionName);

	UFUNCTION(Client, Reliable)
	void InitializeItemShop();

	UFUNCTION(Server, Reliable)
	void ServerNotifyLoaded();

	UFUNCTION(Server, Reliable)
	void ServerNotifyCharacterSpawned();

	UFUNCTION(Client, Reliable)
	void ToggleItemShopVisibility();
	
	UFUNCTION(Server, Reliable)
	void OnHUDBindingComplete();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD", Meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> CrosshairClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD", Meta = (AllowPrivateAccess))
	TSubclassOf<UUHUD> HUDClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD", Meta = (AllowPrivateAccess))
	TSubclassOf<UTargetStatusWidget> TargetStatusClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD", Meta = (AllowPrivateAccess))
	TSubclassOf<UUW_ItemShop> ItemShopClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD", Meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> LoadingScreenClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HUD", Meta = (AllowPrivateAccess))
	TSubclassOf<UMatchResultUI> MatchResultUIClass;


private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "HUD", Meta = (AllowPrivateAccess))
	TObjectPtr<UUserWidget> LoadingScreenWidget;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "HUD", Meta = (AllowPrivateAccess))
	TObjectPtr<UUserWidget> CrosshairWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", Meta = (AllowPrivateAccess))
	TObjectPtr<UUHUD> HUDWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", Meta = (AllowPrivateAccess))
	TObjectPtr<UTargetStatusWidget> TargetStatusWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", Meta = (AllowPrivateAccess))
	TObjectPtr<UUW_ItemShop> ItemShopWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", Meta = (AllowPrivateAccess))
	TObjectPtr<UUW_StateBar> StateBarWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HUD", Meta = (AllowPrivateAccess))
	TObjectPtr<UMatchResultUI> MatchResultWidet;

	bool bItemShopVisibility = false;
};
