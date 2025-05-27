// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/AuroraCharacter.h"
#include "Animations/PlayerAnimInstance.h"
#include "Components/StatComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/ActionStatComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Inputs/InputConfigData.h"
#include "Math/Vector.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Structs/CustomCombatData.h"
#include "Game/AOSGameInstance.h"
#include "Game/ArenaPlayerState.h"
#include "Props/SplineActor.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "Props/FreezeSegment.h"
#include "CrowdControls/StunEffect.h"
#include "CrowdControls/SlowEffect.h"
#include "Plugins/UniqueCodeGenerator.h"


AAuroraCharacter::AAuroraCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	StatComponent = CreateDefaultSubobject<UStatComponent>(TEXT("StatComponent"));
	ActionStatComponent = CreateDefaultSubobject<UActionStatComponent>(TEXT("ActionStatComponent"));

	if (HasAuthority())
	{
		StatComponent->SetIsReplicated(true);
		ActionStatComponent->SetIsReplicated(true);
	}

	SelectedCharacterIndex = 1;
	CharacterName = TEXT("Aurora");

	bSmoothMovement = false;
	bIsTumbling = false;
	bIsDashing = false;
	bIsFalling = false;

	ReplicatedTargetLocation = FVector(0);
	DashingDestination = FVector(0);
	TumblingDestination = FVector(0);
	FallingDestination = FVector(0);

	DashingDuration = 0.1f;
	TumblingDuration = 0.1f;
	FallingDuration = 0.1f;
	DashingElapsedTime = 0.0f;
	TumblingElapsedTime = 0.0f;
	FallingElapsedTime = 0.0f;

	TumblingHeightScale = 1000.f;
	TumbleHeightThreshold = 500.f;

	ComboCount = 1;
	MaxComboCount = 4;
}

void AAuroraCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 캐릭터가 텀블링 중일 때의 처리
	if (HasAuthority() && bIsTumbling)
	{
		HandleTumbling(DeltaSeconds);
	}

	// 캐릭터가 대쉬 중일 때의 처리
	if (HasAuthority() && bIsDashing)
	{
		HandleDashing(DeltaSeconds);
	}

	if (HasAuthority() && bIsFalling)
	{
		HandleFalling(DeltaSeconds);
	}

	// 클라이언트에서 부드러운 이동 처리를 위한 처리
	if (!HasAuthority() && bSmoothMovement)
	{
		SmoothMovement(DeltaSeconds);
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
					CharacterStateString += TEXT(", ");
				}
				CharacterStateString += EnumPtr->GetNameStringByIndex(i);
			}
		}
	}

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("CharacterState: %s"), *CharacterStateString), true, false, FLinearColor::Green, 2.0f, FName("1"));
}

void AAuroraCharacter::BeginPlay()
{
	Super::BeginPlay();

	PathMesh = GetOrLoadMesh("SplineMesh", TEXT(""));
	PathMaterial = GetOrLoadMaterial("SplineMaterial", TEXT(""));
}

void AAuroraCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [&]()
		{
			if (::IsValid(EnhancedInputComponent))
			{
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->MoveAction, ETriggerEvent::Triggered, this, &AAuroraCharacter::Move);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->LookAction, ETriggerEvent::Triggered, this, &AAuroraCharacter::Look);
				EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->JumpAction, ETriggerEvent::Started, this, &AAuroraCharacter::Jump);
			}
		}
	));
}

void AAuroraCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, bSmoothMovement, COND_None);
	DOREPLIFETIME_CONDITION(ThisClass, ReplicatedTargetLocation, COND_None);
}

void AAuroraCharacter::Move(const FInputActionValue& InValue)
{
	if (EnumHasAnyFlags(CharacterState, ECharacterState::Death))
	{
		return;
	}

	if (EnumHasAnyFlags(CharacterState, ECharacterState::Move) == false || EnumHasAnyFlags(CharacterState, ECharacterState::Rooted))
	{
		return;
	}

	if (EnumHasAnyFlags(CharacterState, ECharacterState::ActionActivated))
	{
		ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::ActionActivated);
		ServerStopAllMontages(0.5f, true);
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

void AAuroraCharacter::Look(const FInputActionValue& InValue)
{
	FVector2D LookVector = InValue.Get<FVector2D>();
	FRotator AimRotation = GetBaseAimRotation();

	AddControllerYawInput(LookVector.X);
	AddControllerPitchInput(LookVector.Y);

	CurrentAimYaw = AimRotation.Yaw;
	CurrentAimPitch = AimRotation.Pitch;

	UpdateAimValue_Server(CurrentAimPitch, CurrentAimYaw);
}



void AAuroraCharacter::Q_Executed()
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (EnumHasAnyFlags(CharacterState, ECharacterState::SwitchAction) == false)
	{
		ActionStatComponent->ClientNotifyAlertTextChanged("You cannot use this right now.");
		return;
	}

	UAnimMontage* Montage = GetOrLoadMontage("Q", TEXT("/Game/ParagonAurora/Characters/Heroes/Aurora/Animations/Ability_Q_Montage.Ability_Q_Montage"));
	if (!Montage)
	{
		return;
	}

	ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::Q);
	ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::Rooted);
	ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::ActionActivated);
	ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::SwitchAction);

	OnRootedStateEnded.AddDynamic(this, &ThisClass::RestoreRootedState);
	OnSwitchActionStateEnded.AddDynamic(this, &ThisClass::RestoreSwitchActionState);

	ServerPlayMontage(Montage, 1.0f, NAME_None, true);

	ActionStatComponent->HandleActionExecution(EActionSlot::Q, GetWorld()->GetTimeSeconds());
	ActionStatComponent->ActivateActionCooldown(EActionSlot::Q);
}


