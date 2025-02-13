// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UniqueAttributeData.generated.h"

USTRUCT(BlueprintType)
struct FUniqueAttribute
{
    GENERATED_BODY()

public:
    FUniqueAttribute()
        : Key(NAME_None), Value(0.f)
    {
    }

    FUniqueAttribute(FName InKey, float InValue)
        : Key(InKey), Value(InValue)
    {
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName Key;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Value;
};

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UUniqueAttributeData : public UObject
{
	GENERATED_BODY()
	
	
	
	
};
