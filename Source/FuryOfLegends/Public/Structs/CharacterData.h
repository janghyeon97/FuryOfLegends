// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CharacterData.generated.h"


UENUM(BlueprintType)
enum class ELaneType : uint8
{
	None        UMETA(DisplayName = "None"),
	Top			UMETA(DisplayName = "Top"),
	Mid         UMETA(DisplayName = "Mid"),
	Bottom      UMETA(DisplayName = "Bottom"),
};


UENUM(BlueprintType)
enum class ETeamSide : uint8
{
	None	UMETA(Hidden),
	Blue	UMETA(DisplayName = "Blue"),
	Red		UMETA(DisplayName = "Red"),
	Neutral UMETA(DisplayName = "Neutral"),
};

UENUM(BlueprintType)
enum class EObjectType : uint8
{
    None          UMETA(DisplayName = "None"),
    System        UMETA(DisplayName = "System"),
    Player        UMETA(DisplayName = "Player"),
    Minion        UMETA(DisplayName = "Minion"),
    Monster       UMETA(DisplayName = "Monster"),
    EpicMonster   UMETA(DisplayName = "EpicMonster"),
    Turret        UMETA(DisplayName = "Turret"),
    Trap          UMETA(DisplayName = "Trap")
};

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ECharacterState : uint32
{
	None			= 0	UMETA(Hidden),

	// 기본 비활성화 상태
	Q				= 1 << 0 UMETA(DisplayName = "Q Slot"),      // Q 능력 슬롯
	E				= 1 << 1 UMETA(DisplayName = "E Slot"),      // E 능력 슬롯
	R				= 1 << 2 UMETA(DisplayName = "R Slot"),      // R 능력 슬롯
	LMB				= 1 << 3 UMETA(DisplayName = "Left Mouse"),  // 좌클릭
	RMB				= 1 << 4 UMETA(DisplayName = "Right Mouse"), // 우클릭

	// 기본 활성화 상태
	Move			= 1 << 5 UMETA(DisplayName = "Move"),
	Jump			= 1 << 6 UMETA(DisplayName = "Jump"),
	Rooted			= 1 << 7 UMETA(DisplayName = "Jump"),
	SwitchAction	= 1 << 8 UMETA(DisplayName = "SwitchAction"),

	// 기본 비활성화 상태
	Death			= 1 << 9 UMETA(DisplayName = "Death"),
	Recall			= 1 << 10 UMETA(DisplayName = "Recall"),
	Invulnerability = 1 << 11 UMETA(DisplayName = "Invulnerability"),
	Attacking		= 1 << 12 UMETA(DisplayName = "Attacking"),
	AttackEnded		= 1 << 13 UMETA(DisplayName = "AttackEnded"),
	ActionActivated = 1 << 14 UMETA(DisplayName = "ActionActivated"),
	
};
ENUM_CLASS_FLAGS(ECharacterState);


USTRUCT(BlueprintType)
struct FCharacterAttributesRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FCharacterAttributesRow()
		: Index(0)
		, ChampionName(NAME_None)
		, Position(ELaneType::None)
		, ChampionImage(nullptr)
		, StatTable(nullptr)
		, ActionStatTable(nullptr)
		, CharacterResourcesTable(nullptr)
		, SkinTable(nullptr)
		, CharacterClass(nullptr)
	{
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Index;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChampionName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ELaneType Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture* ChampionImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* StatTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* ActionStatTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* CharacterResourcesTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* SkinTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UClass* CharacterClass;
};

USTRUCT(BlueprintType)
struct FCharacterSkinTableRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FCharacterSkinTableRow()
		: Name(TEXT(""))
		, SkinName(TEXT(""))
		, SkinImage(nullptr)
		, Mesh(nullptr)
	{
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SkinName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture* SkinImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* Mesh;
};

UCLASS()
class FURYOFLEGENDS_API UCharacterData : public UObject
{
	GENERATED_BODY()
	
};