void AAuroraCharacter::Q_CheckHit()
{
	if (!HasAuthority()) return;

	SaveCharacterTransform();

	const FActionAttributes& ActionAttributes = ActionStatComponent->GetActionAttributes(EActionSlot::Q);
	const FActiveActionState& ActiveActionState = ActionStatComponent->GetActiveActionState(EActionSlot::Q);

	const float CharacterAD = StatComponent->GetAttackDamage();
	const float CharacterAP = StatComponent->GetAbilityPower();

	const float FinalDamage =
		(ActionAttributes.AttackDamage + CharacterAD * ActionAttributes.PhysicalScaling)
		+ (ActionAttributes.AbilityDamage + CharacterAP * ActionAttributes.MagicalScaling);

	FTransform Transform(UKismetMathLibrary::MakeRotFromX(LastForwardVector), LastCharacterLocation, FVector(1));

	UClass* FreezeSegmentClass = GetOrLoadClass(TEXT("FreezeSegment"), TEXT("/Game/FuryOfLegends/Characters/Aurora/Blueprints/BP_FreezeSegment.BP_FreezeSegment"));
	if (!FreezeSegmentClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[Server] Failed to load Freeze Segment Class. Path: /Game/FuryOfLegends/Characters/Aurora/Blueprints/BP_FreezeSegment"));
		return;
	}

	FDamageInformation DamageInformation;
	DamageInformation.SetActionSlot(EActionSlot::Q);
	DamageInformation.AddMagicDamage(FinalDamage);
	DamageInformation.AddTrigger(EAttackTrigger::AbilityEffects);
	DamageInformation.CrowdControls.Add(FCrowdControlInformation(ECrowdControl::Snare, 1.25f, 1.0f));

	AFreezeSegment* FreezeSegment = Cast<AFreezeSegment>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), FreezeSegmentClass, Transform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, this));
	if (FreezeSegment != nullptr)
	{
		FreezeSegment->Radius = ActionAttributes.Radius;
		FreezeSegment->Rate = GetUniqueAttribute(EActionSlot::Q, "Rate", 0.01f);
		FreezeSegment->NumParicles = GetUniqueAttribute(EActionSlot::Q, "NumParticles", 28);
		FreezeSegment->Lifetime = GetUniqueAttribute(EActionSlot::Q, "RingDuration", 2.f);
		FreezeSegment->Scale = GetUniqueAttribute(EActionSlot::Q, "ParticleScale", 1.f);
		FreezeSegment->Detection = ActiveActionState.CollisionDetection;
		FreezeSegment->DamageInformation = DamageInformation;

		UGameplayStatics::FinishSpawningActor(FreezeSegment, Transform);
	}

	ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::Q);
}

/*
 *	E 함수는 E 능력을 활성화합니다.
 *	능력 사용 가능 여부와 준비 상태를 확인한 후, 캐릭터를 목표 위치로 대쉬하고 보호막 메쉬를 생성합니다.
 *	고유 속성 값을 가져와서 능력의 범위와 지속 시간을 설정합니다.
 *	생성된 목표 위치는 서버로 전달되어 서버에서 이동이 처리됩니다.
 *
 *	1. UniqueValue[0]: Duration			얼음길 지속시간
 */

void AAuroraCharacter::E_Executed()
{
	if (HasAuthority() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: No Authority."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (EnumHasAnyFlags(CharacterState, ECharacterState::E))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Action E is already in use. Action cannot be performed again."), ANSI_TO_TCHAR(__FUNCTION__));
		ActionStatComponent->ClientNotifyAlertTextChanged("Action is already active.");
		return;
	}

	const FActiveActionState& ActiveAbilityState = ActionStatComponent->GetActiveActionState(EActionSlot::E);
	if (ActiveAbilityState.InstanceIndex == 1)
	{
		ExecutePrimary();
	}
	else
	{
		ExecuteSecondary();
	}
}

void AAuroraCharacter::ExecutePrimary()
{
	if (!ActionStatComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] ActionStatComponent is nullptr! Cannot execute primary action."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	UAnimMontage* Montage = GetOrLoadMontage("E", TEXT("/Game/ParagonAurora/Characters/Heroes/Aurora/Animations/Ability_E_Montage.Ability_E_Montage"));
	if (!Montage)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to load animation montage for action E."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (EnumHasAnyFlags(CharacterState, ECharacterState::SwitchAction) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Character cannot use this action now."), ANSI_TO_TCHAR(__FUNCTION__));
		ActionStatComponent->ClientNotifyAlertTextChanged("You cannot use this right now.");
		return;
	}

	UClass* SplineClass = GetOrLoadClass("Spline", TEXT("/Game/FuryOfLegends/Blueprints/Splines/BP_SplineActor.BP_SplineActor"));
	if (!SplineClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to load Spline class."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	SaveCharacterTransform();
	FTransform Transform(UKismetMathLibrary::MakeRotFromX(LastForwardVector), LastCharacterLocation, FVector(1));
	IcePath = GetWorld()->SpawnActor<ASplineActor>(SplineClass, Transform);
	if (::IsValid(IcePath) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to spawn SplineActor."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::SwitchAction);
	ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::ActionActivated);
	ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::Rooted);
	ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::E);

	OnRootedStateEnded.AddDynamic(this, &ThisClass::RestoreRootedState);
	OnSwitchActionStateEnded.AddDynamic(this, &ThisClass::RestoreSwitchActionState);

	const FActionAttributes& Stats	= ActionStatComponent->GetActionAttributes(EActionSlot::E);
	const float MaxHeightDifference = GetUniqueAttribute(EActionSlot::E, "MaxHeightDifference", 100.f);
	const float HeightThreshold		= GetUniqueAttribute(EActionSlot::E, "HeightThreshold", 600.f);
	const float SegmentLength		= GetUniqueAttribute(EActionSlot::E, "SegmentLength", 50.f);

	// 1. 지형 트레이스
	TArray<FVector> Path = TraceTerrainPath(Stats.Range, SegmentLength, HeightThreshold);

	// 2. 지형 분석
	TArray<FTerrainSegment> TerrainSegments = AnalyzeTerrainSegments(Path, MaxHeightDifference);

	// 3. 지형 분류
	ProcessTerrainSegments(TerrainSegments);

	// 4. 보정 처리
	SmoothTerrainSegments(Path, TerrainSegments);

	//VisualizeTerrainSegments(TerrainSegments, Path);

	DashingDuration = GetUniqueAttribute(EActionSlot::E, "CreationRate", 0.5);
	IcePath->InitializeSpline(Path, DashingDuration, SegmentLength);
	DashingDistance = IcePath->SplineComponent->GetSplineLength();
	DashingDestination = IcePath->SplineComponent->GetLocationAtDistanceAlongSpline(DashingDistance, ESplineCoordinateSpace::World) + FVector(0, 0, 95);
	
	if (DashingDistance <= 0.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid DashingDestination detected! LastCharacterLocation: %s, DashingDestination: %s"),
			ANSI_TO_TCHAR(__FUNCTION__), *LastCharacterLocation.ToString(), *DashingDestination.ToString());
		return;
	}

	if (Path.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] No valid path found for dashing."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	DistanceAlongSpline = 0.0f;
	DashingElapsedTime = 0.0f;

	ClientSetControllerRotationYaw(false);
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	ServerPlayMontage(Montage, 1.0f, NAME_None, true);
	ActionStatComponent->ActivateActionCooldown(EActionSlot::E);
}

void AAuroraCharacter::ExecuteSecondary()
{
	if (::IsValid(IcePath))
	{
		IcePath->SetLifeSpan(1.0f);
	}

	GetWorldTimerManager().ClearTimer(RecastTimer);
	ActionStatComponent->HandleActionExecution(EActionSlot::E, GetWorld()->GetTimeSeconds());
}

void AAuroraCharacter::StartDash()
{
	if (::IsValid(IcePath) == false)
	{
		return;
	}

	SpawnShieldMeshes();
	bIsDashing = true;
	IcePath->Activate();
}

