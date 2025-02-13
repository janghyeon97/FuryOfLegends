// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SwitchNavArea.generated.h"

UENUM(BlueprintType)
enum class ENavAreaType : uint8
{
	Default     UMETA(DisplayName = "Default"),
	Null        UMETA(DisplayName = "Null"),
	Obstacle    UMETA(DisplayName = "Obstacle")
};

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UBTTask_SwitchNavArea : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SwitchNavArea();
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
	ENavAreaType NavAreaType;
};
