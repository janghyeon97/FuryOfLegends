// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SparrowCharacter.h"
#include "Components/StatComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/ActionStatComponent.h"
#include "Game/AOSGameInstance.h"
#include "Game/ArenaPlayerState.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Inputs/InputConfigData.h"
#include "Animations/PlayerAnimInstance.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Math/Vector.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Structs/CustomCombatData.h"
#include "Props/ArrowBase.h"
#include "Engine/OverlapResult.h"
#include "Plugins/UniqueCodeGenerator.h"




ASparrowCharacter::ASparrowCharacter()
{
	bReplicates = true;
	SetReplicateMovement(true);

	StatComponent = CreateDefaultSubobject<UStatComponent>(TEXT("StatComponent"));
	ActionStatComponent = CreateDefaultSubobject<UActionStatComponent>(TEXT("ActionStatComponent"));

	if (HasAuthority())
	{
		StatComponent->SetIsReplicated(true);
		ActionStatComponent->SetIsReplicated(true);
	}

	BowParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("BowParticleSystem"));
	BowParticleSystem->SetAutoActivate(false);
	BowParticleSystem->SetupAttachment(GetMesh());
	BowParticleSystem->SetAutoAttachParams(GetMesh(), FName("BowEmitterSocket"), EAttachLocation::SnapToTarget);

	PrimaryActorTick.bCanEverTick = true;
	SelectedCharacterIndex = 2;
	CharacterName = "Sparrow";

	Ability_Q_Range = 0.0f;
	Ability_Q_DecalLocation = FVector::ZeroVector;
	Ability_LMB_ImpactPoint = FVector::ZeroVector;
}


void ASparrowCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (EnumHasAnyFlags(CharacterState, ECharacterState::Q) && IsLocallyControlled())
	{
		FHitResult ImpactResult = GetImpactPoint(Ability_Q_Range);
		FVector ImpactPoint = ImpactResult.Location;
		FVector TraceStart = FVector(ImpactPoint.X, ImpactPoint.Y, ImpactPoint.Z + 10000.f);
		FVector TraceEnd = FVector(ImpactPoint.X, ImpactPoint.Y, ImpactPoint.Z - 10000.f);

		FHitResult GroundHitResult;
		FCollisionQueryParams CollisionParams(NAME_None, false, this);
		bool bGroundHit = GetWorld()->LineTraceSingleByChannel(
			GroundHitResult,
			TraceStart,
			TraceEnd,
			ECollisionChannel::ECC_WorldStatic,
			CollisionParams
		);

		Ability_Q_DecalLocation = GroundHitResult.Location;

		if (::IsValid(TargetDecalActor))
		{
			TargetDecalActor->SetActorLocation(Ability_Q_DecalLocation);
		}
	}

	if (EnumHasAnyFlags(CharacterState, ECharacterState::RMB))
	{
		ChangeCameraLength(200.f);
	}
	else
	{
		ChangeCameraLength(500.f);
	}

	FString CharacterStateString;
	UEnum* EnumPtr = StaticEnum<ECharacterState>();

	if (EnumPtr)
	{
		for (int32 i = 0; i < EnumPtr->NumEnums() - 1; ++i)
		{
			int64 Value = EnumPtr->GetValueByIndex(i);

			if (EnumHasAnyFlags(CharacterState, static_cast<ECharacterState>(Value)) && Value != static_cast<int64>(ECharacterState::None))
			{
				if (!CharacterStateString.IsEmpty())
				{
					CharacterStateString += TEXT(", "); // 🌟 여러 상태가 있을 경우 쉼표로 구분
				}
				CharacterStateString += EnumPtr->GetNameStringByIndex(i);
			}
		}
	}

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("CharacterState: %s"), *CharacterStateString), true, false, FLinearColor::Green, 2.0f, FName("1"));
}


void ASparrowCharacter::BeginPlay()
{
	Super::BeginPlay();

}

void ASparrowCharacter::PostCharacterSpawn()
{
	Super::PostCharacterSpawn();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!HasAuthority() && PlayerController && PlayerController == UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

		UClass* TargetDecalClass = GetOrLoadClass("TargetDecal", TEXT("/Game/FuryOfLegends/Characters/Sparrow/Blueprints/BP_TargetDecal.BP_TargetDecal"));
		if (TargetDecalClass)
		{
			TargetDecalActor = GetWorld()->SpawnActor<AActor>(TargetDecalClass, FTransform(FRotator(0), Ability_Q_DecalLocation, FVector(1)), SpawnParams);
			if(::IsValid(TargetDecalActor))
			{
				TargetDecalActor->SetActorHiddenInGame(true);
			}
		}

		UParticleSystem* UltimateBuff = GetOrLoadParticle("Ultimate_BowBuff", TEXT("/Game/ParagonSparrow/FX/Particles/Sparrow/Abilities/Ultimate/FX/P_SparrowBuff.P_SparrowBuff"));
		if (UltimateBuff)
		{
			BowParticleSystem->Template = UltimateBuff;
		}
	}
}

void ASparrowCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!HasAuthority() && PlayerController && PlayerController == UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [&]()
			{
				if (::IsValid(EnhancedInputComponent))
				{
					EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->MoveAction, ETriggerEvent::Triggered, this, &ASparrowCharacter::Move);
					EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->LookAction, ETriggerEvent::Triggered, this, &ASparrowCharacter::Look);
					EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->JumpAction, ETriggerEvent::Started, this, &ASparrowCharacter::Jump);
				}

				if (::IsValid(BowParticleSystem))
				{
					BowParticleSystem->AutoAttachSocketName = TEXT("BowEmitterSocket");
				}
			}
		));
	}
}

void ASparrowCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}



