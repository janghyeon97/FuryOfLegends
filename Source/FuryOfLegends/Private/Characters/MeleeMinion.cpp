// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/MeleeMinion.h"
#include "Game/AOSGameInstance.h"
#include "Controllers/MinionAIController.h"
#include "Components/MinionStatComponent.h"
#include "Components/ActionStatComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Structs/MinionData.h"
#include "Kismet/GameplayStatics.h"

AMeleeMinion::AMeleeMinion()
{
	CharacterName = "Melee";

	ComboCount = 1;
	MaxComboCount = 4;
}


void AMeleeMinion::BeginPlay()
{
	Super::BeginPlay();
}

void AMeleeMinion::MontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	Super::MontageEnded(Montage, bInterrupted);

	if (!Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::MontageEnded] Montage is null."));
		return;
	}

	if (Montage->GetFName() == FName("Attack_Montage") && HasAuthority())
	{
		ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::AttackEnded);
	}
}

void AMeleeMinion::LMB_Executed()
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (EnumHasAnyFlags(CharacterState, ECharacterState::Death))
	{
		return;
	}

	if (::IsValid(ActionStatComponent) == false)
	{
		EnumAddFlags(CharacterState, ECharacterState::AttackEnded);
		return;
	}

	bool bIsActionReady = ActionStatComponent->IsActionReady(EActionSlot::LMB);
	if (!bIsActionReady)
	{
		EnumAddFlags(CharacterState, ECharacterState::AttackEnded);
		return;
	}

	UAnimMontage* Montage = GetOrLoadMontage("LMB", *FString::Printf(TEXT("/Game/ParagonMinions/Characters/Minions/Down_Minions/Animations/%s/Attack_Montage.Attack_Montage"), *CharacterName.ToString()));
	if (!Montage)
	{
		EnumAddFlags(CharacterState, ECharacterState::AttackEnded);
		return;
	}

	ActionStatComponent->HandleActionExecution(EActionSlot::LMB, GetWorld()->GetTimeSeconds());
	ActionStatComponent->ActivateActionCooldown(EActionSlot::LMB);

	const float MontageDuration = Montage->GetSectionLength(ComboCount - 1);
	const float PlayRate = AdjustAnimPlayRate(MontageDuration);

	PlayAnimMontage(Montage, PlayRate, FName(*FString::Printf(TEXT("Attack%d"), ComboCount)));
	MulticastPlayMontage(Montage, PlayRate, FName(*FString::Printf(TEXT("Attack%d"), ComboCount)), true);

	ComboCount = FMath::Clamp<int32>((ComboCount % 4) + 1, 1, MaxComboCount);
}

void AMeleeMinion::LMB_CheckHit()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!::IsValid(ActionStatComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::LMB_CheckHit] ActionStatComponent is not valid."));
		return;
	}

	AMinionAIController* AI = Cast<AMinionAIController>(GetController());
	if (!AI)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::LMB_CheckHit] Failed to get AIController."));
		return;
	}

	ACharacterBase* Enemy = Cast<ACharacterBase>(AI->GetBlackboardComponent()->GetValueAsObject(AMinionAIController::TargetActorKey));
	if (!::IsValid(Enemy))
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::LMB_CheckHit] Enemy is not valid or not found."));
		return;
	}

	const FActionAttributes& ActionAttributes = ActionStatComponent->GetActionAttributes(EActionSlot::LMB);

	const float Character_AttackDamage = StatComponent->GetAttackDamage();
	const float BaseAttackDamage = ActionAttributes.AttackDamage;
	const float PhysicalScaling = ActionAttributes.PhysicalScaling;

	const float FinalDamage = BaseAttackDamage + Character_AttackDamage * PhysicalScaling;

	FDamageInformation DamageInformation;
	DamageInformation.ActionSlot = EActionSlot::LMB;
	DamageInformation.AddDamage(EDamageType::Physical, FinalDamage);

	ServerApplyDamage(Enemy, this, AIController, DamageInformation);
}