void AAuroraCharacter::EndDash()
{
	float ReuseTime = 0.5f;
	if (::IsValid(ActionStatComponent))
	{
		ActionStatComponent->HandleActionExecution(EActionSlot::E, GetWorld()->GetTimeSeconds());
		ReuseTime = ActionStatComponent->ActiveActionState_E.ReuseDuration;
	}

	GetWorldTimerManager().SetTimer(RecastTimer, this, &ThisClass::ExecuteSecondary, ReuseTime, false, ReuseTime);
	ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::E);
}

void AAuroraCharacter::HandleDashing(float DeltaSeconds)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed: UWorld is null."), ANSI_TO_TCHAR(__FUNCTION__));

		bIsDashing = false;
		DashingElapsedTime = 0.0f;
		DistanceAlongSpline = 0;
		return;
	}

	if (::IsValid(IcePath) == false)
	{
		return;
	}

	// 시간 업데이트
	DashingElapsedTime += DeltaSeconds;
	const float Alpha = FMath::Clamp(DashingElapsedTime / DashingDuration, 0.0f, 1.0f);

	// `DistanceAlongSpline`이 자연스럽게 증가하도록 수정
	DistanceAlongSpline += (DashingDistance / DashingDuration) * DeltaSeconds;
	DistanceAlongSpline = FMath::Clamp(DistanceAlongSpline, 0.0f, DashingDistance);

	// 현재 위치 및 목표 위치 계산
	FVector CurrentLocation = GetActorLocation();
	FVector TargetLocation = IcePath->SplineComponent->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);

	// 오프셋을 자동 계산하여 자연스럽게 적용
	TargetLocation.Z += 95.f;

	// 속도 및 방향 계산
	FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
	float Speed = FVector::Dist(CurrentLocation, TargetLocation) / DeltaSeconds;
	GetCharacterMovement()->Velocity = Direction * Speed;

	// 스플라인 회전 적용 (보간 추가로 부드럽게 변경)
	FRotator CurrentRotation = GetActorRotation();
	FRotator TargetRotation = IcePath->SplineComponent->GetRotationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
	FRotator SmoothRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaSeconds, 5.0f);
	SetActorRotation(SmoothRotation);

	/*DashingElapsedTime += DeltaSeconds;

	const float Alpha = FMath::Clamp(DashingElapsedTime / DashingDuration, 0.0f, 1.0f);
	DistanceAlongSpline = FMath::Lerp(0.0f, DashingDistance, Alpha);

	// 현재 위치 및 목표 위치 계산
	FVector CurrentLocation = GetActorLocation();
	FVector TargetLocation = IcePath->SplineComponent->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World) + FVector(0, 0, 100);

	// 속도 계산 및 적용
	FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
	float Speed = FVector::Dist(CurrentLocation, TargetLocation) / DeltaSeconds;
	GetCharacterMovement()->Velocity = Direction * Speed;

	// 스플라인 방향을 계산하여 캐릭터 회전 적용
	FRotator TargetRotation = IcePath->SplineComponent->GetRotationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
	SetActorRotation(TargetRotation);*/

	// 대시 종료 조건
	if (DashingElapsedTime >= DashingDuration || FVector::Dist2D(DashingDestination, CurrentLocation) <= 50.f)
	{
		bIsDashing = false;
		DashingElapsedTime = 0.0f;
		DistanceAlongSpline = 0;

		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		GetCharacterMovement()->Velocity = FVector::ZeroVector;

		EndDash();
	}
}

TArray<FVector> AAuroraCharacter::TraceTerrainPath(const float TraceDistance, const float StepSize, const float HeightThreshold)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed: UWorld is null."), ANSI_TO_TCHAR(__FUNCTION__));
		return TArray<FVector>();
	}

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (TraceDistance <= 0.f || StepSize <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid TraceDistance or StepSize. TraceDistance: %f, StepSize: %f"), TraceDistance, StepSize);
		return TArray<FVector>();
	}

	TArray<FVector> OutPath;

	int32 Iterations = FMath::CeilToInt(TraceDistance / StepSize);
	FVector TraceStart = LastCharacterLocation;
	for (int32 i = 0; i < Iterations; i++)
	{
		TraceStart = LastCharacterLocation + LastForwardVector * StepSize * i;

		if (World->LineTraceSingleByChannel(HitResult, TraceStart + FVector(0, 0, 5000), TraceStart + FVector(0, 0, -5000), ECC_WorldStatic, QueryParams))
		{
			FVector HitLocation = HitResult.Location;

			// 높이 차 확인
			if (OutPath.Num() > 0 && FMath::Abs(HitLocation.Z - OutPath.Last().Z) > HeightThreshold)
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] Stopped: Height difference too large. Last Z: %f, Current Z: %f, Difference: %f"),
					ANSI_TO_TCHAR(__FUNCTION__),
					OutPath.Last().Z,
					HitLocation.Z,
					FMath::Abs(HitLocation.Z - OutPath.Last().Z));
				break; // 높이 차가 X 이상이면 Trace 중단
			}

			OutPath.Add(HitLocation);
		}
	}

	return MoveTemp(OutPath);
}

TArray<FTerrainSegment> AAuroraCharacter::AnalyzeTerrainSegments(const TArray<FVector>& PathPoints, const float& MaxHeightDifference)
{
	TArray<FTerrainSegment> OutTerrainSegments;

	int32 StartIndex = 0;
	FString CurrentType = "Flat";

	for (int32 i = 0; i < PathPoints.Num() - 1; i++)
	{
		float DeltaZ = PathPoints[i + 1].Z - PathPoints[i].Z;

		// 현재 타입 결정
		FString NewType = FMath::Abs(DeltaZ) <= MaxHeightDifference ? "Flat" : (DeltaZ > 0 ? "Uphill" : "Downhill");

		if (NewType != CurrentType)
		{
			if (NewType == "Downhill")
			{
				OutTerrainSegments.Add(FTerrainSegment(StartIndex, i, CurrentType));
				StartIndex = i;
			}
			else if (NewType == "Uphill")
			{
				OutTerrainSegments.Add(FTerrainSegment(StartIndex, i + 1, CurrentType));
				StartIndex = i;
			}
			else
			{
				OutTerrainSegments.Add(FTerrainSegment(StartIndex, i, CurrentType));
				StartIndex = i + 1;
			}

			CurrentType = NewType;
		}
	}

	// 마지막 구간 추가
	OutTerrainSegments.Add(FTerrainSegment(StartIndex, PathPoints.Num() - 1, CurrentType));
	return MoveTemp(OutTerrainSegments);
}