void ASparrowCharacter::Move(const FInputActionValue& InValue)
{
	if (EnumHasAnyFlags(CharacterState, ECharacterState::Death))
	{
		return;
	}

	if (!EnumHasAnyFlags(CharacterState, ECharacterState::Move))
	{
		return;
	}

	PreviousForwardInputValue = ForwardInputValue;
	PreviousRightInputValue = RightInputValue;

	ForwardInputValue = InValue.Get<FVector2D>().X;
	RightInputValue = InValue.Get<FVector2D>().Y;

	const FRotator ControlRotation = GetController()->GetControlRotation();
	const FRotator ControlRotationYaw(0.f, ControlRotation.Yaw, 0.f);

	const FVector ForwardVector = FRotationMatrix(ControlRotationYaw).GetUnitAxis(EAxis::X);
	const FVector RightVector = FRotationMatrix(ControlRotationYaw).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardVector, ForwardInputValue);
	AddMovementInput(RightVector, RightInputValue);
}


void ASparrowCharacter::Look(const FInputActionValue& InValue)
{
	FVector2D LookVector = InValue.Get<FVector2D>();
	FRotator AimRotation = GetBaseAimRotation();

	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);

	CurrentAimYaw = AimRotation.Yaw;
	CurrentAimPitch = AimRotation.Pitch;

	UpdateAimValue_Server(CurrentAimPitch, CurrentAimYaw);
}

/*
	Q 함수는 캐릭터의 Q 스킬을 실행합니다.
	ActionStatComponent가 null이거나 Ctrl 키가 눌려 있으면 스킬을 사용할 수 없습니다.

	- UniqueValue[0]: Duration		지속시간
	- UniqueValue[1]: Range			사거리

	1. ActionStatComponent가 유효한지 확인합니다.
	2. Ctrl 키가 눌려 있는지 확인합니다.
	3. 스킬이 준비되었고, 캐릭터가 행동을 전환할 수 있는 상태인지 확인합니다.
	4. 조건이 충족되면 캐릭터 상태를 업데이트하고 스킬의 고유 값을 가져옵니다.
	5. 애니메이션 몽타주를 재생하고, 스킬의 히트 체크를 수행합니다.
	6. 조건이 충족되지 않으면 스킬이 준비되지 않았음을 알립니다.
*/
void ASparrowCharacter::Q_Started()
{
	// 서버에서 실행
	if (HasAuthority())
	{
		if (EnumHasAnyFlags(CharacterState, ECharacterState::SwitchAction) == false)
		{
			return;
		}

		OnSwitchActionStateEnded.AddDynamic(this, &ThisClass::RestoreSwitchActionState);
		ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::SwitchAction);
		ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::Q);
		return;
	}

	// 클라이언트에서 실행
	if (EnumHasAnyFlags(CharacterState, ECharacterState::SwitchAction) == false)
	{
		return;
	}

	UAnimMontage* Montage = GetOrLoadMontage("Q", TEXT("/Game/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Ability_Q_Montage.Ability_Q_Montage"));
	if (!Montage)
	{
		return;
	}

	Ability_Q_Range = ActionStatComponent->GetActionAttributes(EActionSlot::Q).Range;
	if (::IsValid(TargetDecalActor))
	{
		TargetDecalActor->SetActorHiddenInGame(false);
	}

	PlayAnimMontage(Montage, 1.0f);
	ServerPlayMontage(Montage, 1.0f);
}

void ASparrowCharacter::Q_Released()
{
	if (EnumHasAnyFlags(CharacterState, ECharacterState::Q) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: Character is not in Q state."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (::IsValid(TargetDecalActor))
	{
		TargetLocations[EActionSlot::Q] = TargetDecalActor->GetActorLocation();
		TargetDecalActor->SetActorHiddenInGame(true);
	}
}



void ASparrowCharacter::Q_Executed()
{
	if (HasAuthority() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: No Authority."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (EnumHasAnyFlags(CharacterState, ECharacterState::Q) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: Ability usage not valid or not in Q state."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	UAnimMontage* Montage = GetOrLoadMontage("Q", TEXT("/Game/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Ability_Q_Montage.Ability_Q_Montage"));
	if (!Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: Montage not found or uninitialized."), ANSI_TO_TCHAR(__FUNCTION__));
		OnSwitchActionStateEnded.RemoveAll(this);
		ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::SwitchAction);
		ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::Q);
		return;
	}

	const FActionAttributes& ActionAttributes = ActionStatComponent->GetActionAttributes(EActionSlot::Q);
	if (ActionAttributes.Name.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: ActionAttributes for Q slot not found or uninitialized."), ANSI_TO_TCHAR(__FUNCTION__));
		OnSwitchActionStateEnded.RemoveAll(this);
		ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::SwitchAction);
		ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::Q);
		return;
	}

	FVector TargetLocation = FVector::ZeroVector;
	if (TargetLocations.Contains(EActionSlot::Q) && !TargetLocations[EActionSlot::Q].IsNearlyZero(0.01))
	{
		TargetLocation = TargetLocations[EActionSlot::Q];
	}
	else
	{
		FHitResult HitResult = SweepTraceFromAimAngles(ActionAttributes.Range);
		FCollisionQueryParams CollisionParams(NAME_None, false, this);
		if (GetWorld()->LineTraceSingleByChannel(HitResult, FVector(HitResult.Location.X, HitResult.Location.Y, HitResult.Location.Z + 10000.f), FVector(HitResult.Location.X, HitResult.Location.Y, HitResult.Location.Z - 10000.f), ECC_WorldStatic, CollisionParams))
		{
			TargetLocation = HitResult.Location;
			TargetLocations.Add(EActionSlot::Q, TargetLocation);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to find target ground location."), ANSI_TO_TCHAR(__FUNCTION__));
			return;
		}
	}

	const float Distance = FVector::Dist2D(GetActorLocation(), TargetLocation);
	if (Distance + 200.f > ActionAttributes.Range)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Target distance (%.2f) exceeds ability range (%.2f)."), ANSI_TO_TCHAR(__FUNCTION__), Distance, ActionAttributes.Range);
		return;
	}

	float RemainingTime = 0.f;
	if (AnimInstance->Montage_IsActive(Montage))
	{
		const float CurrentPosition = AnimInstance->Montage_GetPosition(Montage);
		RemainingTime = Montage->GetSectionLength(0) - CurrentPosition;
	}

	// 화살 발사 모션이 끝날 때까지 딜레이
	FTimerDelegate RainOfArrowsDelegate;
	RainOfArrowsDelegate.BindLambda([this, Montage]()
		{
			HandleRainOfArrows(Montage);
		});

	FTimerHandle DelayTimer;
	GetWorldTimerManager().SetTimer(DelayTimer, RainOfArrowsDelegate, FMath::Max(RemainingTime, 0.01f), false, FMath::Max(RemainingTime, -1.0f));
}


