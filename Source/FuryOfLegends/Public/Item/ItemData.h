// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Structs/CharacterStatData.h"
#include "Structs/UniqueAttributeData.h"
#include "ItemData.generated.h"

class UStatComponent;
class AItem;


UENUM(BlueprintType)
enum class EItemActivationState : uint8
{
	Inactive     UMETA(DisplayName = "Inactive"),    // 아이템이 비활성화 상태
	Active       UMETA(DisplayName = "Active"),      // 아이템이 활성화되어 사용 중
	Cooldown     UMETA(DisplayName = "Cooldown"),    // 재사용 대기 상태x
	Expired      UMETA(DisplayName = "Expired"),     // 아이템 효과가 만료됨
	Pending      UMETA(DisplayName = "Pending")      // 대기 중 상태 (예: 활성화 예약)
};

UENUM(meta = (UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EItemClassification : uint32
{
	/** 기본값: 아이템이 분류되지 않았습니다. */
	None UMETA(DisplayName = "None"),

	/** 스타터 아이템으로 초기 단계에서 사용하는 아이템입니다. */
	Starter UMETA(DisplayName = "Starter"),

	/** 체력 또는 마나 포션과 같은 회복용 아이템입니다. */
	Potions UMETA(DisplayName = "Potions"),

	/** 사용 후 소비되는 일회용 아이템입니다. */
	Consumables UMETA(DisplayName = "Consumables"),

	/** 특수 장신구 아이템입니다. */
	Trinkets UMETA(DisplayName = "Trinkets"),

	/** 부츠와 같은 이동 속도 향상 아이템입니다. */
	Boots UMETA(DisplayName = "Boots"),

	/** 기본 단계의 아이템입니다. */
	Basic UMETA(DisplayName = "Basic"),

	/** 중급 단계의 아이템입니다. */
	Epic UMETA(DisplayName = "Epic"),

	/** 최고 등급의 전설적인 아이템입니다. */
	Legendary UMETA(DisplayName = "Legendary"),

	/** 분배 아이템: 팀이나 전체적인 효과를 제공하는 아이템입니다. */
	Distributed UMETA(DisplayName = "Distributed"),

	/** 특정 챔피언만 사용할 수 있는 전용 아이템입니다. */
	ChampionExclusive UMETA(DisplayName = "Champion Exclusive")
};

USTRUCT(BlueprintType)
struct FItemStatModifier
{
	GENERATED_BODY()

public:
	FItemStatModifier()
		: Key(ECharacterStat::None), Value(0.f)
	{
	}

	FItemStatModifier(ECharacterStat InKey, float InValue)
		: Key(InKey), Value(InValue)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECharacterStat Key;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value;
};

USTRUCT(BlueprintType)
struct FItemTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FItemTableRow();

	bool IsEmpty() const { return ItemCode == 0; }
	FString ConverClassificationToString() const;
	FString ConvertCharacterStatToString(ECharacterStat StatToConvert) const;
	FString ConvertToRichText(UStatComponent* StatComponent) const;

private:
	FString ApplyColorTags(const FString& Text) const;
	FString ReplaceStatTags(const FString& Text, UStatComponent* StatComponent) const;
	FString ReplaceCalcTags(const FString& Text, UStatComponent* StatComponent) const;

	TMap<FString, TFunction<float(const UStatComponent&)>> StatGetters;
	TMap<FString, TFunction<int32(const UStatComponent&)>> StatGettersInt;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 ItemCode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Price;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UTexture* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 MaxConcurrentUses; // 한 번에 활성화할 수 있는 최대 중첩

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 MaxStackPerSlot; // 한 슬롯에서 중첩 가능한 최대 개수

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 MaxInventoryQuantity; // 인벤토리에서 보유 가능한 최대 개수

	UPROPERTY(Transient, meta = (Hidden))
	int32 ConcurrentUses;

	UPROPERTY(Transient, meta = (Hidden))
	int32 CurrentStackPerSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EItemClassification Classification;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TArray<FItemStatModifier> StatModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TArray<int> RequiredItems;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int32> UniqueAttributes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSubclassOf<AItem> ItemClass;
};

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UItemData : public UObject
{
	GENERATED_BODY()
	
};