void AAuroraCharacter::ProcessTerrainSegments(TArray<FTerrainSegment>& TerrainSegments)
{
	// 1. Dip 처리: Downhill -> (Flat) -> Uphill
	for (int32 i = 0; i < TerrainSegments.Num() - 1; ++i)
	{
		if (TerrainSegments[i].Type == "Downhill")
		{
			int32 DipStart = i;
			int32 DipEnd = i;

			while (DipEnd + 1 < TerrainSegments.Num() && TerrainSegments[DipEnd + 1].Type == "Flat")
			{
				DipEnd++;
			}

			if (DipEnd + 1 < TerrainSegments.Num() && TerrainSegments[DipEnd + 1].Type == "Uphill")
			{
				TerrainSegments[DipStart].EndIndex = TerrainSegments[DipEnd + 1].EndIndex;
				TerrainSegments[DipStart].Type = "Dip";

				TerrainSegments.RemoveAt(DipStart + 1, DipEnd - DipStart + 1);
				--i;
				continue;
			}
		}
	}

	// 오르막 확장: 이전 Flat 포함
	for (int32 i = 0; i < TerrainSegments.Num(); ++i)
	{
		if (TerrainSegments[i].Type == "Uphill" && i > 0 && TerrainSegments[i - 1].Type == "Flat")
		{
			TerrainSegments[i].StartIndex = TerrainSegments[i - 1].StartIndex;
			TerrainSegments.RemoveAt(i - 1);
			--i;
		}
	}

	// 내리막 확장: 이후 Flat 포함
	for (int32 i = 0; i < TerrainSegments.Num() - 1; ++i)
	{
		if (TerrainSegments[i].Type == "Downhill" && TerrainSegments[i + 1].Type == "Flat")
		{
			TerrainSegments[i].EndIndex = TerrainSegments[i + 1].EndIndex;
			TerrainSegments.RemoveAt(i + 1);
			--i;
		}
	}
}

void AAuroraCharacter::SmoothTerrainSegments(TArray<FVector>& PathPoints, const TArray<FTerrainSegment>& TerrainSegments)
{
	for (const FTerrainSegment& Segment : TerrainSegments)
	{
		int32 StartIndex = Segment.StartIndex;
		int32 EndIndex = Segment.EndIndex;

		if (EndIndex <= StartIndex)
		{
			continue; // 구간이 유효하지 않은 경우 무시
		}

		FVector StartPoint = PathPoints[StartIndex];
		FVector EndPoint = PathPoints[EndIndex];

		// Leap 보간
		/*for (int32 i = StartIndex + 1; i < EndIndex; ++i)
		{
			float Alpha = static_cast<float>(i - StartIndex) / static_cast<float>(EndIndex - StartIndex);
			PathPoints[i].Z = FMath::Lerp(StartPoint.Z, EndPoint.Z, Alpha);
		}*/

		// Ease-In/Ease-Out 보간
		for (int32 i = StartIndex + 1; i < EndIndex; ++i)
		{
			float Alpha = static_cast<float>(i - StartIndex) / static_cast<float>(EndIndex - StartIndex);

			float EaseAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 1.5f);  //  Ease-In/Ease-Out
			// float EaseAlpha = FMath::InterpCircularInOut(0.0f, 1.0f, Alpha); //  CircularInOut
			// float EaseAlpha = FMath::InterpCubicInOut(0.0f, 1.0f, Alpha);    //  Cubic 보간
			// float EaseAlpha = FMath::InterpSinInOut(0.0f, 1.0f, Alpha);      //  Sine 곡선 보간
			// float EaseAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);          //  SmoothStep 보간
			PathPoints[i].Z = FMath::Lerp(StartPoint.Z, EndPoint.Z, EaseAlpha);
		}
	}
}

void AAuroraCharacter::VisualizeTerrainSegments(const TArray<FTerrainSegment>& TerrainSegments, const TArray<FVector>& PathPoints)
{
	UWorld* World = GetWorld();
	if (!World) return;

	// 지형 타입별 색상 매핑
	TMap<FString, FColor> TerrainColors = {
		{"Flat", FColor::Green},
		{"Uphill", FColor::Blue},
		{"Downhill", FColor::Red},
		{"Dip", FColor::Yellow}
	};

	// 모든 구간 시각화
	for (const auto& Segment : TerrainSegments)
	{
		FColor SegmentColor = TerrainColors.Contains(Segment.Type) ? TerrainColors[Segment.Type] : FColor::White;

		for (int32 i = Segment.StartIndex; i <= Segment.EndIndex; ++i)
		{
			DrawDebugSphere(World, PathPoints[i], 10.f, 12, SegmentColor, true, -1.0f, 0, 2.0f);
		}
	}
}

void AAuroraCharacter::SpawnShieldMeshes()
{
	// 방패 메쉬 로드 및 유효성 검사
	UStaticMesh* ShieldTop = GetOrLoadMesh(TEXT("ShieldTop"), TEXT("/Game/ParagonAurora/FX/Meshes/Aurora/SM_FrostShield_Spikey_Top.SM_FrostShield_Spikey_Top"));
	UStaticMesh* ShieldMiddle = GetOrLoadMesh(TEXT("ShieldMiddle"), TEXT("/Game/ParagonAurora/FX/Meshes/Aurora/SM_FrostShield_Spikey_Middle.SM_FrostShield_Spikey_Middle"));
	UStaticMesh* ShieldBottom = GetOrLoadMesh(TEXT("ShieldBottom"), TEXT("/Game/ParagonAurora/FX/Meshes/Aurora/SM_FrostShield_Spikey_Bottom.SM_FrostShield_Spikey_Bottom"));

	if (::IsValid(ShieldTop) && ::IsValid(ShieldMiddle) && ::IsValid(ShieldBottom))
	{
		USceneComponent* SceneComponent = GetRootComponent();
		ServerSpawnMeshAttached(ShieldBottom, SceneComponent, 1.25f);
		ServerSpawnMeshAttached(ShieldMiddle, SceneComponent, 1.25f);
		ServerSpawnMeshAttached(ShieldTop, SceneComponent, 1.25f);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn shield meshes: one or more meshes are invalid."));
	}
}


void AAuroraCharacter::R_Started()
{

}

void AAuroraCharacter::R_Executed()
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (EnumHasAnyFlags(CharacterState, ECharacterState::SwitchAction) == false)
	{
		ActionStatComponent->ClientNotifyAlertTextChanged("You cannot use this right now.");
		return;
	}

	UAnimMontage* Montage = GetOrLoadMontage("R", TEXT("/Game/ParagonAurora/Characters/Heroes/Aurora/Animations/Ability_R_Montage.Ability_R_Montage"));
	if (!Montage)
	{
		return;
	}

	SaveCharacterTransform();
	ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::SwitchAction);
	ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::R);
	ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::Rooted);
	ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::ActionActivated);

	OnRootedStateEnded.AddDynamic(this, &ThisClass::RestoreRootedState);
	OnSwitchActionStateEnded.AddDynamic(this, &ThisClass::RestoreSwitchActionState);


	ServerPlayMontage(Montage, 1.0f, NAME_None, true);

	const float BoostStrength = GetUniqueAttribute(EActionSlot::R, TEXT("BoostStrength"), 600.f);
	if (!GetCharacterMovement()->IsFalling())
	{
		LaunchCharacter(FVector(0, 0, BoostStrength), false, true);
	}

	ActionStatComponent->HandleActionExecution(EActionSlot::R, GetWorld()->GetTimeSeconds());
	ActionStatComponent->ActivateActionCooldown(EActionSlot::R);
}