void ASparrowCharacter::HandleRainOfArrows(UAnimMontage* Montage)
{
	FVector ArrowAnchorLocation = GetMesh()->GetSocketLocation(TEXT("arrow_anchor"));
	FVector ArrowLocation = GetMesh()->GetSocketLocation(TEXT("Arrow"));

	if (ArrowAnchorLocation.IsZero() || ArrowLocation.IsZero())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid socket locations for arrow creation."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	FRotator SpawnRotation = UKismetMathLibrary::MakeRotFromX(ArrowAnchorLocation - ArrowLocation);
	SpawnRotation.Pitch = SpawnRotation.Pitch + 60.f;
	FTransform SpawnTransform = FTransform(SpawnRotation, ArrowAnchorLocation, FVector(1));

	ServerPlayMontage(Montage, 1.0f, TEXT("Fire"), true);

	// 화살 클래스를 로드하고 유효성 확인
	UClass* BasicArrowClass = GetOrLoadClass(TEXT("BasicArrow"), TEXT("/Game/FuryOfLegends/Characters/Sparrow/Blueprints/BP_Arrow.BP_Arrow"));
	if (!BasicArrowClass)
	{
		return;
	}

	AArrowBase* NewArrow = Cast<AArrowBase>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), BasicArrowClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, this));
	if (NewArrow != nullptr)
	{
		NewArrow->ArrowProjectileMovement->InitialSpeed = 5000.f;
		NewArrow->ArrowProjectileMovement->MaxSpeed = 5000.f;

		UGameplayStatics::FinishSpawningActor(NewArrow, SpawnTransform);
	}

	ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::Q);
	ActionStatComponent->HandleActionExecution(EActionSlot::Q, GetWorld()->GetTimeSeconds());
	ActionStatComponent->ActivateActionCooldown(EActionSlot::Q);
}



/**
 * @brief Rain of Arrows(Q) 스킬의 효과를 처리하는 함수입니다.
 *
 * 정해진 위치(TargetLocation)에 파티클을 재생하고, 재생 시간 동안 일정 간격으로 파티클 범위 내 대상을 검색하여 데미지를 적용합니다.
 * 총 데미지는 MaxExplosions 횟수만큼 나누어대상에게 단계적으로 적용됩니다.
 *
 * 1. 지정된 위치에 파티클을 재생합니다.
 * 2. 일정 시간 간격으로 파티클 내 범위에 있는 적 캐릭터를 검색합니다.
 * 3. 적 캐릭터에게 데미지를 나누어 적용하며, 아군은 영향을 받지 않습니다.
 * 4. 최대 폭발 횟수(MaxExplosions)만큼 반복한 후 효과를 종료합니다.
 */
