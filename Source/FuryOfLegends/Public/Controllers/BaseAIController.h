// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BaseAIController.generated.h"

DECLARE_DELEGATE_OneParam(FOnMoveFinishedDelegate, bool);


/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API ABaseAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ABaseAIController();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnPossess(APawn* InPawn) override;

public:
	virtual void BeginAI(APawn* InPawn) {};
	virtual void EndAI();
	virtual void PauseAI(const FString& Reason);
	virtual void ResumeAI(const FString& Reason);

public:
	static const FName RangeKey;
	static const FName DetectRangeKey;
	static const FName MovementSpeedKey;
	static const FName TargetActorKey;
	static const FName TargetLocationKey;
	static const FName DistanceToTargetKey;
	static const FName ChaseThresholdKey;
	static const FName LocationAlongSplineKey;
	static const FName DistanceAlongSplineKey;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseAIController", meta = (AllowPrivateAccess))
	TObjectPtr<class UBlackboardData> BlackboardDataAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaseAIController", meta = (AllowPrivateAccess))
	TObjectPtr<class UBehaviorTree> BehaviorTree;
};
