#include "Controllers/BaseAIController.h"
#include "Characters/CharacterBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Algo/Reverse.h"


const FName ABaseAIController::RangeKey(TEXT("Range"));
const FName ABaseAIController::DetectRangeKey(TEXT("DetectRange"));
const FName ABaseAIController::MovementSpeedKey(TEXT("MovementSpeed"));
const FName ABaseAIController::TargetActorKey(TEXT("TargetActor"));
const FName ABaseAIController::TargetLocationKey(TEXT("TargetLocation"));
const FName ABaseAIController::DistanceToTargetKey(TEXT("DistanceToTarget"));
const FName ABaseAIController::ChaseThresholdKey(TEXT("ChaseThreshold"));
const FName ABaseAIController::LocationAlongSplineKey(TEXT("LocationAlongSpline"));
const FName ABaseAIController::DistanceAlongSplineKey(TEXT("DistanceAlongSpline"));


ABaseAIController::ABaseAIController()
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackBoard"));
	BrainComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BrainComponent"));
}

void ABaseAIController::BeginPlay()
{
	Super::BeginPlay();

}

void ABaseAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	EndAI();
}

void ABaseAIController::EndAI()
{
	UBehaviorTreeComponent* BehaviroTreeComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (::IsValid(BehaviroTreeComponent))
	{
		BehaviroTreeComponent->StopTree();
	}
}

void ABaseAIController::PauseAI(const FString& Reason)
{
	UBehaviorTreeComponent* BehaviroTreeComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (::IsValid(BehaviroTreeComponent))
	{
		BehaviroTreeComponent->PauseLogic(Reason);
	}
}

void ABaseAIController::ResumeAI(const FString& Reason)
{
	UBehaviorTreeComponent* BehaviroTreeComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (::IsValid(BehaviroTreeComponent))
	{
		BehaviroTreeComponent->ResumeLogic(Reason);
	}
}


void ABaseAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	BeginAI(InPawn);
}