void ASparrowCharacter::Q_CheckHit()
{
	if (!HasAuthority())
	{
		return;
	}

	if (::IsValid(ArenaPlayerState) == false)
	{
		return;
	}

	const FActiveActionState& ActiveAbilityState = ActionStatComponent->GetActiveActionState(EActionSlot::Q);
	const FActionAttributes& ActionAttributes = ActionStatComponent->GetActionAttributes(EActionSlot::Q);

	if (!ActionAttributes.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] ActionAttributes is invalid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	UParticleSystem* RainOfArrowsEffect = GetOrLoadParticle(TEXT("RainOfArrows"), TEXT("/Game/ParagonSparrow/FX/Particles/Sparrow/Abilities/RainOfArrows/FX/P_RainofArrows.P_RainofArrows"));
	if (!RainOfArrowsEffect)
	{
		return;
	}

	// 스킬 효과 위치 표시
	SpawnEmitterAtLocation(RainOfArrowsEffect, FTransform(FRotator(0), TargetLocations[EActionSlot::Q], FVector(1)));

	int32 PlayerIndex = ArenaPlayerState->GetPlayerIndex();
	const uint32 UniqueCode = UUniqueCodeGenerator::GenerateUniqueCode(ObjectType, PlayerIndex, ETimerCategory::Action, static_cast<uint8>(EActionSlot::Q), static_cast<uint8>(FPlatformTime::Seconds() * 1000) % 256);
	const ECollisionChannel CollisionChannel = ActiveAbilityState.CollisionDetection;
	const float ExplosionRadius = GetUniqueAttribute(EActionSlot::Q, "ExplosionRadius", 400.f);
	const int MaxExplosions = GetUniqueAttribute(EActionSlot::Q, "MaxHitCount", 10);

	const float PlayerAttackDamage = StatComponent->GetAttackDamage();
	const float PlayerAbilityPower = StatComponent->GetAbilityPower();
	const float BaseAttackDamage = ActionAttributes.AttackDamage;
	const float BaseAbilityPower = ActionAttributes.AbilityDamage;
	const float AttackScaling = ActionAttributes.PhysicalScaling;
	const float MagicScaling = ActionAttributes.MagicalScaling;

	const float TotalDamage = (BaseAttackDamage + PlayerAttackDamage * AttackScaling) + (BaseAbilityPower + PlayerAbilityPower * MagicScaling);

	// 스마트 포인터로 상태 관리
	ExplosionCounts.Add(UniqueCode, 0);
	

	FTimerDelegate ExplosionCallback;
	ExplosionCallback.BindLambda([this, WeakPlayerState = TWeakObjectPtr<AArenaPlayerState>(ArenaPlayerState), TargetLocation = TargetLocations[EActionSlot::Q], Index = ExplosionCounts.Num(), UniqueCode, CollisionChannel, ExplosionRadius, TotalDamage, MaxExplosions]()
		{
			if (!WeakPlayerState.IsValid())
			{
				return;
			}

			// 📌 MaxExplosions이 0이면 데미지 연산 방지
			if (MaxExplosions <= 0)
			{
				UE_LOG(LogTemp, Error, TEXT("[%s] MaxExplosions must be greater than 0!"), ANSI_TO_TCHAR(__FUNCTION__));
				WeakPlayerState->ClearTimer(UniqueCode);
				return;
			}

			TArray<FOverlapResult> OverlapResults;
			FCollisionQueryParams QueryParams(NAME_None, false, this);

			// 범위 내 대상 감지 실패 시 조기 반환
			if (!GetWorld()->OverlapMultiByChannel(OverlapResults, TargetLocation, FQuat::Identity, CollisionChannel, FCollisionShape::MakeSphere(ExplosionRadius), QueryParams))
			{
				return;
			}

			for (const auto& OverlapResult : OverlapResults)
			{
				ACharacterBase* OverlappedCharacter = Cast<ACharacterBase>(OverlapResult.GetActor());
				if (::IsValid(OverlappedCharacter) == false)
				{
					continue;
				}

				if (OverlappedCharacter->TeamSide == this->TeamSide)
				{
					continue;
				}

				FDamageInformation DamageInformation;
				DamageInformation.ActionSlot = EActionSlot::Q;
				DamageInformation.AddDamage(EDamageType::Physical, TotalDamage / MaxExplosions);
				DamageInformation.AddTrigger(EAttackTrigger::AbilityEffects);

				ServerApplyDamage(OverlappedCharacter, this, GetController(), DamageInformation);
			}

			//  UniqueCode가 존재하지 않으면 ClearTimer 후 조기 반환
			if (!ExplosionCounts.Contains(UniqueCode))
			{
				WeakPlayerState->ClearTimer(UniqueCode);
				return;
			}

			// UniqueCode 증가 (0에서 시작 보장)
			ExplosionCounts[UniqueCode]++;

			// 최대 폭발 횟수 도달 시 타이머 제거 및 데이터 정리
			if (ExplosionCounts[UniqueCode] >= MaxExplosions)
			{
				WeakPlayerState->ClearTimer(UniqueCode);
				ExplosionCounts.Remove(UniqueCode);
				return;
			}
		});

	// 타이머 설정
	const float ExplosionInterval = 3.0f / static_cast<float>(MaxExplosions);
	ArenaPlayerState->SetTimer(UniqueCode, ExplosionCallback, ExplosionInterval, true, 0.1f);
}


void ASparrowCharacter::E_Executed()
{
	if (HasAuthority() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: No Authority."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (::IsValid(ArenaPlayerState) == false)
	{
		return;
	}

	float Duration = GetUniqueAttribute(EActionSlot::E, "Duration", 4.0f);
	const float BonusAttackSpeed = GetUniqueAttribute(EActionSlot::E, "BonusAttackSpeed", 0.f);
	const float BonusFlatMovementSpeed = GetUniqueAttribute(EActionSlot::E, "BonusFlatMovementSpeed", 0.f);
	const float BonusPercentMovementSpeed = GetUniqueAttribute(EActionSlot::E, "BonusPercentMovementSpeed", 0.f);

	ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::E);

	int32 PlayerIndex = ArenaPlayerState->GetPlayerIndex();
	uint32 UniqueCode = UUniqueCodeGenerator::GenerateUniqueCode(ObjectType, PlayerIndex, ETimerCategory::Action, static_cast<uint8>(EActionSlot::E), 1);
	if (ArenaPlayerState->IsTimerActive(UniqueCode))
	{
		float RemainingTime = ArenaPlayerState->GetTimerRemaining(UniqueCode);
		ArenaPlayerState->ClearTimer(UniqueCode);
		Duration += RemainingTime;
	}
	else
	{
		// 처음에만 스탯을 변경
		StatComponent->ModifyAccumulatedPercentAttackSpeed(BonusAttackSpeed);
		StatComponent->ModifyAccumulatedFlatMovementSpeed(BonusFlatMovementSpeed);
		StatComponent->ModifyAccumulatedPercentMovementSpeed(BonusPercentMovementSpeed);
	}

	auto EndLambda = [this, UniqueCode, BonusAttackSpeed, BonusFlatMovementSpeed, BonusPercentMovementSpeed]()
		{
			// 능력 종료 시 스탯 복원
			ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::E);

			StatComponent->ModifyAccumulatedPercentAttackSpeed(-BonusAttackSpeed);
			StatComponent->ModifyAccumulatedFlatMovementSpeed(-BonusFlatMovementSpeed);
			StatComponent->ModifyAccumulatedPercentMovementSpeed(-BonusPercentMovementSpeed);

			ArenaPlayerState->ClearTimer(UniqueCode);
		};

	ArenaPlayerState->SetTimer(UniqueCode, EndLambda, Duration, false, Duration, true);
	ActionStatComponent->HandleActionExecution(EActionSlot::E, GetWorld()->GetTimeSeconds());
	ActionStatComponent->ActivateActionCooldown(EActionSlot::E);
}