void AAuroraCharacter::FindExplosionTargets(const FVector& Pos, TSet<TWeakObjectPtr<ACharacterBase>>& OutTargets, TEnumAsByte<ECollisionChannel> CollisionChannel, const float InRadius)
{
	TArray<FOverlapResult> OverlapResults;
	if (GetWorld()->OverlapMultiByChannel(OverlapResults, Pos, FQuat::Identity, CollisionChannel, FCollisionShape::MakeSphere(InRadius)))
	{
		for (const auto& Result : OverlapResults)
		{
			ACharacterBase* OverlappedCharacter = Cast<ACharacterBase>(Result.GetActor());
			if (IsValid(OverlappedCharacter) == false)
			{
				continue;
			}

			if (OverlappedCharacter->TeamSide == this->TeamSide)
			{
				continue; // 적 팀이 아닌 경우 다음으로
			}

			OutTargets.Add(OverlappedCharacter);
		}
	}
}

void AAuroraCharacter::R_CheckHit()
{
	if (!HasAuthority()) return;

	const FActionAttributes& StatTable = ActionStatComponent->GetActionAttributes(EActionSlot::R);
	const FActiveActionState& ActiveAbilityState = ActionStatComponent->GetActiveActionState(EActionSlot::R);
	const float FirstDelay = GetUniqueAttribute(EActionSlot::R, "FirstDelay", 1.5f);

	ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::R);

	TSet<TWeakObjectPtr<ACharacterBase>> InitialTargets;
	TSharedPtr<TSet<TWeakObjectPtr<ACharacterBase>>> AffectedCharacters = MakeShared<TSet<TWeakObjectPtr<ACharacterBase>>>();	

	FindExplosionTargets(GetActorLocation(), InitialTargets, ActiveAbilityState.CollisionDetection, StatTable.Radius);

	if (InitialTargets.Num() == 0)
	{
		return;
	}

	//ScheduleExplosion(InitialTargets, AffectedCharacters);

	FTimerHandle SlowTimer;
	TSharedPtr<TSet<TWeakObjectPtr<ACharacterBase>>> TargetsCopy = MakeShared<TSet<TWeakObjectPtr<ACharacterBase>>>(InitialTargets);
	GetWorldTimerManager().SetTimer(SlowTimer, [this, TargetsCopy, AffectedCharacters]()
		{
			ScheduleExplosion(*TargetsCopy, AffectedCharacters);
		}, FirstDelay, false, FirstDelay);
}

// 초기 슬로우
void AAuroraCharacter::ScheduleExplosion(TSet<TWeakObjectPtr<ACharacterBase>>& Targets, TSharedPtr<TSet<TWeakObjectPtr<ACharacterBase>>> AffectedCharacters)
{
	UParticleSystem* UltimateSlowed = GetOrLoadParticle(TEXT("UltimateSlowed"), TEXT("/Game/ParagonAurora/FX/Particles/Abilities/Ultimate/FX/P_Aurora_Ultimate_Slowed.P_Aurora_Ultimate_Slowed"));
	if (!UltimateSlowed) return;

	const FActionAttributes& StatTable = ActionStatComponent->GetActionAttributes(EActionSlot::R);
	const FActiveActionState& ActiveAbilityState = ActionStatComponent->GetActiveActionState(EActionSlot::R);

	const float SlowDuration = GetUniqueAttribute(EActionSlot::R, "SlowDuration", 1.5f);
	const float MovementSpeedSlow = GetUniqueAttribute(EActionSlot::R, "MovementSpeedSlow", 20.f);

	FCrowdControlInformation SlowEffect;
	SlowEffect.Type = ECrowdControl::Slow;
	SlowEffect.Duration = SlowDuration;
	SlowEffect.Percent = MovementSpeedSlow;

	for (auto It = Targets.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			continue;
		}

		ServerApplyCrowdControl((*It).Get(), this, Controller, SlowEffect);
		if ((*It)->GetMesh() && (*It)->IsValidLowLevelFast())
		{
			ServerSpawnEmitterAttached(UltimateSlowed, (*It)->GetMesh(), (*It)->GetMesh()->GetSocketTransform(FName("Root")), EAttachLocation::KeepWorldPosition);
		}
		AffectedCharacters->Add(*It);
	}

	FTimerHandle ExplosionTimer;
	TSharedPtr<TSet<TWeakObjectPtr<ACharacterBase>>> TargetsCopy = MakeShared<TSet<TWeakObjectPtr<ACharacterBase>>>(Targets);
	GetWorldTimerManager().SetTimer(ExplosionTimer, [this, TargetsCopy, AffectedCharacters]()
		{
			HandleExplosion(*TargetsCopy, AffectedCharacters);
		}, SlowDuration, false);
}

// 초기 슬로우 후 폭팔 및 기절 적용
void AAuroraCharacter::HandleExplosion(TSet<TWeakObjectPtr<ACharacterBase>>& FrozenEnemy, TSharedPtr<TSet<TWeakObjectPtr<ACharacterBase>>> AffectedCharacters)
{
	UParticleSystem* UltimateExplode = GetOrLoadParticle(TEXT("UltimateExplode"), TEXT("/Game/ParagonAurora/FX/Particles/Abilities/Ultimate/FX_P_Aurora_Ultimate_Explode"));
	if (!UltimateExplode) return;

	const FActiveActionState& ActiveAbilityState = ActionStatComponent->GetActiveActionState(EActionSlot::R);
	const FActionAttributes& ActionAttributes = ActionStatComponent->GetActionAttributes(EActionSlot::R);

	const float StunDuration = GetUniqueAttribute(EActionSlot::R, "StunDuration", 1.0f);
	const float ChainRadius = GetUniqueAttribute(EActionSlot::R, "ChainRadius", 500.0f);
	const float HeroShatterAbilityDamage = GetUniqueAttribute(EActionSlot::R, "HeroShatterAbilityDamage", 0.f);
	const float NonHeroShatterAbilityDamage = GetUniqueAttribute(EActionSlot::R, "NonHeroShatterAbilityDamage", 0.f);
	const float InitialPowerScaling = GetUniqueAttribute(EActionSlot::R, "InitialPowerScaling", 0.f);
	const float PowerScalingOnHero = GetUniqueAttribute(EActionSlot::R, "PowerScalingOnHero", 0.f);
	const float PowerScalingOnNonHero = GetUniqueAttribute(EActionSlot::R, "PowerScalingOnNonHero", 0.f);

	const float CharacterAP = StatComponent->GetAbilityPower();

	FDamageInformation PlayerDamageInfo;
	FDamageInformation NonHeroDamageInfo;

	// 공통 설정
	PlayerDamageInfo.SetActionSlot(EActionSlot::R);
	NonHeroDamageInfo.SetActionSlot(EActionSlot::R);

	PlayerDamageInfo.AddMagicDamage((ActionAttributes.AbilityDamage + CharacterAP * PowerScalingOnHero) + HeroShatterAbilityDamage);
	NonHeroDamageInfo.AddMagicDamage((ActionAttributes.AbilityDamage + CharacterAP * PowerScalingOnNonHero) + NonHeroShatterAbilityDamage);

	PlayerDamageInfo.AddCrowdControl(FCrowdControlInformation(ECrowdControl::Stun, StunDuration));
	NonHeroDamageInfo.AddCrowdControl(FCrowdControlInformation(ECrowdControl::Stun, StunDuration));

	TSet<TWeakObjectPtr<ACharacterBase>> ChainTargets;
	for (auto It = FrozenEnemy.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			continue;
		}

		// 플레이어 여부 확인
		if (EnumHasAnyFlags((*It)->ObjectType, EObjectType::Player))
		{
			ServerApplyDamage((*It).Get(), this, GetController(), PlayerDamageInfo);
		}
		else
		{
			ServerApplyDamage((*It).Get(), this, GetController(), NonHeroDamageInfo);
		}

		if ((*It)->GetMesh() && (*It)->IsValidLowLevelFast())
		{
			ServerSpawnEmitterAttached(UltimateExplode, (*It)->GetMesh(), (*It)->GetMesh()->GetSocketTransform(FName("Root")), EAttachLocation::KeepWorldPosition);
		}

		FindExplosionTargets((*It)->GetActorLocation(), ChainTargets, ActiveAbilityState.CollisionDetection, ChainRadius);
		AffectedCharacters->Add((*It));
	}


	TSet<TWeakObjectPtr<ACharacterBase>> UniqueChainTargets;
	for (auto It = ChainTargets.CreateIterator(); It; ++It)
	{
		if (It->IsValid() && !AffectedCharacters->Contains(*It))
		{
			UniqueChainTargets.Add(*It);
		}
	}
	ChainTargets = MoveTemp(UniqueChainTargets);

	if (ChainTargets.Num() == 0) return;
	ScheduleExplosion(ChainTargets, AffectedCharacters);
}


