// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateSplineLocation.generated.h"

class ABaseAIController;
class AMinionBase;
class USplineComponent;

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UBTService_UpdateSplineLocation : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_UpdateSplineLocation();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	void UpdateNextSplineLocation(ABaseAIController* AIController, AMinionBase* AICharacter, USplineComponent* Spline, float DeltaSeconds);
};
