// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_StatPanel.h"
#include "Components/TextBlock.h"
#include "Components/StatComponent.h"


void UUW_StatPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	
}

void UUW_StatPanel::InitializeWidget(UStatComponent* InStatComponent)
{
	if (::IsValid(InStatComponent) == false)
	{
		return;
	}

    StatComponent = InStatComponent;
	StatComponent->OnAttackDamageChanged.AddDynamic(this, &ThisClass::UpdateAttackDamageText);
	StatComponent->OnAbilityPowerChanged.AddDynamic(this, &ThisClass::UpdateAbilityPowerText);
	StatComponent->OnDefensePowerChanged.AddDynamic(this, &ThisClass::UpdateDefensePowerText);
	StatComponent->OnMagicResistanceChanged.AddDynamic(this, &ThisClass::UpdateMagicResistanceText);
	StatComponent->OnAttackSpeedChanged.AddDynamic(this, &ThisClass::UpdateAttackSpeedText);
	StatComponent->OnAbilityHasteChanged.AddDynamic(this, &ThisClass::UpdateAbilityHasteText);
	StatComponent->OnCriticalChanceChanged.AddDynamic(this, &ThisClass::UpdateCriticalChanceText);
	StatComponent->OnMovementSpeedChanged.AddDynamic(this, &ThisClass::UpdateMovementSpeedText);
}

void UUW_StatPanel::UpdateAttackDamageText(float PreviousAttackDamage, float NewAttackDamage)
{
    FString StatString = FString::Printf(TEXT("%s : %d"), TEXT("AD"), FMath::CeilToInt(NewAttackDamage));
    AttackDamageText->SetText(FText::FromString(StatString));
}

void UUW_StatPanel::UpdateAbilityPowerText(float PreviousAbilityPower, float NewAbilityPower)
{
    FString StatName = TEXT("AP");
    FString StatString = FString::Printf(TEXT("%s : %d"), *StatName, FMath::CeilToInt(NewAbilityPower));
    AbilityPowerText->SetText(FText::FromString(StatString));
}

void UUW_StatPanel::UpdateDefensePowerText(float PreviousDefensePower, float NewDefensePower)
{
    FString StatName = TEXT("DP");
    FString StatString = FString::Printf(TEXT("%s : %d"), *StatName, FMath::CeilToInt(NewDefensePower));
    DefensePowerText->SetText(FText::FromString(StatString));
}

void UUW_StatPanel::UpdateMagicResistanceText(float PreviousMagicResistance, float NewMagicResistance)
{
    FString StatName = TEXT("MR");
    FString StatString = FString::Printf(TEXT("%s : %d"), *StatName, FMath::CeilToInt(NewMagicResistance));
    MagicResistanceText->SetText(FText::FromString(StatString));
}

void UUW_StatPanel::UpdateAttackSpeedText(float PreviousAttackSpeed, float NewAttackSpeed)
{
    FString StatName = TEXT("AS");
    FString StatString = FString::Printf(TEXT("%s : %.2f"), *StatName, NewAttackSpeed);
    AttackSpeedText->SetText(FText::FromString(StatString));
}

void UUW_StatPanel::UpdateAbilityHasteText(int32 PreviousAbilityHaste, int32 NewAbilityHaste)
{
    FString StatName = TEXT("AH");
    FString StatString = FString::Printf(TEXT("%s : %d"), *StatName, NewAbilityHaste);
    AbilityHasteText->SetText(FText::FromString(StatString));
}

void UUW_StatPanel::UpdateCriticalChanceText(int32 PreviousCriticalChance, int32 NewCriticalChance)
{
    FString StatName = TEXT("CC");
    FString StatString = FString::Printf(TEXT("%s : %d"), *StatName, NewCriticalChance);
    CriticalChanceText->SetText(FText::FromString(StatString));
}

void UUW_StatPanel::UpdateMovementSpeedText(float PreviousMovementSpeed, float NewMovementSpeed)
{
    FString StatName = TEXT("MS");
    FString StatString = FString::Printf(TEXT("%s : %d"), *StatName, FMath::CeilToInt(NewMovementSpeed));
    MovementSpeedText->SetText(FText::FromString(StatString));
}