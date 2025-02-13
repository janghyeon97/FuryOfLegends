// Fill out your copyright notice in the Description page of Project Settings.

#include "Controllers/NPCAIController.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetSystemLibrary.h"


ANPCAIController::ANPCAIController()
{

}

void ANPCAIController::BeginAI(APawn* InPawn)
{
	Super::BeginAI(InPawn);

	UBlackboardComponent* BlackboardComponent = Cast<UBlackboardComponent>(Blackboard);
	if (::IsValid(BlackboardComponent))
	{
		if (UseBlackboard(BlackboardDataAsset, BlackboardComponent))
		{
			bool bRunSucceed = RunBehaviorTree(BehaviorTree);
			ensure(bRunSucceed == true);
		}
	}
}

void ANPCAIController::EndAI()
{
	Super::EndAI();

	UBehaviorTreeComponent* BehaviroTreeComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (::IsValid(BehaviroTreeComponent))
	{
		BehaviroTreeComponent->StopTree();
	}
}

void ANPCAIController::OnPatrolTimerElapsed()
{
	APawn* ControlledPawn = GetPawn();
	if (::IsValid(ControlledPawn))
	{
		UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());
		if (::IsValid(NavSystem))
		{
			FNavLocation NextLocation;
			if (true == NavSystem->GetRandomPointInNavigableRadius(FVector::ZeroVector, 500.f, NextLocation))
			{
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, NextLocation.Location);
			}
		}
	}
}
