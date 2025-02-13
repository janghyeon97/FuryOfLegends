// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_StatPanel.generated.h"


class UTextBlock;
class UStatComponent;


/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UUW_StatPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;

	void InitializeWidget(UStatComponent* InStatComponent);

public:
	UFUNCTION()
	void UpdateAttackDamageText(float PreviousAttackDamage, float NewAttackDamage);

	UFUNCTION()
	void UpdateAbilityPowerText(float PreviousAbilityPower, float NewAbilityPower);

	UFUNCTION()
	void UpdateDefensePowerText(float PreviousDefensePower, float NewDefensePower);

	UFUNCTION()
	void UpdateMagicResistanceText(float PreviousMagicResistance, float NewMagicResistance);

	UFUNCTION()
	void UpdateAttackSpeedText(float PreviousAttackSpeed, float NewAttackSpeed);

	UFUNCTION()
	void UpdateAbilityHasteText(int32 PreviousAbilityHaste, int32 NewAbilityHaste);

	UFUNCTION()
	void UpdateCriticalChanceText(int32 PreviousCriticalChance, int32 NewCriticalChance);

	UFUNCTION()
	void UpdateMovementSpeedText(float PreviousMovementSpeed, float NewMovementSpeed);
	
private:
	TWeakObjectPtr<UStatComponent> StatComponent;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat", Meta = (BindWidget))
	TObjectPtr<UTextBlock> AttackDamageText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat", Meta = (BindWidget))
	TObjectPtr<UTextBlock> AbilityPowerText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat", Meta = (BindWidget))
	TObjectPtr<UTextBlock> DefensePowerText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat", Meta = (BindWidget))
	TObjectPtr<UTextBlock> MagicResistanceText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat", Meta = (BindWidget))
	TObjectPtr<UTextBlock> AttackSpeedText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat", Meta = (BindWidget))
	TObjectPtr<UTextBlock> AbilityHasteText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat", Meta = (BindWidget))
	TObjectPtr<UTextBlock> CriticalChanceText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat", Meta = (BindWidget))
	TObjectPtr<UTextBlock> MovementSpeedText;
};
