// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/MinionBase.h"
#include "Controllers/MinionAIController.h"
#include "Components/MinionStatComponent.h"
#include "Components/ActionStatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CharacterWidgetComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Game/ArenaGameMode.h"
#include "Game/AOSGameInstance.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Animations/MinionAnimInstance.h"
#include "UI/UserWidgetBarBase.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "NavigationSystem.h"
#include "NavModifierComponent.h"
#include "NavAreas/NavArea_Null.h"
#include "NavAreas/NavArea_Obstacle.h"
#include "NavAreas/NavArea_Default.h"
#include "Components/SplineComponent.h"
#include "Structs/CharacterResources.h"

AMinionBase::AMinionBase()
{
	AutoPossessAI = EAutoPossessAI::Disabled;  // AI Controller를 자동으로 생성하지 않도록 설정
	AIControllerClass = nullptr;  // AIController를 자동으로 생성하지 않음

	StatComponent = CreateDefaultSubobject<UMinionStatComponent>(TEXT("MinionStatComponent"));
	ActionStatComponent = CreateDefaultSubobject<UActionStatComponent>(TEXT("MinionActionStatComponent"));

	WidgetComponent = CreateDefaultSubobject<UCharacterWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(GetRootComponent());
	WidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 130.f));

	static ConstructorHelpers::FClassFinder<UUserWidgetBase> StateBarWidgetRef(TEXT("/Game/FuryOfLegends/UI/Blueprints/HUD/WBP_HPBar.WBP_HPBar_C"));
	if (StateBarWidgetRef.Succeeded())
	{
		WidgetComponent->SetWidgetClass(StateBarWidgetRef.Class);
		WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		WidgetComponent->SetDrawSize(FVector2D(150.f, 10.0f));
		WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (CapsuleComp)
	{
		float CharacterHalfHeight = 65.f;
		float CharacterRadius = 60.f;

		CapsuleComp->InitCapsuleSize(CharacterRadius, CharacterHalfHeight);
		CapsuleComp->SetCollisionProfileName(TEXT("AICharacter"));
		CapsuleComp->SetCanEverAffectNavigation(false);

		HomingTargetSceneComponent->SetRelativeLocation(FVector(0, 0, CharacterHalfHeight));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CapsuleComponent is not initialized."));
	}

	NavModifier = CreateDefaultSubobject<UNavModifierComponent>(TEXT("NavModifier"));
	if (NavModifier)
	{
		NavModifier->FailsafeExtent = FVector(5.f, 5.f, 100.f);
	}

	// 캐릭터 이동 컴포넌트 설정
	if (UCharacterMovementComponent* CharacterMovementComp = GetCharacterMovement())
	{
		CharacterMovementComp->bOrientRotationToMovement = false;
		CharacterMovementComp->bUseControllerDesiredRotation = true;
		CharacterMovementComp->RotationRate = FRotator(0.f, 480.f, 0.f);
		CharacterMovementComp->MaxWalkSpeed = 500.f;
		CharacterMovementComp->bRequestedMoveUseAcceleration = true;

		//회피 관련 설정 (사용하는 경우에만 활성화)
		CharacterMovementComp->bUseRVOAvoidance = true;
		CharacterMovementComp->AvoidanceWeight = 1.0f;
		CharacterMovementComp->AvoidanceConsiderationRadius = 10.f;
		CharacterMovementComp->MaxAcceleration = 2048.0f;
		CharacterMovementComp->bRequestedMoveUseAcceleration = true;
		CharacterMovementComp->SetAvoidanceGroup(1);
		CharacterMovementComp->SetGroupsToAvoid(1);
		CharacterMovementComp->SetGroupsToIgnore(0);
	}

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (CharacterMesh)
	{
		FVector PivotPosition(0.f, 0.f, -60.f);
		FRotator PivotRotation(0.f, -90.f, 0.f);
		float Scale = 0.8f;

		CharacterMesh->SetRelativeLocationAndRotation(PivotPosition, PivotRotation);
		CharacterMesh->SetRelativeScale3D(FVector(Scale));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Character mesh is not initialized."));
	}

	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;

	bReplicates = true;
	SetReplicateMovement(true);
	StatComponent->SetIsReplicated(true);
	ActionStatComponent->SetIsReplicated(true);

	CharacterName = NAME_None;

	ReplicatedSkeletalMesh = nullptr;

	RelativeDirection = 0;

	ObjectType = EObjectType::Minion;

	// 경험치 공유 반경 기본값 설정
	ExperienceShareRadius = 1000.f;

	ShareFactor.Empty();

	// Bounty 관련 초기값 설정
	ExpBounty = 0.f;
	GoldBounty = 0;

	// MinionBase 관련 설정 (래그돌 및 힘)
	RagdollBlendTime = 0.2f;
	ImpulseStrength = 1000.0f;

	// 최대 추적 거리 설정
	ChaseThreshold = 1000.0f;

	// Fade 관련 변수 초기화
	CurrentFadeDeath = 0.0f;
	FadeOutDuration = 1.0f;
}

