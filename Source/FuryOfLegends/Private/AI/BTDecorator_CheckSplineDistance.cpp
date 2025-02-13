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

	// ���� ��ġ�� Spline���� ���� ����� ���� ������ �Ÿ� ���
	FVector CurrentLocation = AICharacter->GetActorLocation();
	float ClosestDistance = Spline->FindInputKeyClosestToWorldLocation(CurrentLocation);
	FVector ClosestPoint = Spline->GetLocationAtSplineInputKey(ClosestDistance, ESplineCoordinateSpace::World);

	// �Ÿ� ���� �˻�: Spline���� �������� �Ǵ� Spline�� ���� �̵����� ����
	return FVector::Dist(CurrentLocation, ClosestPoint) <= 200.f;
}
