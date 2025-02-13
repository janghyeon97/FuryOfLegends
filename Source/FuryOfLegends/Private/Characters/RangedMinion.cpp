// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/RangedMinion.h"
#include "Controllers/MinionAIController.h"
#include "Components/BoxComponent.h"
#include "Components/MinionStatComponent.h"
#include "Components/ActionStatComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Props/Projectile.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


ARangedMinion::ARangedMinion()
{
	CharacterName = "Ranged";

	ComboCount = 1;
	MaxComboCount = 4;
}


void ARangedMinion::BeginPlay()
{
	Super::BeginPlay();


}


void ARangedMinion::MontageEnded(UAnimMontage* Montage, bool bInterrupted)
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


void ARangedMinion::LMB_Executed()
{
	if (!HasAuthority()) return;

	// AI 컨트롤러 가져오기
	AMinionAIController* AI = Cast<AMinionAIController>(GetController());
	if (!AI)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to get AIController."), ANSI_TO_TCHAR(__FUNCTION__));
		EnumAddFlags(CharacterState, ECharacterState::AttackEnded);
		return;
	}

	// 적 타겟 가져오기
	ACharacterBase* Enemy = Cast<ACharacterBase>(AI->GetBlackboardComponent()->GetValueAsObject(ABaseAIController::TargetActorKey));
	if (::IsValid(Enemy) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Enemy is not valid or not found."), ANSI_TO_TCHAR(__FUNCTION__));
		EnumAddFlags(CharacterState, ECharacterState::AttackEnded);
		return;
	}

	// ActionStatComponent 확인
	if (::IsValid(ActionStatComponent) == false)
	{
		EnumAddFlags(CharacterState, ECharacterState::AttackEnded);
		return;
	}

	// 스킬 준비 여부 확인
	if (!ActionStatComponent->IsActionReady(EActionSlot::LMB))
	{
		EnumAddFlags(CharacterState, ECharacterState::AttackEnded);
		return;
	}

	// 발사체 및 파티클 설정
	UClass* ProjectileClass = GetOrLoadClass(TEXT("PrimaryProjectile"), TEXT(""));
	UParticleSystem* TrailEffect = GetOrLoadParticle(TEXT("PrimaryTrail"), TEXT(""));

	if (!ProjectileClass || !TrailEffect)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] ProjectileClass or TrailEffect is nullptr."), ANSI_TO_TCHAR(__FUNCTION__));
		EnumAddFlags(CharacterState, ECharacterState::AttackEnded);
		return;
	}

	// 공격 애니메이션 로드
	UAnimMontage* Montage = GetOrLoadMontage("LMB", *FString::Printf(TEXT("/Game/ParagonMinions/Characters/Minions/Down_Minions/Animations/%s/Attack_Montage.Attack_Montage"), *CharacterName.ToString()));
	if (!Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to load or find montage."), ANSI_TO_TCHAR(__FUNCTION__));
		EnumAddFlags(CharacterState, ECharacterState::AttackEnded);
		return;
	}

	FVector SpawnLocation = FVector::ZeroVector;
	if (USkeletalMeshComponent* SkeletalMesh = GetMesh())
	{
		SpawnLocation = SkeletalMesh->DoesSocketExist(FName("Muzzle_Front"))
			? SkeletalMesh->GetSocketLocation(FName("Muzzle_Front"))
			: GetActorLocation() + FVector(0.0f, 83.39f, 18.79f);
	}

	FRotator SpawnRotation = UKismetMathLibrary::MakeRotFromX(Enemy->GetActorLocation() - SpawnLocation);
	FTransform Transform(SpawnRotation, SpawnLocation, FVector(1));

	ActionStatComponent->HandleActionExecution(EActionSlot::LMB, GetWorld()->GetTimeSeconds());
	ActionStatComponent->ActivateActionCooldown(EActionSlot::LMB);

	const FActionAttributes& ActionAttributes = ActionStatComponent->GetActionAttributes(EActionSlot::LMB);

	const float Character_AttackDamage = StatComponent->GetAttackDamage();
	const float BaseAttackDamage = ActionAttributes.AttackDamage;
	const float PhysicalScaling = ActionAttributes.PhysicalScaling;

	const float FinalDamage = BaseAttackDamage + Character_AttackDamage * PhysicalScaling;

	FDamageInformation DamageInformation;
	DamageInformation.ActionSlot = EActionSlot::LMB;
	DamageInformation.AddDamage(EDamageType::Physical, FinalDamage);

	const float MontageDuration = Montage->GetSectionLength(ComboCount - 1);
	const float PlayRate = AdjustAnimPlayRate(MontageDuration);

	PlayAnimMontage(Montage, PlayRate, FName(*FString::Printf(TEXT("Attack%d"), ComboCount)));
	MulticastPlayMontage(Montage, PlayRate, FName(*FString::Printf(TEXT("Attack%d"), ComboCount)));

	AProjectile* Projectile = Cast<AProjectile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), ProjectileClass, Transform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, this));
	if (Projectile)
	{
		const float HomingAcceleration = GetUniqueAttribute(EActionSlot::LMB, TEXT("HomingAcceleration"), 1024.f);
		const float InitialSpeed = GetUniqueAttribute(EActionSlot::LMB, TEXT("InitialSpeed"), 2000.f);
		const float MaxSpeed = GetUniqueAttribute(EActionSlot::LMB, TEXT("MaxSpeed"), 2000.f);

		Projectile->TargetActor = Enemy;
		Projectile->TrailParticleSystem->Template = TrailEffect;
		Projectile->DamageInformation = DamageInformation;

		Projectile->ProjectileMovement->bIsHomingProjectile = true;
		Projectile->ProjectileMovement->HomingTargetComponent = Enemy->HomingTargetSceneComponent;
		Projectile->ProjectileMovement->HomingAccelerationMagnitude = HomingAcceleration;
		Projectile->ProjectileMovement->InitialSpeed = InitialSpeed;
		Projectile->ProjectileMovement->MaxSpeed = MaxSpeed;

		Projectile->ProjectileInteractionType = EProjectileInteractionType::BlockableBySkill;
		
		UGameplayStatics::FinishSpawningActor(Projectile, Transform);

		Projectile->MulticastConfigureProjectile(TrailEffect, Enemy, true, HomingAcceleration, InitialSpeed, MaxSpeed);
	}

	ComboCount = FMath::Clamp<int32>((ComboCount % 4) + 1, 1, MaxComboCount);
}

void ARangedMinion::LMB_CheckHit()
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