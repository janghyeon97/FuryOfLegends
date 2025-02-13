// Fill out your copyright notice in the Description page of Project Settings.


#include "CrowdControls/StunEffect.h"
#include "Characters/CharacterBase.h"
#include "Characters/AOSCharacterBase.h"
#include "Characters/MinionBase.h"
#include "Controllers/BaseAIController.h"
#include "Components/ActionStatComponent.h"
#include "CrowdControls/CrowdControlManager.h"
#include "GameFramework/CharacterMovementComponent.h"


void UStunEffect::ApplyEffect(ACharacter* InTarget, const float InDuration, const float InPercent)
{
	if (!InTarget || !InTarget->GetWorld())
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Invalid target or world."));
		return;
	}

	ACharacterBase* Character = Cast<ACharacterBase>(InTarget);
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Target is not a valid ACharacterBase."));
		return;
	}

	// �⺻ CC ����
	SetupBaseEffect(Character, InTarget, InDuration, InPercent);

	// ĳ���Ͱ� Player���� AI������ ���� CC ������ ����
	if (EnumHasAnyFlags(Character->ObjectType, EObjectType::Player))
	{
		ApplyEffectToPlayer(Cast<AAOSCharacterBase>(Character));
	}
	else
	{
		ApplyEffectToAI(Character);
	}

	UActionStatComponent* ActionStatComponent = Character->GetActionStatComponent();
	if (!ActionStatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: ActionStatComponent is null for the target."));
		return;
	}

	// ActiveAbilityStates �� ��� ��ų ������ ���� �迭
	TArray<FActiveActionState*> ActiveAbilityStates = ActionStatComponent->GetActiveActionStatePtrs();
	if (ActiveAbilityStates.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyEffect failed: Abilities array is empty."));
		return;
	}

	for (FActiveActionState* ActiveAbilityState : ActiveAbilityStates)
	{
		if (ActiveAbilityState == nullptr)
		{
			continue;
		}

		if (ActiveAbilityState->Name.IsNone())
		{
			continue;
		}

		if (ActiveAbilityState->ActionType == EActionType::Cleanse)
		{
			continue;
		}

		ActiveAbilityState->bCanCastAction = false; // ��ų�� ����� �� ������ ����
		ActiveAbilityState->ActiveCrowdControlCount++;
		ActionStatComponent->ClientNotifyActivationChanged(ActiveAbilityState->SlotID, false);
		DisabledAbilities.Add(ActiveAbilityState);

		UE_LOG(LogTemp, Log, TEXT("%s skill has been disabled due to Stun effect."), *ActiveAbilityState->Name.ToString());
	}
}

void UStunEffect::ApplyEffectToPlayer(AAOSCharacterBase* Player)
{
	if (!::IsValid(Player)) return;

	Player->ClientSetControllerRotationYaw(false);
	Player->MulticastPauseMontage();
	Player->CancelAction();
}

void UStunEffect::ApplyEffectToAI(ACharacterBase* AICharacter)
{
	if (!::IsValid(AICharacter)) return;

	ABaseAIController* AIController = Cast<ABaseAIController>(AICharacter->GetController());
	if (AIController)
	{
		AIController->PauseAI(TEXT("Stun"));
	}

	AICharacter->MulticastPauseMontage();
}

void UStunEffect::SetupBaseEffect(ACharacterBase* Character, ACharacter* InTarget, const float InDuration, const float InPercent)
{
	// ���� CC ����
	Character->ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::Move);
	EnumAddFlags(Character->CrowdControlState, ECrowdControl::Stun);

	UCharacterMovementComponent* MovementComponent = Character->GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->StopMovementImmediately();
	}

	// �⺻ Ÿ�̸ӿ� Duration ����
	Target = InTarget;
	Duration = InDuration;
	Percent = InPercent;

	// �⺻ Ÿ�̸� ����
	InTarget->GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	InTarget->GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			this->RemoveEffect();
		},
		Duration,
		false
	);
}



