// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_CheckForEnemies.h"
#include "Controllers/BaseAIController.h"
#include "Characters/CharacterBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/OverlapResult.h"

UBTService_CheckForEnemies::UBTService_CheckForEnemies()
{
	NodeName = TEXT("Check For Enemies");
	Interval = 0.1f;
}

void UBTService_CheckForEnemies::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return;
	}

	ACharacterBase* AICharacter = Cast<ACharacterBase>(AIController->GetPawn());
	if (!AICharacter)
	{
		return;
	}

	// ���� Ÿ���� ��ȿ���� Ȯ���ϰ�, ��ȿ���� ������ ���ο� Ž�� ����
	ACharacterBase* CurrentTarget = Cast<ACharacterBase>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(ABaseAIController::TargetActorKey));
	if (::IsValid(CurrentTarget) == false || EnumHasAnyFlags(CurrentTarget->CharacterState, ECharacterState::Death))
	{
		OwnerComp.GetBlackboardComponent()->ClearValue(ABaseAIController::TargetActorKey);
	}
	else
	{
		return;
	}

	FVector CenterPosition = AICharacter->GetActorLocation();
	const float Range = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(ABaseAIController::RangeKey);
	const float DetectRadius = FMath::Clamp(Range, 200, 1000);

	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(AICharacter);

	bool bResult = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		CenterPosition,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel7,
		FCollisionShape::MakeSphere(DetectRadius),
		QueryParams
	);

	ACharacterBase* ClosestObject = nullptr;
	ACharacterBase* ClosestMinion = nullptr;
	ACharacterBase* ClosestPlayer = nullptr;
	float ClosestObjectDistance = FLT_MAX;
	float ClosestMinionDistance = FLT_MAX;
	float ClosestPlayerDistance = FLT_MAX;

	for (const auto& OverlapResult : OverlapResults)
	{
		ACharacterBase* Character = Cast<ACharacterBase>(OverlapResult.GetActor());
		if (!::IsValid(Character))
		{
			continue;
		}

		// ���� �� ����
		if (AICharacter->TeamSide == Character->TeamSide)
		{
			continue;
		}

		if (Character->ObjectType == EObjectType::Turret)
		{
			float DistanceToObject = FVector::Dist(CenterPosition, Character->GetActorLocation());
			if (DistanceToObject < ClosestObjectDistance)
			{
				ClosestObject = Character;
				ClosestObjectDistance = DistanceToObject;
			}
		}
		// �̴Ͼ� ó��
		else if (Character->ObjectType == EObjectType::Minion)
		{
			float DistanceToMinion = FVector::Dist(CenterPosition, Character->GetActorLocation());
			if (DistanceToMinion < ClosestMinionDistance)
			{
				ClosestMinion = Character;
				ClosestMinionDistance = DistanceToMinion;
			}
		}
		// �÷��̾� ó��
		else if (Character->ObjectType == EObjectType::Player)
		{
			float DistanceToPlayer = FVector::Dist(CenterPosition, Character->GetActorLocation());
			if (DistanceToPlayer < ClosestPlayerDistance)
			{
				ClosestPlayer = Character;
				ClosestPlayerDistance = DistanceToPlayer;
			}
		}
	}
	
	// �켱������ ���� Ÿ�� ����
	if (ClosestObject)
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsObject(ABaseAIController::TargetActorKey, ClosestObject);
	}
	else if (ClosestMinion)
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsObject(ABaseAIController::TargetActorKey, ClosestMinion);
	}
	else if (ClosestPlayer)
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsObject(ABaseAIController::TargetActorKey, ClosestPlayer);

	}
}