void AAuroraCharacter::LandWithinDuration()
{
	if (!HasAuthority()) return;

	FVector StartLocation = GetActorLocation();
	FVector EndLocation = StartLocation - FVector(0, 0, 10000.0f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams);

	SaveCharacterTransform();

	FallingDestination = HitResult.Location + FVector(0, 0, 95.f);
	FallingDuration = 0.13f;
	FallingElapsedTime = 0.0f;
	bIsFalling = true;
}


void AAuroraCharacter::HandleFalling(float DeltaSeconds)
{
	FallingElapsedTime += DeltaSeconds;

	float InterpolationAlpha = FMath::Clamp(FallingElapsedTime / FallingDuration, 0.0f, 1.0f);

	if (InterpolationAlpha >= 1.0f)
	{
		bIsFalling = false;
		bSmoothMovement = false;

		GetCharacterMovement()->Velocity.Z = 0;
		SetActorLocation(FallingDestination, false, nullptr, ETeleportType::None);
	}
	else
	{
		FVector CurrentLocation = GetActorLocation();
		FVector Direction = (FallingDestination - LastCharacterLocation).GetSafeNormal();
		float Speed = (FallingDestination - LastCharacterLocation).Size() / FallingDuration;

		FVector NewVelocity = Direction * Speed;
		GetCharacterMovement()->Velocity = FVector(NewVelocity.X, NewVelocity.Y, NewVelocity.Z);
	}
}


/**
 * @brief LMB 능력을 활성화합니다. Ctrl 키가 눌리거나 ActionStatComponent가 없는 경우 능력을 사용할 수 없습니다.
 *        능력이 준비된 경우, 능력을 사용하고 쿨다운을 시작합니다. 첫 번째 공격과 후속 공격을 처리합니다.
 */
void AAuroraCharacter::LMB_Started()
{

}


void AAuroraCharacter::LMB_Executed()
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (EnumHasAnyFlags(CharacterState, ECharacterState::SwitchAction) == false)
	{
		ActionStatComponent->ClientNotifyAlertTextChanged("You cannot use this right now.");
		return;
	}

	UAnimMontage* Montage = GetOrLoadMontage("LMB", TEXT("/Game/ParagonAurora/Characters/Heroes/Aurora/Animations/Ability_LMB_Montage.Ability_LMB_Montage"));
	if (!Montage)
	{
		return;
	}

	switch (ComboCount)
	{
	case 1:
	case 2:
	case 3:
		BasicAttackAnimLength = 1.0f;
		break;
	case 4:
		BasicAttackAnimLength = 0.5f;
		break;
	default:
		BasicAttackAnimLength = 1.0f;
		break;
	}

	BasicAttackAnimPlayRate = AdjustAnimPlayRate(BasicAttackAnimLength);
	FName NextSectionName = GetAttackMontageSection(ComboCount);

	ServerPlayMontage(Montage, BasicAttackAnimPlayRate, NextSectionName, true);
	ComboCount = FMath::Clamp<int32>((ComboCount % 4) + 1, 1, MaxComboCount);

	ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::SwitchAction);
	ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::LMB);
	OnSwitchActionStateEnded.AddDynamic(this, &ThisClass::RestoreSwitchActionState);

	ActionStatComponent->HandleActionExecution(EActionSlot::LMB, GetWorld()->GetTimeSeconds());
	ActionStatComponent->ActivateActionCooldown(EActionSlot::LMB);
}

void AAuroraCharacter::LMB_CheckHit()
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (::IsValid(this) == false)
	{
		return;
	}

	if (!ActionStatComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: ActionStatComponent is null."), ANSI_TO_TCHAR(__FUNCTION__));
		ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::LMB);
		return;
	}

	const FActionAttributes& ActionAttributes = ActionStatComponent->GetActionAttributes(EActionSlot::LMB);
	if (ActionAttributes.Name.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: ActionAttributes for LMB not found."), ANSI_TO_TCHAR(__FUNCTION__));
		ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::LMB);
		return;
	}

	FHitResult ImpactResult = SweepTraceFromAimAngles(ActionAttributes.Range);
	AActor* HitActor = ImpactResult.GetActor();
	if (::IsValid(HitActor) == false)
	{
		ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::LMB);
		return;
	}

	ACharacterBase* TargetCharacter = Cast<ACharacterBase>(HitActor);
	if (!::IsValid(TargetCharacter))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: TargetCharacter is invalid."), ANSI_TO_TCHAR(__FUNCTION__));
		ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::LMB);
		return;
	}

	ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::LMB);

	// 적팀일 경우에만 데미지 적용.
	if (TargetCharacter->TeamSide != this->TeamSide)
	{
		const FActionAttributes& AbilityStatTable = ActionStatComponent->GetActionAttributes(EActionSlot::LMB);

		const float Character_AttackDamage = StatComponent->GetAttackDamage();
		const float Character_AbilityPower = StatComponent->GetAbilityPower();

		const float BaseAttackDamage = AbilityStatTable.AttackDamage;
		const float BaseAbilityPower = AbilityStatTable.AbilityDamage;
		const float PhysicalScaling = AbilityStatTable.PhysicalScaling;
		const float MagicalScaling = AbilityStatTable.MagicalScaling;

		const float FinalDamage = (BaseAttackDamage + Character_AttackDamage * PhysicalScaling) + (BaseAbilityPower + Character_AbilityPower * MagicalScaling);

		FDamageInformation DamageInformation;
		DamageInformation.ActionSlot = EActionSlot::LMB;
		DamageInformation.AddDamage(EDamageType::Physical, FinalDamage);
		DamageInformation.AddTrigger(EAttackTrigger::OnHit);
		DamageInformation.AddTrigger(EAttackTrigger::OnAttack);

		ServerApplyDamage(TargetCharacter, this, GetController(), DamageInformation);

		/* Spawn HitSuccessImpact */
		UParticleSystem* MeleeSuccessImpact = GetOrLoadParticle("MeleeSuccessImpact", TEXT("/Game/ParagonAurora/FX/Particles/Abilities/Primary/FX/P_Aurora_Melee_SucessfulImpact.P_Aurora_Melee_SucessfulImpact"));
		if (MeleeSuccessImpact)
		{
			FTransform transform(FRotator(0), TargetCharacter->GetMesh()->GetSocketLocation("Impact"), FVector(1));
			SpawnEmitterAtLocation(MeleeSuccessImpact, transform);
		}
	}
}



