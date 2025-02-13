// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavAreas/NavArea.h"
#include "NavArea_AvoidCharacter.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UNavArea_AvoidCharacter : public UNavArea
{
	GENERATED_BODY()
	
public:
	UNavArea_AvoidCharacter()
	{
		DefaultCost = 10000.0f;
	}
};
