// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MinionData.generated.h"


UENUM(BlueprintType)
enum class EMinionType : uint8
{
	None,
	Melee,
	Ranged,
	Siege,
	Super
};

USTRUCT(BlueprintType)
struct FMinionAttributesRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FMinionAttributesRow()
		: MinionType(EMinionType::None)
		, ExpBounty(0.0f)
		, GoldBounty(0)
		, StatTable(nullptr)
		, AbilityStatTable(nullptr)
		, ResourcesTable(nullptr)
		, SkeletalMesh_Down(nullptr)
		, SkeletalMesh_Dusk(nullptr)
		, MinionClass(nullptr)
	{
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMinionType MinionType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExpBounty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GoldBounty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* StatTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* AbilityStatTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* ResourcesTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* SkeletalMesh_Down;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* SkeletalMesh_Dusk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UClass* MinionClass;
};


/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UMinionData : public UObject
{
	GENERATED_BODY()
	
};