void ASparrowCharacter::R_Executed()
{
	if (HasAuthority() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: No Authority."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (::IsValid(ArenaPlayerState) == false)
	{
		return;
	}
	
	int32 PlayerIndex = ArenaPlayerState->GetPlayerIndex();
	const float Duration = GetUniqueAttribute(EActionSlot::R, "Duration", 4.0f);
	uint32 UniqueCode = UUniqueCodeGenerator::GenerateUniqueCode(ObjectType, PlayerIndex, ETimerCategory::Action, static_cast<uint8>(EActionSlot::R), 1);

	if (::IsValid(BowParticleSystem))
	{
		BowParticleSystem->Activate();
	}

	ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::R);

	auto EndLambda = [this, UniqueCode]()
		{
			if (::IsValid(BowParticleSystem))
			{
				BowParticleSystem->Deactivate();
			}

			ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::R);
			ArenaPlayerState->ClearTimer(UniqueCode);
		};

	ArenaPlayerState->SetTimer(UniqueCode, EndLambda, Duration, false, Duration, true);

	ActionStatComponent->HandleActionExecution(EActionSlot::R, GetWorld()->GetTimeSeconds());
	ActionStatComponent->ActivateActionCooldown(EActionSlot::R);
}



/**
 * LMB 능력을 실행합니다.
 * Ctrl 키가 눌려있거나 필요한 컴포넌트가 유효하지 않은 경우 능력을 사용할 수 없습니다.
 * 능력이 준비되고 캐릭터 상태가 SwitchAction을 포함하는 경우 능력을 실행합니다.
 * R 상태에서는 궁극기 화살을 발사하고, 그렇지 않으면 기본 화살을 발사합니다.
 */
