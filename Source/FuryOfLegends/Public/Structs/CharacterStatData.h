// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CharacterStatData.generated.h"

UENUM(BlueprintType)
enum class ECharacterStat : uint8
{
	None,
	CurrentHealth,
	CurrentMana,
	MaxHealthPoints,
	MaxManaPoints,
	HealthRegeneration,
	ManaRegeneration,
	AttackDamage,
	AbilityPower,
	DefensePower,
	MagicResistance,
	AttackSpeed,
	AbilityHaste,
	CriticalChance,
	MovementSpeed,
};

USTRUCT(BlueprintType)
struct FStatTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FStatTableRow()
		: MaxHP(0.0f)
		, MaxMP(0.0f)
		, MaxEXP(0.0f)
		, HealthRegeneration(0.0f)
		, ManaRegeneration(0.0f)
		, AttackDamage(0.0f)
		, AbilityPower(0.0f)
		, DefensePower(0.0f)
		, MagicResistance(0.0f)
		, AttackSpeed(0.0f)
		, CriticalChance(0)
		, MovementSpeed(0.0f)
		, UniqueAttributes()
	{
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxMP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxEXP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealthRegeneration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ManaRegeneration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AbilityPower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefensePower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MagicResistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CriticalChance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, float> UniqueAttributes;
};

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UCharacterStatData : public UObject
{
	GENERATED_BODY()
	
};