/**
 * RMB 능력을 활성화합니다. Ctrl 키가 눌렸는지 여부를 확인하고, 능력이 준비되었는지 확인합니다.
 * 능력이 준비되었으면 이동 및 행동 상태를 업데이트하고, 해당 섹션의 애니메이션을 재생한 후 서버에서 능력을 실행합니다.
 *
 * 캐릭터의 현재 위치와 이동 방향을 기반으로 목표 위치를 설정합니다.
 * 목표 위치로의 이동 중 장애물 충돌 여부를 확인하고, 충돌한 경우 적절히 처리합니다.
 *
 *	- UniqueValue[0]: Range			사거리
 */
void AAuroraCharacter::RMB_Started()
{

}

void AAuroraCharacter::RMB_Executed()
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (EnumHasAnyFlags(CharacterState, ECharacterState::SwitchAction) == false)
	{
		ActionStatComponent->ClientNotifyAlertTextChanged("You cannot use this right now.");
		return;
	}

	ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::SwitchAction);

	UAnimMontage* Montage = GetOrLoadMontage("RMB", TEXT("/Game/ParagonAurora/Characters/Heroes/Aurora/Animations/Ability_RMB_Montage.Ability_RMB_Montage"));
	if (!Montage)
	{
		ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::SwitchAction);
		return;
	}

	const FRotator ControlRotationYaw(0.f, CurrentAimYaw, 0.f);
	const FVector ForwardDirection = FRotationMatrix(ControlRotationYaw).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(ControlRotationYaw).GetUnitAxis(EAxis::Y);

	FVector MoveDirection = (ForwardDirection * ForwardInputValue) + (RightDirection * RightInputValue);
	MoveDirection = MoveDirection.GetSafeNormal();

	const FActionAttributes& ActionAttributes = ActionStatComponent->GetActionAttributes(EActionSlot::RMB);

	if (ActionAttributes.Range <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid ability range: %f"), ANSI_TO_TCHAR(__FUNCTION__), ActionAttributes.Range);
		ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::SwitchAction);
		return;
	}

	if (MoveDirection.IsNearlyZero())
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] MoveDirection is nearly zero."), ANSI_TO_TCHAR(__FUNCTION__));
		ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::SwitchAction);
		return;
	}

	SaveCharacterTransform();
	ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::RMB);
	ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::Rooted);
	ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::ActionActivated);

	OnRootedStateEnded.AddDynamic(this, &ThisClass::RestoreRootedState);
	OnSwitchActionStateEnded.AddDynamic(this, &ThisClass::RestoreSwitchActionState);

	FName MontageSectionName = FName(*FString::Printf(TEXT("RMB%d"), CalculateDirectionIndex()));
	TumblingHeightScale = GetUniqueAttribute(EActionSlot::RMB, TEXT("HeightScale"), 1000.f);
	TumbleHeightThreshold = GetUniqueAttribute(EActionSlot::RMB, TEXT("HeightThreshold"), 500.f);
	TumblingDestination = CalculateTumblingDestination(MoveDirection, ActionAttributes.Range) + FVector(0, 0, 95.f);

	// 능력을 사용하고 쿨다운 시작
	if (!TumblingDestination.ContainsNaN() && !TumblingDestination.IsNearlyZero(0.1))
	{
		ActionStatComponent->HandleActionExecution(EActionSlot::RMB, GetWorld()->GetTimeSeconds());
		ActionStatComponent->ActivateActionCooldown(EActionSlot::RMB);

		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);

		TumblingSpeed = GetUniqueAttribute(EActionSlot::RMB, TEXT("TumblingSpeed"), 200.f);
		TumblingDirection = (TumblingDestination - LastCharacterLocation).GetSafeNormal();
		TumblingDistance = FVector::Dist(LastCharacterLocation, TumblingDestination);
		TumblingDuration = 0.35f;
		TumblingElapsedTime = 0.0f;

		ServerPlayMontage(Montage, 1.0f, MontageSectionName, true);
		bIsTumbling = true;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Calculated target location is invalid: %s"), ANSI_TO_TCHAR(__FUNCTION__), *TumblingDestination.ToString());
		RMB_Canceled();
	}
}


FVector AAuroraCharacter::CalculateTumblingDestination(const FVector& MoveDirection, const float Range)
{
	FVector CurrentLocation = GetActorLocation();
	FVector TargetLocation = CurrentLocation + (MoveDirection * Range);

	FCollisionQueryParams CollisionParams(NAME_None, false, this);
	FHitResult FirstHitResult;
	bool bHitOccurred = GetWorld()->LineTraceSingleByChannel(
		FirstHitResult,
		CurrentLocation,
		TargetLocation,
		ECC_WorldStatic,
		CollisionParams
	);

	if (bHitOccurred)
	{
		FVector AdjustedLocation = FirstHitResult.Location + MoveDirection * 10;

		FHitResult SecondHitResult;
		bool bSecondHitOccurred = GetWorld()->LineTraceSingleByChannel(
			SecondHitResult,
			AdjustedLocation + FVector(0, 0, 10000.f),
			AdjustedLocation + FVector(0, 0, -10000.f),
			ECC_WorldStatic,
			CollisionParams
		);

		if (FMath::Abs(FirstHitResult.Location.Z - SecondHitResult.Location.Z) >= TumbleHeightThreshold)
		{
			TargetLocation = (AdjustedLocation - MoveDirection * 10.f);
		}
		else
		{
			TargetLocation = SecondHitResult.Location + MoveDirection * 50;
		}
	}
	else
	{
		bool bSecondHitOccurred = GetWorld()->LineTraceSingleByChannel(
			FirstHitResult,
			TargetLocation + FVector(0, 0, 10000.f),
			TargetLocation + FVector(0, 0, -10000.f),
			ECC_WorldStatic,
			CollisionParams
		);

		TargetLocation = FirstHitResult.Location;
	}

	return TargetLocation;
}



