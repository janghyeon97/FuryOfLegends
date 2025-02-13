// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTDecorator_CheckSplineDistance.h"
#include "Controllers/BaseAIController.h"
#include "Characters/MinionBase.h"
#include "Components/SplineComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_CheckSplineDistance::UBTDecorator_CheckSplineDistance()
{
	NodeName = TEXT("Check Spline Distance");
}

bool UBTDecorator_CheckSplineDistance::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] [Character: None] AIController is null."), ANSI_TO_TCHAR(__FUNCTION__));
		return false;
	}

	AMinionBase* AICharacter = Cast<AMinionBase>(AIController->GetPawn());
	if (!AICharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] [Character: None] AICharacter is null."), ANSI_TO_TCHAR(__FUNCTION__));
		return false;
	}

	USplineComponent* Spline = AICharacter->SplineActor->FindComponentByClass<USplineComponent>();
	if (!Spline)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] [Character: %s] Spline component is null"), ANSI_TO_TCHAR(__FUNCTION__), *AICharacter->GetName());
		return false;
	}

	// 현재 위치와 Spline에서 가장 가까운 지점 사이의 거리 계산
	FVector CurrentLocation = AICharacter->GetActorLocation();
	float ClosestDistance = Spline->FindInputKeyClosestToWorldLocation(CurrentLocation);
	FVector ClosestPoint = Spline->GetLocationAtSplineInputKey(ClosestDistance, ESplineCoordinateSpace::World);

	// 거리 조건 검사: Spline으로 복귀할지 또는 Spline을 따라 이동할지 결정
	return FVector::Dist(CurrentLocation, ClosestPoint) <= 200.f;
}
