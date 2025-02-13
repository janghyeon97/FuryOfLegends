// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTTask_ReturnToSpline.h"
#include "Controllers/BaseAIController.h"
#include "Characters/CharacterBase.h"
#include "Characters/MinionBase.h"
#include "Components/SplineComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

UBTTask_ReturnToSpline::UBTTask_ReturnToSpline()
{
	NodeName = TEXT("Return To Spline");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_ReturnToSpline::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);
	if (Result == EBTNodeResult::Failed)
	{
		return EBTNodeResult::Failed;
	}

	ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s][Controller: None] AIController is null, finishing task as failed."), ANSI_TO_TCHAR(__FUNCTION__));
		return EBTNodeResult::Failed;
	}

	AMinionBase* AICharacter = Cast<AMinionBase>(AIController->GetPawn());
	if (!AICharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s][Controller: %s] AICharacter is null, finishing task as failed."), ANSI_TO_TCHAR(__FUNCTION__), *AIController->GetName());
		return EBTNodeResult::Failed;
	}

	USplineComponent* Spline = AICharacter->SplineActor->FindComponentByClass<USplineComponent>();
	if (!Spline)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s][Character: %s] Spline component is null."), ANSI_TO_TCHAR(__FUNCTION__), *AICharacter->GetName());
		return EBTNodeResult::Failed;
	}

	MoveToClosestSplinePoint(OwnerComp, AIController, AICharacter, Spline);
	return EBTNodeResult::InProgress;
}

void UBTTask_ReturnToSpline::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s][Character: None] AIController is null, finishing task as failed."), ANSI_TO_TCHAR(__FUNCTION__));
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AMinionBase* AICharacter = Cast<AMinionBase>(AIController->GetPawn());
	if (!AICharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s][Controller: %s] AICharacter is null, finishing task as failed."), ANSI_TO_TCHAR(__FUNCTION__), *AIController->GetName());
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	USplineComponent* Spline = AICharacter->SplineActor->FindComponentByClass<USplineComponent>();
	if (!Spline)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s][Character: %s] Spline component is null."), ANSI_TO_TCHAR(__FUNCTION__), *AICharacter->GetName());
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FVector CurrentLocation = AICharacter->GetActorLocation();
	FVector TargetSplineLocation = AIController->GetBlackboardComponent()->GetValueAsVector(ABaseAIController::LocationAlongSplineKey);
	float MaxChaseDistance = AIController->GetBlackboardComponent()->GetValueAsFloat(ABaseAIController::ChaseThresholdKey);
	float DetectRadius = AIController->GetBlackboardComponent()->GetValueAsFloat(ABaseAIController::RangeKey);;

	float DistanceToSpline = FVector::Dist(CurrentLocation, TargetSplineLocation);

	// 목표 Spline 포인트에 도달했는지 확인
	if (DistanceToSpline <= 200.f)
	{
		//UE_LOG(LogTemp, Log, TEXT("[%s][Character: %s] Reached the target spline point, finishing task."), ANSI_TO_TCHAR(__FUNCTION__), *AICharacter->GetName());

		float ClosestInputKey = Spline->FindInputKeyClosestToWorldLocation(AICharacter->GetActorLocation());
		float ClosestDistanceOnSpline = Spline->GetDistanceAlongSplineAtSplineInputKey(ClosestInputKey);

		// DistanceAlongSplineKey를 업데이트
		AIController->GetBlackboardComponent()->SetValueAsFloat(ABaseAIController::DistanceAlongSplineKey, ClosestDistanceOnSpline);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}

	// 탐지 거리와 Spline까지의 거리의 합이 최대 추적 거리를 초과하지 않으면 적 탐지
	if (DistanceToSpline + DetectRadius + 100 < MaxChaseDistance)
	{
		// 주변 적 탐지
		TArray<FOverlapResult> OverlapResults;
		FCollisionQueryParams CollisionQueryParams(NAME_None, false, AICharacter);

		bool bResult = GetWorld()->OverlapMultiByChannel(
			OverlapResults,
			CurrentLocation,
			FQuat::Identity,
			ECollisionChannel::ECC_GameTraceChannel5,
			FCollisionShape::MakeSphere(DetectRadius),
			CollisionQueryParams
		);

		if (bResult)
		{
			for (const auto& OverlapResult : OverlapResults)
			{
				ACharacterBase* Character = Cast<ACharacterBase>(OverlapResult.GetActor());
				if (::IsValid(Character) && AICharacter->TeamSide != Character->TeamSide)
				{
					//UE_LOG(LogTemp, Log, TEXT("[%s][Character: %s] Enemy detected, switching to attack mode."), ANSI_TO_TCHAR(__FUNCTION__), *AICharacter->GetName());
					AIController->GetBlackboardComponent()->SetValueAsObject(ABaseAIController::TargetActorKey, Character);
					FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
					return;
				}
			}
		}
	}
}

void UBTTask_ReturnToSpline::MoveToClosestSplinePoint(UBehaviorTreeComponent& OwnerComp, class ABaseAIController* AIController, class AMinionBase* AICharacter, class USplineComponent* Spline)
{
	if (!Spline)
	{
		return;
	}

	FVector CurrentLocation = AICharacter->GetActorLocation();
	float ClosestDistance = Spline->FindInputKeyClosestToWorldLocation(CurrentLocation);
	FVector ClosestPoint = Spline->GetLocationAtSplineInputKey(ClosestDistance, ESplineCoordinateSpace::World);

	// 목표 Spline 포인트를 블랙보드에 저장
	AIController->GetBlackboardComponent()->SetValueAsVector(ABaseAIController::LocationAlongSplineKey, ClosestPoint);
	AIController->MoveToLocation(ClosestPoint, -1.0f, true, true, true, true, 0, true);

	//UE_LOG(LogTemp, Log, TEXT("[%s][Character: %s] Moving to closest spline point at location: %s."), ANSI_TO_TCHAR(__FUNCTION__), *AICharacter->GetName(), *ClosestPoint.ToString());
}