void AMinionBase::InitializeCharacterResources()
{
	UWorld* CurrentWorld = GetWorld();
	if (!CurrentWorld)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeCharacterResources: Failed to retrieve the World context. Unable to proceed with initialization."));
		return;
	}

	UAOSGameInstance* GameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(CurrentWorld));
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeCharacterResources: Invalid GameInstance."));
		return;
	}

	const FMinionAttributesRow* MinionsListRow = GameInstance->GetMinionsListTableRow(CharacterName);
	if (!MinionsListRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to get MinionsListRow for MinionType: %s"), ANSI_TO_TCHAR(__FUNCTION__), *CharacterName.ToString());
		return;
	}

	UDataTable* ResourcesTable = MinionsListRow->ResourcesTable;
	if (!ResourcesTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] CharacterResourcesTable is null for MinionType: %s"), ANSI_TO_TCHAR(__FUNCTION__), *CharacterName.ToString());
		return;
	}

	const FCharacterGamePlayDataRow* DataRow = ResourcesTable->FindRow<FCharacterGamePlayDataRow>(FName(*FString::FromInt(1)), TEXT(""));
	if (!DataRow)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeCharacterResources: Failed to find FCharacterGamePlayDataRow in DataTable for %s."), *CharacterName.ToString());
		return;
	}

	GameplayMontages = DataRow->GetGamePlayMontagesMap();
	GameplayParticles = DataRow->GetGamePlayParticlesMap();
	GameplayMeshes = DataRow->GetGamePlayMeshesMap();
	GameplayMaterials = DataRow->GetGamePlayMaterialsMap();
	GameplayClasses = DataRow->GetGamePlayClassesMap();
}

void AMinionBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMinionBase, ReplicatedSkeletalMesh);
}

void AMinionBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (StatComponent->OnOutOfCurrentHP.IsAlreadyBound(this, &ThisClass::OnCharacterDeath) == false)
		{
			StatComponent->OnOutOfCurrentHP.AddDynamic(this, &ThisClass::OnCharacterDeath);
		}

		UParticleSystem* Particle = GetOrLoadSharedParticle(TEXT("MinionSpawn"), TEXT("/Game/ParagonMinions/FX/Particles/Minions/Shared/P_MinionSpawn.P_MinionSpawn"));
		if (Particle)
		{
			SpawnEmitterAtLocation(Particle, FTransform(FRotator(0), GetActorLocation(), FVector(1)));
		}
	}

	AnimInstance = Cast<UMinionAnimInstance>(GetMesh()->GetAnimInstance());
	if (::IsValid(AnimInstance))
	{
		AnimInstance->OnMontageEnded.AddDynamic(this, &ThisClass::MontageEnded);
	}
}

void AMinionBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateWidgetToLocalPlayer();
}

void AMinionBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		UAOSGameInstance* GameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
		if (!GameInstance) return;

		const FMinionAttributesRow* MinionsListRow = GameInstance->GetMinionsListTableRow(CharacterName);
		if (!MinionsListRow) return;

		UDataTable* StatTable = MinionsListRow->StatTable;
		if (!StatTable) return;

		UDataTable* AbilityStatTable = MinionsListRow->AbilityStatTable;
		if (!AbilityStatTable) return;

		UMinionStatComponent* MinionStatComponent = Cast<UMinionStatComponent>(StatComponent);
		if (MinionStatComponent)
		{
			MinionStatComponent->InitStatComponent(StatTable);
			MinionStatComponent->OnOutOfCurrentHP.AddDynamic(this, &AMinionBase::OnCharacterDeath);
		}

		UActionStatComponent* MinionActionStatComponent = Cast<UActionStatComponent>(ActionStatComponent);
		if (MinionActionStatComponent)
		{
			ActionStatComponent->InitActionStatComponent(AbilityStatTable, MinionStatComponent);
			ActionStatComponent->InitializeActionAtLevel(EActionSlot::LMB, 1);
		}
	}

	StatComponent->OnCharacterStatReplicated.AddDynamic(this, &AMinionBase::InitializeWidget);

	InitializeCharacterResources();
}