void ASparrowCharacter::LMB_Executed()
{
	if (HasAuthority() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: No Authority."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (EnumHasAnyFlags(CharacterState, ECharacterState::SwitchAction) == false)
	{
		ActionStatComponent->ClientNotifyAlertTextChanged("You cannot use this right now.");
		return;
	}

	const float AttackSpeed = StatComponent->GetAttackSpeed();
	UAnimMontage* MontageToPlay = GetMontageBasedOnAttackSpeed(AttackSpeed);
	if (!MontageToPlay)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: Montage not found or uninitialized."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::SwitchAction);
	OnSwitchActionStateEnded.AddDynamic(this, &ThisClass::RestoreSwitchActionState);

	BasicAttackAnimLength = MontageToPlay->GetPlayLength();
	BasicAttackAnimPlayRate = AdjustAnimPlayRate(BasicAttackAnimLength);
	ServerPlayMontage(MontageToPlay, BasicAttackAnimPlayRate, NAME_None, true);
	
	if (EnumHasAnyFlags(CharacterState, ECharacterState::R))
	{
		ExecuteUltimateAction();
	}
	else
	{
		ExecutePrimaryAction();
	}
	
	// 능력 사용 및 쿨타임 시작
	ActionStatComponent->HandleActionExecution(EActionSlot::LMB, GetWorld()->GetTimeSeconds());
	ActionStatComponent->ActivateActionCooldown(EActionSlot::LMB);
}

void ASparrowCharacter::ExecutePrimaryAction()
{
	UClass* BasicArrowClass = GetOrLoadClass("BasicArrow", TEXT("/Game/FuryOfLegends/Characters/Sparrow/Blueprints/BP_Arrow.BP_Arrow"));
	if (!BasicArrowClass)
	{
		return;
	}

	const FActionAttributes& ActionAttributes = ActionStatComponent->GetActionAttributes(EActionSlot::LMB);
	const FActiveActionState& ActiveAbilityState = ActionStatComponent->GetActiveActionState(EActionSlot::LMB);

	FArrowProperties ArrowProperties;
	ArrowProperties.bIsHoming	= true;
	ArrowProperties.MaxRange	= ActionAttributes.Range;
	ArrowProperties.Detection	= ActiveAbilityState.CollisionDetection;
	ArrowProperties.TargetActor = ::IsValid(CurrentTarget) ? CurrentTarget : nullptr;

	ArrowProperties.MaxSpeed			= GetUniqueAttribute(EActionSlot::LMB, "MaxSpeed", 6500.f);
	ArrowProperties.InitialSpeed		= GetUniqueAttribute(EActionSlot::LMB, "InitialSpeed", 6500.f);
	ArrowProperties.HomingAcceleration	= GetUniqueAttribute(EActionSlot::LMB, "HomingAcceleration", 20000.f);
	ArrowProperties.CollisionRadius		= GetUniqueAttribute(EActionSlot::LMB, "CollisionRadius", 20.f);

	const float CharacterAD = StatComponent->GetAttackDamage();
	const float CharacterAP = StatComponent->GetAbilityPower();
	const float FinalDamage = (ActionAttributes.AttackDamage + CharacterAD * ActionAttributes.PhysicalScaling)
		+ (ActionAttributes.AbilityDamage + CharacterAP * ActionAttributes.MagicalScaling);

	FDamageInformation DamageInformation;
	DamageInformation.ActionSlot = EActionSlot::LMB;
	DamageInformation.AddDamage(EDamageType::Physical, FinalDamage);
	DamageInformation.AddTrigger(EAttackTrigger::OnHit);
	DamageInformation.AddTrigger(EAttackTrigger::OnAttack);

	// 타겟팅 및 임팩트 포인트 계산
	FHitResult ImpactResult = SweepTraceFromAimAngles(ActionAttributes.Range);
	Ability_LMB_ImpactPoint = ProcessImpactPoint(ImpactResult);

	// 화살의 스폰 위치 및 방향 계산
	FVector SpawnLocation = GetMesh()->GetSocketLocation(FName("Arrow"));
	FRotator SpawnRotation = UKismetMathLibrary::MakeRotFromX(Ability_LMB_ImpactPoint - SpawnLocation);
	SpawnRotation.Normalize();
	FTransform SpawnTransform(SpawnRotation, SpawnLocation, FVector(1));

	ServerSpawnArrow(BasicArrowClass, SpawnTransform, ArrowProperties, DamageInformation);
}

void ASparrowCharacter::ExecuteUltimateAction()
{
	UClass* UltimateArrowClass = GetOrLoadClass("UltimateArrow", TEXT("/Game/FuryOfLegends/Characters/Sparrow/Blueprints/BP_UltimateArrow.BP_UltimateArrow_C"));
	if (!UltimateArrowClass)
	{
		return;
	}

	const FActionAttributes& ActionAttributes = ActionStatComponent->GetActionAttributes(EActionSlot::R);
	const FActiveActionState& ActiveAbilityState = ActionStatComponent->GetActiveActionState(EActionSlot::R);

	FArrowProperties ArrowProperties;
	ArrowProperties.bIsHoming	= false;
	ArrowProperties.MaxRange	= ActionAttributes.Range;
	ArrowProperties.Detection	= ActiveAbilityState.CollisionDetection;

	ArrowProperties.MaxSpeed		= GetUniqueAttribute(EActionSlot::R, "MaxSpeed", 6500.f);
	ArrowProperties.InitialSpeed	= GetUniqueAttribute(EActionSlot::R, "InitialSpeed", 6500.f);
	ArrowProperties.CollisionRadius = GetUniqueAttribute(EActionSlot::R, "CollisionRadius", 50.f);
	ArrowProperties.ExplosionRadius = GetUniqueAttribute(EActionSlot::R, "ExplosionRadius", 300.f);

	const float CharacterAD = StatComponent->GetAttackDamage();
	const float CharacterAP = StatComponent->GetAbilityPower();
	const float FinalDamage = (ActionAttributes.AttackDamage + CharacterAD * ActionAttributes.PhysicalScaling)
		+ (ActionAttributes.AbilityDamage + CharacterAP * ActionAttributes.MagicalScaling);

	FDamageInformation DamageInformation;
	DamageInformation.ActionSlot = EActionSlot::R;
	DamageInformation.AddPhysicalDamage(FinalDamage);
	DamageInformation.AddTrigger(EAttackTrigger::AbilityEffects);

	// 궁극기 화살 설정
	const float SideDamage = GetUniqueAttribute(EActionSlot::R, "SideArrowsDamage", 55.f);
	const float Angle = GetUniqueAttribute(EActionSlot::R, "SideArrowsAngle", 10.f);

	// 타겟팅 및 임팩트 포인트 계산
	FHitResult ImpactResult = SweepTraceFromAimAngles(ActionAttributes.Range);

	FVector SpawnLocation = GetMesh()->GetSocketLocation(FName("Arrow"));
	FRotator SpawnRotation = UKismetMathLibrary::MakeRotFromX(ImpactResult.Location - SpawnLocation);
	SpawnRotation.Normalize();
	FTransform SpawnTransform(SpawnRotation, SpawnLocation, FVector(1));

	// 정면 화살 생성
	ServerSpawnArrow(UltimateArrowClass, SpawnTransform, ArrowProperties, DamageInformation);

	// 좌우 각도로 회전만 조정하면서 화살 생성
	SpawnTransform.SetRotation((SpawnRotation + FRotator(0, -Angle, 0)).Quaternion());
	DamageInformation.PhysicalDamage *= SideDamage; // 좌측 화살에 적용할 데미지 조정
	ServerSpawnArrow(UltimateArrowClass, SpawnTransform, ArrowProperties, DamageInformation);

	SpawnTransform.SetRotation((SpawnRotation + FRotator(0, Angle, 0)).Quaternion());
	ServerSpawnArrow(UltimateArrowClass, SpawnTransform, ArrowProperties, DamageInformation);
}


FVector ASparrowCharacter::ProcessImpactPoint(const FHitResult& ImpactResult)
{
	AActor* HitActor = ImpactResult.GetActor();
	if (!HitActor)
	{
		return ImpactResult.Location;
	}

	// Skeletal Mesh 컴포넌트 확인
	USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(HitActor->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
	if (!SkeletalMeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Skeletal Mesh Component found on Actor: %s"), *HitActor->GetName());
		return ImpactResult.ImpactPoint;
	}

	// FHitResult의 BoneName 사용
	if (ImpactResult.BoneName != NAME_None)
	{
		return SkeletalMeshComponent->GetBoneLocation(ImpactResult.BoneName);
	}

	return ImpactResult.Location;
}



UAnimMontage* ASparrowCharacter::GetMontageBasedOnAttackSpeed(float AttackSpeed)
{
	UAnimMontage* Montage = nullptr;

	if (EnumHasAnyFlags(CharacterState, ECharacterState::R))
	{
		if (AttackSpeed < 1.f)
		{
			Montage = GetOrLoadMontage("LMB_UltimateMode_Slow", TEXT("/Game/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Ability_LMB_UltimateMode_Slow.Ability_LMB_UltimateMode_Slow"));
		}
		else if (AttackSpeed <= 2.0f)
		{
			Montage = GetOrLoadMontage("LMB_UltimateMode_Med", TEXT("/Game/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Ability_LMB_UltimateMode_Med.Ability_LMB_UltimateMode_Med"));
		}
		else
		{
			Montage = GetOrLoadMontage("LMB_UltimateMode_Fast", TEXT("/Game/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Ability_LMB_UltimateMode_Fast.Ability_LMB_UltimateMode_Fast"));
		}
	}
	else
	{
		if (AttackSpeed < 1.f)
		{
			Montage = GetOrLoadMontage("LMB_Slow", TEXT("/Game/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Primary_Fire_Slow_Montage.Primary_Fire_Slow_Montage"));
		}
		else if (AttackSpeed <= 2.0f)
		{
			Montage = GetOrLoadMontage("LMB_Med", TEXT("/Game/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Primary_Fire_Med_Montage.Primary_Fire_Med_Montage"));
		}
		else
		{
			Montage = GetOrLoadMontage("LMB_Fast", TEXT("/Game/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Primary_Fire_Fast_Montage.Primary_Fire_Fast_Montage"));
		}
	}

	return Montage;
}



/*
	1. UniqueVale[0]: Range		화살 사거리
	2. UniqueVale[0]: Speed		화살 속도
*/
void ASparrowCharacter::RMB_Started()
{
	if (HasAuthority())
	{
		OnSwitchActionStateEnded.AddDynamic(this, &ThisClass::RestoreSwitchActionState);
		ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::SwitchAction);
		ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::RMB);
		return;
	}

	if (EnumHasAnyFlags(CharacterState, ECharacterState::SwitchAction) == false)
	{
		ActionStatComponent->OnAlertTextChanged.Broadcast("You cannot use this right now.");
		return;
	}

	UAnimMontage* Montage = GetOrLoadMontage("RMB", TEXT("/Game/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Ability_RMB_Montage.Ability_RMB_Montage"));
	if (!Montage)
	{
		return;
	}	

	PlayAnimMontage(Montage, 1.0f);
	ServerPlayMontage(Montage, 1.0f);
}