/**
 * 텀블링 중인 캐릭터를 처리합니다. 일정 시간 동안 텀블링 상태를 유지하고, 텀블링이 끝나면 상태를 업데이트합니다.
 * @param DeltaSeconds 프레임 간의 시간 간격
 */
void AAuroraCharacter::HandleTumbling(float DeltaSeconds)
{
	TumblingElapsedTime += DeltaSeconds;
	float Alpha = FMath::Clamp(TumblingElapsedTime / TumblingDuration, 0.0f, 1.0f);

	if (Alpha >= 1.0f)
	{
		ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::RMB);

		bIsTumbling = false;
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		GetCharacterMovement()->Velocity = FVector(0);
		SetActorLocation(TumblingDestination);
		return;
	}

	// 수평 속도 계산
	FVector HorizontalVelocity = TumblingDirection * (TumblingDistance / TumblingDuration);

	// 포물선 높이 계산
	float ParabolicHeight = FMath::Sin(Alpha * PI) * TumblingHeightScale;

	// 최종 속도 계산 (수평 + 수직)
	FVector NewVelocity = HorizontalVelocity;
	NewVelocity.Z = (TumblingDestination.Z - LastCharacterLocation.Z) / TumblingDuration + ParabolicHeight / DeltaSeconds;

	GetCharacterMovement()->Velocity = NewVelocity;
}


/**
 * 캐릭터를 목표 위치로 부드럽게 이동시킵니다.
 * 현재 위치와 목표 위치 사이를 보간하여 캐릭터를 이동시킵니다.
 */
void AAuroraCharacter::SmoothMovement(float DeltaSeconds)
{
	FVector CurrentLocation = GetActorLocation();
	FVector NewLocation = FMath::VInterpTo(CurrentLocation, ReplicatedTargetLocation, DeltaSeconds, 20.f);
	SetActorLocation(NewLocation);
}


/**
 * 이동 방향을 기반으로 방향 인덱스를 계산합니다.
 * 이동 벡터와 캐릭터의 기준 조준 방향을 사용하여 각도를 계산하고, 이를 8방향(각 방향은 45도 간격) 중 하나로 매핑합니다.
 * @param MoveDirection 이동 방향 벡터
 * @return 1에서 8까지의 방향 인덱스
 */
int32 AAuroraCharacter::CalculateDirectionIndex()
{
	float Yaw = UKismetMathLibrary::NormalizedDeltaRotator(
		UKismetMathLibrary::MakeRotFromX(GetCharacterMovement()->Velocity),
		GetBaseAimRotation()
	).Yaw;

	float NormalizedYaw = FMath::Fmod(Yaw + 360.0f, 360.0f);
	int32 DirectionIndex = FMath::RoundToInt(NormalizedYaw / 45.0f) % 8;

	return DirectionIndex + 1;
}




void AAuroraCharacter::CancelAction()
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

void AAuroraCharacter::Q_Canceled()
{
	if (::IsValid(AnimInstance) == false)
	{
		return;
	}

	ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::Q);
}

void AAuroraCharacter::E_Canceled()
{
	if (::IsValid(AnimInstance) == false)
	{
		return;
	}

	bIsDashing = false;
	DashingElapsedTime = 0.0f;
	DistanceAlongSpline = 0;

	ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::E);
}


void AAuroraCharacter::LMB_Canceled()
{
	if (::IsValid(AnimInstance) == false)
	{
		return;
	}

	ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::LMB);
}

void AAuroraCharacter::RMB_Canceled()
{
	if (::IsValid(AnimInstance) == false)
	{
		return;
	}

	TumblingSpeed = 0.f;
	TumblingDirection = FVector::ZeroVector;
	TumblingDistance = 0.f;
	TumblingDuration = 0.35f;
	TumblingElapsedTime = 0.0f;

	ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::RMB);
}


void AAuroraCharacter::HandleActionNotifyEvent(EActionSlot ActionSlot, int32 EventID)
{
	if (EnumHasAnyFlags(ActionSlot, EActionSlot::LMB) && EventID == 1)
	{
		ComboCount = 1;

		if (::IsValid(AnimInstance) == false)
		{
			return;
		}

		AnimInstance->StopAllMontages(0.5f);
		ServerStopAllMontages(0.5, false);
		return;
	}

	if (EnumHasAnyFlags(ActionSlot, EActionSlot::E) && EventID == 1)
	{
		if (HasAuthority())
		{
			StartDash();
		}

		return;
	}

	if (EnumHasAnyFlags(ActionSlot, EActionSlot::R) && EventID == 1)
	{
		if (HasAuthority())
		{
			LandWithinDuration();
		}
		return;
	}
}



void AAuroraCharacter::OnPreDamageReceived(float FinalDamage)
{
	UE_LOG(LogTemp, Warning, TEXT("AAuroraCharacter::OnPreDamageReceived() FinalDamage: %f"), FinalDamage);
}


void AAuroraCharacter::ExecuteSomethingSpecial()
{
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("[AAuroraCharacter::ExecuteSomethingSpecial] World is null."));
		return;
	}

	UKismetSystemLibrary::PrintString(GetWorld(), TEXT("[AAuroraCharacter::ExecuteSomethingSpecial] ExecuteSomethingSpecial function called."), true, true, FLinearColor::Red, 2.0f, NAME_None);

	FCrowdControlInformation CrowdControlInformation;
	CrowdControlInformation.Type = ECrowdControl::Snare;
	CrowdControlInformation.Percent = 90;
	CrowdControlInformation.Duration = 2.0f;

	ServerApplyCrowdControl(this, this, GetController(), CrowdControlInformation);
}


bool AAuroraCharacter::ValidateAbilityUsage()
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

	if (EnumHasAnyFlags(CharacterState, ECharacterState::SwitchAction))
	{
		return true;
	}

	return false;
}

void AAuroraCharacter::MontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	Super::MontageEnded(Montage, bInterrupted);

	if (Montage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("MontageEnded called with null Montage"));
		return;
	}

	if (HasAuthority())
	{
		if (Montage->GetName().Equals(TEXT("Ability_E_Montage")))
		{
			ClientSetControllerRotationYaw(true);
		}
	}
}

void AAuroraCharacter::OnRep_CharacterStateChanged()
{
	Super::OnRep_CharacterStateChanged();

}

void AAuroraCharacter::OnRep_CrowdControlStateChanged()
{

}


void AAuroraCharacter::MulticastSetTargetLocation_Implementation(FVector InTargetLocation)
{
	if (HasAuthority())
	{
		return;
	}

	if (InTargetLocation.ContainsNaN() || InTargetLocation.IsNearlyZero(0.01))
	{
		return;
	}

	ReplicatedTargetLocation = InTargetLocation;
}