void AMinionBase::InitializeWidget()
{
	UUserWidget* Widget = WidgetComponent->GetWidget();
	if (::IsValid(Widget) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Widget is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	HPBar = Cast<UUserWidgetBarBase>(Widget);
	if (::IsValid(HPBar) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] HPBar cast failed."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	ACharacterBase* LocalPlayerCharacter = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (::IsValid(LocalPlayerCharacter))
	{
		FLinearColor Color = LocalPlayerCharacter->TeamSide == TeamSide ? FLinearColor::Blue : FLinearColor::Red;
		HPBar->SetProgressBarColor(Color);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] LocalPlayerCharacter is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
	}

	HPBar->SetBorderColor(FLinearColor(0.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f));
	HPBar->SetTextVisibility(ESlateVisibility::Hidden);
	StatComponent->OnMaxHPChanged.AddDynamic(HPBar, &UUserWidgetBarBase::OnMaxMaxFigureChanged);
	StatComponent->OnCurrentHPChanged.AddDynamic(HPBar, &UUserWidgetBarBase::OnCurrentFigureChanged);

	float MaxHP = StatComponent->GetMaxHP();
	HPBar->InitializeWidget(MaxHP, MaxHP, 0);

	if (StatComponent->OnCharacterStatReplicated.IsAlreadyBound(this, &AMinionBase::InitializeWidget))
	{
		StatComponent->OnCharacterStatReplicated.RemoveDynamic(this, &AMinionBase::InitializeWidget);
	}
}


void AMinionBase::MontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::MontageEnded] Montage is null."));
		return;
	}

	if (Montage->GetFName() == FName("Death_Montage"))
	{
		if (USkeletalMeshComponent* MeshComponent = GetMesh())
		{
			MeshComponent->SetCollisionProfileName("Ragdoll");
			MeshComponent->SetAllBodiesSimulatePhysics(true);
			MeshComponent->SetSimulatePhysics(true);
			MeshComponent->SetPhysicsBlendWeight(0.0f);

			GetWorld()->GetTimerManager().SetTimer(DeathMontageTimerHandle, this, &AMinionBase::EnableRagdoll, RagdollBlendTime, false);
			GetWorld()->GetTimerManager().SetTimer(FadeOutTimerHandle, this, &AMinionBase::StartFadeOut, 0.05f, true, 2.0f);
		}
	}
}


void AMinionBase::OnCharacterDeath()
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (EnumHasAnyFlags(CharacterState, ECharacterState::Death))
	{
		return;
	}

	bool bIsDeathInProgress = true;
	if (OnPreDeathEvent.IsBound())
	{
		OnPreDeathEvent.Broadcast(bIsDeathInProgress);
	}

	if (!bIsDeathInProgress)
	{
		return;
	}

	// 네비게이션 장애물 해제
	ChangeNavModifierAreaClass(UNavArea_Default::StaticClass());

	// 충돌 해제
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 전체 충돌 해제
	SetActorEnableCollision(false);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	SetActorTickEnabled(false);

	// HP 콜백 제거
	StatComponent->OnOutOfCurrentHP.RemoveDynamic(this, &ThisClass::OnCharacterDeath);

	// Death 상태 설정 및 Multicast 호출
	OnCharacterDeath_Multicast();
	EnumAddFlags(CharacterState, ECharacterState::Death);

	AMinionAIController* AIC = Cast<AMinionAIController>(GetController());
	if (AIC)
	{
		AIC->EndAI();
	}

	// 상대 방향 계산 및 유효한 방향으로 설정
	RelativeDirection = GetRelativeDirection(LastHitCharacter);
	if (RelativeDirection <= 0)
	{
		RelativeDirection = 1;
	}

	if (::IsValid(AnimInstance))
	{
		AnimInstance->StopAllMontages(0.0f);
	}

	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (MovementComponent)
	{
		MovementComponent->StopMovementImmediately();
	}

	// Death Montage 로드 및 처리
	UAnimMontage* DeathMontage = GetOrLoadMontage("Death", *FString::Printf(TEXT("/Game/ParagonMinions/Characters/Minions/Down_Minions/Animations/%s/Death_Montage.Death_Montage"), *CharacterName.ToString()));
	if (!DeathMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::OnCharacterDeath] Failed to load DeathMontage."));
		SetActorHiddenInGame(true);
	}
	else
	{
		FName SectionName = FName(*FString::Printf(TEXT("Death%d"), RelativeDirection));
		PlayAnimMontage(DeathMontage, 1.0f, SectionName);
		ServerPlayMontage(DeathMontage, 1.0f, SectionName);
	}

	// 게임 모드 및 경험치/골드 분배
	AArenaGameMode* GM = Cast<AArenaGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::OnCharacterDeath] Failed to get game mode."));
		return;
	}

	// 근처 적군 찾기 및 경험치 분배
	TArray<ACharacterBase*> NearbyEnemies;
	FindNearbyPlayers(NearbyEnemies, this->TeamSide == ETeamSide::Blue ? ETeamSide::Red : ETeamSide::Blue, ExperienceShareRadius);

	ACharacterBase* Eliminator = Cast<ACharacterBase>(LastHitCharacter);
	if (Eliminator && EnumHasAnyFlags(Eliminator->ObjectType, EObjectType::Player))
	{
		GM->AddCurrencyToPlayer(Eliminator, GoldBounty);
	}

	DistributeExperience(Eliminator, NearbyEnemies);

	if (OnPostDeathEvent.IsBound())
	{
		OnPostDeathEvent.Broadcast(this, LastHitCharacter);
	}
}