/**
 * RMB 능력을 실행합니다.
 * 캐릭터가 죽었거나 이미 우클릭 능력을 사용하는 경우 능력을 실행하지 않습니다.
 * 능력이 유효한 경우 화살을 발사하고 데미지를 계산하여 적용합니다.
 *
 * [Ability RMB]
 * 1. UniqueAttribute[0]: ArrowSpeed   화살 속도
 * 2. UniqueAttribute[1]: Range        화살 사거리
 */
void ASparrowCharacter::RMB_Executed()
{
	if (HasAuthority() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: No Authority."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (EnumHasAnyFlags(CharacterState, ECharacterState::RMB) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: Character is not in RMB state."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	UAnimMontage* Montage = GetOrLoadMontage("RMB", TEXT("/Game/ParagonSparrow/Characters/Heroes/Sparrow/Animations/Ability_RMB_Montage.Ability_RMB_Montage"));
	if (!Montage)
	{
		ServerStopAllMontages(0.4f, true);
		ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::SwitchAction);
		ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::RMB);
		return;
	}

	float ChargeTimeThreshold = GetUniqueAttribute(EActionSlot::RMB, TEXT("ChargeTimeThreshold"), 1.0f);
	if (KeyElapsedTimes.Contains(EActionSlot::RMB) && KeyElapsedTimes[EActionSlot::RMB] <= ChargeTimeThreshold)
	{
		ServerStopMontage(0.4f, Montage, true);
		ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::SwitchAction);
		ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::RMB);

		ActionStatComponent->ActivateActionCooldown(EActionSlot::RMB);
		ActionStatComponent->ActiveActionState_RMB.MaxCooldown = 1.0f;
		ActionStatComponent->ActiveActionState_RMB.Cooldown = 1.0f;
		return;
	}

	// 능력 스탯 테이블을 가져옵니다.
	const FActionAttributes& ActionAttributes = ActionStatComponent->GetActionAttributes(EActionSlot::RMB);
	const FActiveActionState& ActiveAbilityState = ActionStatComponent->GetActiveActionState(EActionSlot::RMB);
	if (!ActionAttributes.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[ASparrowCharacter::Ability_RMB_Fire] ActionAttributes is null."));
		ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::SwitchAction);
		ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::RMB);
		ServerStopMontage(0.4f, Montage, true);
		return;
	}

	FHitResult ImpactResult = SweepTraceFromAimAngles(ActionAttributes.Range > 0 ? ActionAttributes.Range : 10000.f);

	// 화살 스폰 위치와 회전을 계산합니다.
	FVector ArrowSpawnLocation = GetMesh()->GetSocketLocation(FName("Arrow"));
	FRotator ArrowSpawnRotation = UKismetMathLibrary::MakeRotFromX(ImpactResult.Location - ArrowSpawnLocation);
	FTransform ArrowSpawnTransform(ArrowSpawnRotation, ArrowSpawnLocation, FVector(1));

	// 캐릭터의 공격력과 능력 파워를 가져옵니다.
	const float Character_AttackDamage = StatComponent->GetAttackDamage();
	const float Character_AbilityPower = StatComponent->GetAbilityPower();

	// 능력의 기본 공격력과 능력 파워를 가져옵니다.
	const float BaseAttackDamage = ActionAttributes.AttackDamage;
	const float BaseAbilityPower = ActionAttributes.AbilityDamage;
	const float PhysicalScaling = ActionAttributes.PhysicalScaling;
	const float MagicalScaling = ActionAttributes.MagicalScaling;

	// 최종 데미지를 계산합니다.
	const float FinalDamage = (BaseAttackDamage + Character_AttackDamage * PhysicalScaling) + (BaseAbilityPower + Character_AbilityPower * MagicalScaling);

	// 화살의 속성과 데미지 정보를 설정합니다.
	FArrowProperties ArrowProperties;
	ArrowProperties.MaxRange = ActionAttributes.Range;
	ArrowProperties.Detection = ActiveAbilityState.CollisionDetection;

	ArrowProperties.InitialSpeed = GetUniqueAttribute(EActionSlot::RMB, "InitialSpeed", 6500.f);
	ArrowProperties.MaxSpeed = GetUniqueAttribute(EActionSlot::RMB, "MaxSpeed", 6500.f);
	ArrowProperties.MaxPierceCount = GetUniqueAttribute(EActionSlot::RMB, "PierceCount", 3);
	ArrowProperties.DamageReductionPerPierce = GetUniqueAttribute(EActionSlot::RMB, "DamageReduction", 10);
	ArrowProperties.CollisionRadius = GetUniqueAttribute(EActionSlot::RMB, "CollisionRadius", 20.f);

	FDamageInformation DamageInformation;
	DamageInformation.ActionSlot = EActionSlot::RMB;
	DamageInformation.AddDamage(EDamageType::Magic, FinalDamage);
	DamageInformation.AddTrigger(EAttackTrigger::AbilityEffects);

	// 애니메이션을 재생합니다.
	ServerPlayMontage(Montage, 1.0f, TEXT("Fire"	), true);

	UClass* PiercingArrowClass = GetOrLoadClass("PiercingArrow", TEXT("/Game/FuryOfLegends/Characters/Sparrow/Blueprints/BP_PiercingArrow.BP_PiercingArrow"));
	if (PiercingArrowClass)
	{
		ServerSpawnArrow(PiercingArrowClass, ArrowSpawnTransform, ArrowProperties, DamageInformation);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Cannot spawn arrow because PiercingArrowClass is null."), ANSI_TO_TCHAR(__FUNCTION__));
		ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::SwitchAction);
		ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::RMB);
		ServerStopMontage(0.4f, Montage, true);

		return;
	}

	// 능력 사용 및 쿨타임 시작
	ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::RMB);
	ActionStatComponent->HandleActionExecution(EActionSlot::RMB, GetWorld()->GetTimeSeconds());
	ActionStatComponent->ActivateActionCooldown(EActionSlot::RMB);
}