void UStunEffect::RemoveEffect()
{
	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveEffect failed: Target is null."));
		return;
	}

	ACharacterBase* Character = Cast<ACharacterBase>(Target);
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveEffect failed: Target is not a valid ACharacterBase."));
		return;
	}

	AController* Controller = Cast<AController>(Character->GetController());
	if (!Controller)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveEffect failed: Target's controller is not a valid AController."));
		return;
	}

	if (EnumHasAnyFlags(Character->ObjectType, EObjectType::Player))
	{
		RemoveEffectFromPlayer(Cast<AAOSCharacterBase>(Character));
	}
	else
	{
		RemoveEffectFromAI(Character);
	}

	RemoveBaseEffect(Character);
	ReturnEffect();
}

void UStunEffect::RemoveBaseEffect(ACharacterBase* Character)
{
	EnumRemoveFlags(Character->CrowdControlState, ECrowdControl::Stun);
	EnumAddFlags(Character->CharacterState, ECharacterState::SwitchAction);

	Character->ServerStopAllMontages(0.25f, true);

	// CrowdControlState �� CharacterState ������Ʈ
	Character->ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::Move);

	UActionStatComponent* ActionStatComponent = Character->GetActionStatComponent();
	if (!ActionStatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveEffect failed: ActionStatComponent is null for the target."));
		return;
	}

	if (DisabledAbilities.Num() > 0)
	{
		for (FActiveActionState* ActiveAbilityState : DisabledAbilities)
		{
			ActiveAbilityState->ActiveCrowdControlCount--;

			// CC�� �ϳ��� ����Ǿ� �ְ� ������ 1 �̻��� ��� �����ϰ� ��ų ��� ���� ���·� ����
			if (ActiveAbilityState->ActiveCrowdControlCount == 0 && ActiveAbilityState->CurrentLevel >= 1)
			{
				ActiveAbilityState->bCanCastAction = true;
				ActionStatComponent->ClientNotifyActivationChanged(ActiveAbilityState->SlotID, true);
			}
		}

		DisabledAbilities.Empty();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveEffect: DisabledAbilities array is empty."));
	}
}

void UStunEffect::RemoveEffectFromPlayer(AAOSCharacterBase* Player)
{
	if (!Player) return;

	Player->ClientSetControllerRotationYaw(true);
}

// AI ĳ������ ���� ȿ�� ����
void UStunEffect::RemoveEffectFromAI(ACharacterBase* AICharacter)
{
	if (!AICharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("RemoveEffectFromAI failed: AICharacter is null."));
		return;
	}

	if (ABaseAIController* AIController = Cast<ABaseAIController>(AICharacter->GetController()))
	{
		AIController->ResumeAI(TEXT("StunEnded"));
	}
}


void UStunEffect::ReturnEffect()
{
	if (!::IsValid(Target))
	{
		UE_LOG(LogTemp, Warning, TEXT("ReturnEffect failed: Target is null."));
		return;
	}

	ACharacterBase* Character = Cast<ACharacterBase>(Target);
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReturnEffect failed: Target is not a valid ACharacterBase."));
		return;
	}

	// ActiveEffects���� ȿ�� ����
	if (Character->ActiveEffects.Remove(ECrowdControl::Stun) > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Removed Stun effect from ActiveEffects."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ReturnEffect: ActiveEffects does not contain Stun effect."));
	}

	// ȿ�� ��ü�� Ǯ�� ��ȯ
	if (UCrowdControlManager* CrowdControlManager = UCrowdControlManager::Get())
	{
		if (::IsValid(CrowdControlManager))
		{
			CrowdControlManager->ReturnEffect(ECrowdControl::Stun, this);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ReturnEffect failed: Retrieved CrowdControlManager instance is invalid."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ReturnEffect failed: Could not retrieve UCrowdControlManager instance."));
	}
}



void UStunEffect::Reset()
{
	DisabledAbilities.Empty();
	Duration = 0.0f;
	Percent = 0.0f;
}

float UStunEffect::GetDuration() const
{
	return Duration;
}

float UStunEffect::GetPercent() const
{
	return Percent;
}