void AMinionBase::OnCharacterDeath_Multicast_Implementation()
{
	// HPBar의 유효성 검사 후 숨김 처리
	if (HPBar && HPBar->IsValidLowLevel())
	{
		HPBar->SetVisibility(ESlateVisibility::Hidden);
	}

	// 캡슐 컴포넌트의 유효성 검사 후 충돌 비활성화
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (CapsuleComp && CapsuleComp->IsValidLowLevel())
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CapsuleComponent is invalid or null in OnCharacterDeath_Multicast_Implementation"));
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (MeshComponent && MeshComponent->IsValidLowLevel())
	{
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CapsuleComponent is invalid or null in OnCharacterDeath_Multicast_Implementation"));
	}
}


void AMinionBase::EnableRagdoll()
{
	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshComponent->SetPhysicsBlendWeight(1.0f);
	}

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->DisableMovement();
		MovementComponent->StopMovementImmediately();
	}
}

void AMinionBase::ApplyDirectionalImpulse()
{
	FVector ImpulseDirection = FVector::ZeroVector;

	// 맞은 방향에 따라 임펄스 방향 설정
	switch (RelativeDirection)
	{
	case 1: // 앞
		ImpulseDirection = -GetActorForwardVector();
		break;
	case 2: // 뒤
		ImpulseDirection = GetActorForwardVector();
		break;
	case 3: // 오른쪽
		ImpulseDirection = -GetActorRightVector();
		break;
	case 4: // 왼쪽
		ImpulseDirection = GetActorRightVector();
		break;
	default:
		break;
	}

	FVector Impulse = ImpulseDirection * ImpulseStrength;

	GetMesh()->AddImpulse(Impulse, NAME_None, true);

	// Draw a debug directional arrow to represent the impulse
	FVector StartLocation = GetMesh()->GetComponentLocation();
	FVector EndLocation = StartLocation + Impulse;
}

void AMinionBase::MulticastApplyImpulse_Implementation(FVector Impulse)
{
	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{

	}
}

void AMinionBase::StartFadeOut()
{
	CurrentFadeDeath += (0.05f / FadeOutDuration);
	if (CurrentFadeDeath >= 1.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(FadeOutTimerHandle);
		CurrentFadeDeath = 1.0f;
		if (HasAuthority()) Destroy(true, true);
	}

	// 메쉬의 머티리얼 투명도 업데이트
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (MeshComponent)
	{
		int32 MaterialCount = MeshComponent->GetNumMaterials();
		for (int32 i = 0; i < MaterialCount; i++)
		{
			UMaterialInstanceDynamic* DynMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(i);
			if (DynMaterial)
			{
				DynMaterial->SetScalarParameterValue(FName("FadeOut"), CurrentFadeDeath);
			}
		}
	}
}

void AMinionBase::DistributeExperience(ACharacterBase* Eliminator, const TArray<ACharacterBase*>& NearbyEnemies)
{
	AArenaGameMode* GM = Cast<AArenaGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GM)
	{
		return;
	}

	int32 NearbyAlliesCount = NearbyEnemies.Num() + (Eliminator ? 1 : 0);
	float ShareFactorValue = 1.0f;

	if (NearbyAlliesCount > 0)
	{
		if (ShareFactor.Contains(NearbyAlliesCount))
		{
			ShareFactorValue = ShareFactor[NearbyAlliesCount];
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No share factor defined for %d players"), NearbyAlliesCount);
		}

		float TotalExp = ExpBounty * ShareFactorValue;
		float SharedExpBounty = TotalExp / NearbyAlliesCount;

		if (Eliminator && EnumHasAnyFlags(Eliminator->ObjectType, EObjectType::Player))
		{
			GM->AddExpToPlayer(Eliminator, SharedExpBounty);
		}

		for (auto& Player : NearbyEnemies)
		{
			GM->AddExpToPlayer(Player, SharedExpBounty);
		}
	}
}

