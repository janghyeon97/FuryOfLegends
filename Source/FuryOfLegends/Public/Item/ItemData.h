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
	Inactive     UMETA(DisplayName = "Inactive"),    // �������� ��Ȱ��ȭ ����
	Active       UMETA(DisplayName = "Active"),      // �������� Ȱ��ȭ�Ǿ� ��� ��
	Cooldown     UMETA(DisplayName = "Cooldown"),    // ���� ��� ����x
	Expired      UMETA(DisplayName = "Expired"),     // ������ ȿ���� �����
	Pending      UMETA(DisplayName = "Pending")      // ��� �� ���� (��: Ȱ��ȭ ����)
};

UENUM(meta = (UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EItemClassification : uint32
{
	/** �⺻��: �������� �з����� �ʾҽ��ϴ�. */
	None UMETA(DisplayName = "None"),

	/** ��Ÿ�� ���������� �ʱ� �ܰ迡�� ����ϴ� �������Դϴ�. */
	Starter UMETA(DisplayName = "Starter"),

	/** ü�� �Ǵ� ���� ���ǰ� ���� ȸ���� �������Դϴ�. */
	Potions UMETA(DisplayName = "Potions"),

	/** ��� �� �Һ�Ǵ� ��ȸ�� �������Դϴ�. */
	Consumables UMETA(DisplayName = "Consumables"),

	/** Ư�� ��ű� �������Դϴ�. */
	Trinkets UMETA(DisplayName = "Trinkets"),

	/** ������ ���� �̵� �ӵ� ��� �������Դϴ�. */
	Boots UMETA(DisplayName = "Boots"),

	/** �⺻ �ܰ��� �������Դϴ�. */
	Basic UMETA(DisplayName = "Basic"),

	/** �߱� �ܰ��� �������Դϴ�. */
	Epic UMETA(DisplayName = "Epic"),

	/** �ְ� ����� �������� �������Դϴ�. */
	Legendary UMETA(DisplayName = "Legendary"),

	/** �й� ������: ���̳� ��ü���� ȿ���� �����ϴ� �������Դϴ�. */
	Distributed UMETA(DisplayName = "Distributed"),

	/** Ư�� è�Ǿ� ����� �� �ִ� ���� �������Դϴ�. */
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
	int32 MaxConcurrentUses; // �� ���� Ȱ��ȭ�� �� �ִ� �ִ� ��ø

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 MaxStackPerSlot; // �� ���Կ��� ��ø ������ �ִ� ����

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 MaxInventoryQuantity; // �κ��丮���� ���� ������ �ִ� ����

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