void ASparrowCharacter::CancelAction()
{
	if (EnumHasAnyFlags(CharacterState, ECharacterState::Q))
	{
		Q_Canceled();
	}

	if (EnumHasAnyFlags(CharacterState, ECharacterState::E))
	{
		E_Canceled();
	}

	if (EnumHasAnyFlags(CharacterState, ECharacterState::R))
	{
		R_Canceled();
	}

	if (EnumHasAnyFlags(CharacterState, ECharacterState::LMB))
	{
		LMB_Canceled();
	}

	if (EnumHasAnyFlags(CharacterState, ECharacterState::RMB))
	{
		RMB_Canceled();
	}
}

void ASparrowCharacter::Q_Canceled()
{
	ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::Q);

	if (::IsValid(TargetDecalActor))
	{
		TargetDecalActor->SetActorHiddenInGame(true);
	}
}


void ASparrowCharacter::RMB_Canceled()
{
	ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::RMB);
}


void ASparrowCharacter::ChangeCameraLength(float TargetLength)
{
	float CurrentArmLength = SpringArmComponent->TargetArmLength;

	//InterpolationAlpha = FMath::Clamp(RotationTimer / AnimLength, 0.f, 1.f);
	SpringArmComponent->TargetArmLength = FMath::InterpEaseInOut(CurrentArmLength, TargetLength, GetWorld()->GetDeltaSeconds(), 0.8);
}


void ASparrowCharacter::ServerSpawnArrow_Implementation(UClass* SpawnArrowClass, FTransform SpawnTransform, FArrowProperties InArrowProperties, FDamageInformation InDamageInfomation)
{
	if (SpawnArrowClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[Server] ASparrowCharacter::ServerSpawnArrow - Spawn arrow class is nullptr."));
		return;
	}

	AArrowBase* NewArrow = Cast<AArrowBase>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), SpawnArrowClass, SpawnTransform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, this));
	if (NewArrow != nullptr)
	{
		NewArrow->InitializeArrow(InArrowProperties, InDamageInfomation);
		UGameplayStatics::FinishSpawningActor(NewArrow, SpawnTransform);
	}
}

void ASparrowCharacter::ExecuteSomethingSpecial()
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("[ASparrowCharacter::ExecuteSomethingSpecial] World is null."));
		return;
	}

	UKismetSystemLibrary::PrintString(GetWorld(), TEXT("[ASparrowCharacter::ExecuteSomethingSpecial] ExecuteSomethingSpecial function called."), true, true, FLinearColor::Red, 2.0f, NAME_None);

	FCrowdControlInformation CrowdControlInformation;
	CrowdControlInformation.Type = ECrowdControl::Slow;
	CrowdControlInformation.Percent = 90;
	CrowdControlInformation.Duration = 2.0f;

	ServerApplyCrowdControl(this, this, GetController(), CrowdControlInformation);
}


bool ASparrowCharacter::ValidateAbilityUsage()
{
	if (EnumHasAnyFlags(CharacterState, ECharacterState::Death))
	{
		return false;
	}

	if (bCtrlKeyPressed)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Ability cannot be used because the Ctrl key is pressed."), ANSI_TO_TCHAR(__FUNCTION__));
		return false;
	}

	if (::IsValid(StatComponent) == false || ::IsValid(ActionStatComponent) == false || ::IsValid(AnimInstance) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] StatComponent, ActionStatComponent, or AnimInstance is null."), ANSI_TO_TCHAR(__FUNCTION__));
		return false;
	}

	return true;
}


void ASparrowCharacter::MontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	Super::MontageEnded(Montage, bInterrupted);


}

void ASparrowCharacter::OnRep_CharacterStateChanged()
{
	Super::OnRep_CharacterStateChanged();

	if (::IsValid(BowParticleSystem))
	{
		EnumHasAnyFlags(CharacterState, ECharacterState::R) ? BowParticleSystem->Activate() : BowParticleSystem->Deactivate();
	}
}

void ASparrowCharacter::OnRep_CrowdControlStateChanged()
{

}