void AMinionBase::FindNearbyPlayers(TArray<ACharacterBase*>& PlayerCharacters, ETeamSide InTeamSide, float Distance)
{
	// Overlap detection
	TArray<FOverlapResult> OutHits;
	FVector CollisionBoxSize = FVector(2 * Distance, 2 * Distance, 2 * Distance); // 박스의 한 변의 길이를 원의 지름으로 설정
	FVector OverlapLocation = GetActorLocation();
	FCollisionQueryParams Params(NAME_None, false, this);

	bool bResult = GetWorld()->OverlapMultiByChannel(
		OutHits,
		OverlapLocation,
		FQuat::Identity,
		ECC_GameTraceChannel4,
		FCollisionShape::MakeBox(CollisionBoxSize),
		Params
	);

	if (bResult)
	{
		for (const auto& OutHit : OutHits)
		{
			AActor* OverlappedActor = OutHit.GetActor();
			if (!::IsValid(OverlappedActor) || OverlappedActor == LastHitCharacter)
			{
				continue;
			}

			ACharacterBase* OverlappedCharacter = Cast<ACharacterBase>(OverlappedActor);
			if (!::IsValid(OverlappedCharacter))
			{
				continue;
			}

			if (OverlappedCharacter->ObjectType != EObjectType::Player)
			{
				continue;
			}

			if (TeamSide == OverlappedCharacter->TeamSide)
			{
				continue;
			}

			// 추가적인 거리 확인
			float DistanceToPlayer = FVector::Dist(OverlapLocation, OverlappedCharacter->GetActorLocation());
			if (DistanceToPlayer <= Distance)
			{
				PlayerCharacters.Add(OverlappedCharacter);
			}
		}
	}
}
int32 AMinionBase::GetRelativeDirection(AActor* OtherActor) const
{
	if (::IsValid(OtherActor) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AMinionBase::GetRelativeDirection] OtherActor is null."));
		return -1; // OtherActor가 유효하지 않은 경우
	}

	// 현재 캐릭터와 상대 캐릭터의 위치 벡터
	FVector MyLocation = GetActorLocation();
	FVector OtherLocation = OtherActor->GetActorLocation();

	// 상대 캐릭터 방향 벡터
	FVector DirectionToOther = (OtherLocation - MyLocation).GetSafeNormal();

	// 현재 캐릭터의 앞을 향하는 방향 벡터
	FVector ForwardVector = GetActorForwardVector();

	// Dot Product를 이용하여 방향 계산
	float ForwardDot = FVector::DotProduct(GetActorForwardVector(), DirectionToOther);
	float RightDot = FVector::DotProduct(GetActorRightVector(), DirectionToOther);

	if (ForwardDot > 0.0f && FMath::Abs(ForwardDot) > FMath::Abs(RightDot))
	{
		return 1; // 앞
	}
	else if (ForwardDot < 0.0f && FMath::Abs(ForwardDot) > FMath::Abs(RightDot))
	{
		return 2; // 뒤
	}
	else if (RightDot > 0.0f && FMath::Abs(RightDot) > FMath::Abs(ForwardDot))
	{
		return 3; // 오른쪽
	}
	else if (RightDot < 0.0f && FMath::Abs(RightDot) > FMath::Abs(ForwardDot))
	{
		return 4; // 왼쪽
	}

	// 기본값 반환
	return 0;
}

float AMinionBase::GetExpBounty() const
{
	return ExpBounty;
}

void AMinionBase::SetExpBounty(float NewExpBounty)
{
	ExpBounty = NewExpBounty;
}

int32 AMinionBase::GetGoldBounty() const
{
	return GoldBounty;
}

void AMinionBase::SetGoldBounty(int32 NewGoldBounty)
{
	GoldBounty = NewGoldBounty;
}

void AMinionBase::OnRep_SkeletalMesh()
{
	if (ReplicatedSkeletalMesh)
	{
		GetMesh()->SetSkeletalMesh(ReplicatedSkeletalMesh);
	}
}




void AMinionBase::ChangeNavModifierAreaClass(TSubclassOf<UNavArea> NewAreaClass)
{
	if (NavModifier)
	{
		NavModifier->SetAreaClass(NewAreaClass);

		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		if (NavSys)
		{
			NavSys->UpdateComponentInNavOctree(*NavModifier);
		}
	}
}