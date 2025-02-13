// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/AOSCharacterBase.h"

// Game 관련 헤더 파일
#include "Game/ArenaGameMode.h"
#include "Game/ArenaGameState.h"
#include "Game/ArenaPlayerState.h"
#include "Game/AOSGameInstance.h"

// Input 관련 헤더 파일
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Inputs/InputConfigData.h"

// Camera 관련 헤더 파일
#include "Camera/CameraComponent.h"

// Animation 관련 헤더 파일
#include "Animations/PlayerAnimInstance.h"

// Component 관련 헤더 파일
#include "Components/CapsuleComponent.h"
#include "Components/StatComponent.h"
#include "Components/ActionStatComponent.h"
#include "Components/CharacterWidgetComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Controller 관련 헤더 파일
#include "Controllers/AOSPlayerController.h"

// Kismet 관련 헤더 파일
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

// UI 관련 헤더 파일
#include "UI/UW_StateBar.h"

// Timer 관련 헤더 파일
#include "TimerManager.h"

// Network 관련 헤더 파일
#include "Net/UnrealNetwork.h"

// Engine 관련 헤더 파일
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/PostProcessVolume.h"
#include "EngineUtils.h"

// 기타 유틸리티
#include "Structs/ActionData.h"
#include "Structs/CharacterResources.h"
#include "Plugins/UniqueCodeGenerator.h"



AAOSCharacterBase::AAOSCharacterBase()
{
	bReplicates = true;
	SetReplicateMovement(true);

	// ----- Components Initialization -----
	WidgetComponent = CreateDefaultSubobject<UCharacterWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetupAttachment(GetRootComponent());
	WidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 130.f));

	static ConstructorHelpers::FClassFinder<UUserWidgetBase> STATE_BAR(TEXT("/Game/FuryOfLegends/UI/Blueprints/HUD/WBP_StateBar.WBP_StateBar_C"));
	if (STATE_BAR.Succeeded())
	{
		WidgetComponent->SetWidgetClass(STATE_BAR.Class);
		WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		WidgetComponent->SetDrawSize(FVector2D(200.0f, 35.0f));
		WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Initialize the Spring Arm Component
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	if (SpringArmComponent)
	{
		SpringArmComponent->SetupAttachment(RootComponent);
		SpringArmComponent->TargetArmLength = 500.f;
		SpringArmComponent->bUsePawnControlRotation = true;
		SpringArmComponent->bDoCollisionTest = true;
		SpringArmComponent->bInheritPitch = true;
		SpringArmComponent->bInheritYaw = true;
		SpringArmComponent->bInheritRoll = false;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SpringArmComponent initialization failed."));
	}

	// Initialize the Camera Component
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	if (CameraComponent)
	{
		CameraComponent->SetupAttachment(SpringArmComponent);
		CameraComponent->SetRelativeLocation(FVector(0.f, 80.f, 100.f));
		CameraComponent->PostProcessSettings.bOverride_MotionBlurAmount = true;
		CameraComponent->PostProcessSettings.MotionBlurAmount = 0.0f;  // Disable motion blur
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraComponent initialization failed."));
	}

	// Initialize the Screen Particle System
	ScreenParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ScreenParticleSystem"));
	if (ScreenParticleSystem)
	{
		ScreenParticleSystem->SetupAttachment(CameraComponent);
		ScreenParticleSystem->SetRelativeLocation(FVector(40.f, 0.f, 0.f));
		ScreenParticleSystem->SetRelativeScale3D(FVector(0.5f, 0.5f, 0.5f));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ScreenParticleSystem initialization failed."));
	}

	// ----- Character Movement -----
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Ensure CharacterMovementComponent is valid
	UCharacterMovementComponent* CharacterMovementComp = GetCharacterMovement();
	if (CharacterMovementComp)
	{
		CharacterMovementComp->MaxWalkSpeed = 100.f;
		CharacterMovementComp->MinAnalogWalkSpeed = 20.f;
		CharacterMovementComp->JumpZVelocity = 400.f;
		CharacterMovementComp->AirControl = 0.35f;
		CharacterMovementComp->GravityScale = 1.0f;
		CharacterMovementComp->BrakingDecelerationWalking = 2000.f;
		CharacterMovementComp->bOrientRotationToMovement = false;
		CharacterMovementComp->bUseControllerDesiredRotation = false;
		CharacterMovementComp->RotationRate = FRotator(0.f, 400.f, 0.f);
		CharacterMovementComp->SetIsReplicated(true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CharacterMovement initialization failed."));
	}

	// Ensure CapsuleComponent is valid
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (CapsuleComp)
	{
		float CharacterHalfHeight = 95.f;
		float CharacterRadius = 40.f;
		CapsuleComp->InitCapsuleSize(CharacterRadius, CharacterHalfHeight);

		FVector PivotPosition(0.f, 0.f, -CharacterHalfHeight);
		FRotator PivotRotation(0.f, -90.f, 0.f);
		GetMesh()->SetRelativeLocationAndRotation(PivotPosition, PivotRotation);

		CapsuleComp->SetCollisionProfileName(TEXT("AOSCharacter"));

		HomingTargetSceneComponent->SetRelativeLocation(FVector(0, 0, 30));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CapsuleComponent initialization failed."));
	}

	// ----- Input Config -----
	PlayerCharacterInputConfigData = nullptr;
	PlayerInputMappingContext = nullptr;
	PlayerCtrlInputMappingContext = nullptr;
	EnhancedInputComponent = nullptr;
	PostProcessVolume = nullptr;

	// ----- UI -----
	StateBar = nullptr;

	// ----- Character State -----
	ForwardInputValue = 0.0f;
	PreviousForwardInputValue = 0.0f;
	RightInputValue = 0.0f;
	PreviousRightInputValue = 0.0f;
	CurrentAimPitch = 0.0f;
	PreviousAimPitch = 0.0f;
	CurrentAimYaw = 0.0f;
	PreviousAimYaw = 0.0f;

	// Character Flags
	bCtrlKeyPressed = false;

	// ----- Character Metadata -----
	CharacterName = NAME_None;
	SelectedCharacterIndex = -1;

	// ----- Combat Stats -----
	BasicAttackAnimPlayRate = 1.0f;
	BasicAttackAnimLength = 0.0f;

	// ----- Overlap and Transform Data -----
	LastCharacterRotation = FRotator::ZeroRotator;
	LastCharacterLocation = FVector::ZeroVector;
	LastForwardVector = FVector::ZeroVector;
	LastRightVector = FVector::ZeroVector;
	LastUpVector = FVector::ZeroVector;

	// ----- Combat Materials -----
	OverlayMaterial_Ally = nullptr;
	OverlayMaterial_Enemy = nullptr;
	OriginalMaterial = nullptr;

	// ----- Object Type -----
	ObjectType = EObjectType::Player;

	// ----- Combat Counters -----
	TotalAttacks = 0;
	CriticalHits = 0;

	// Enable ticking for this actor
	PrimaryActorTick.bCanEverTick = true;

	CurrentTarget = nullptr;

	TargetLocations.Add(EActionSlot::Q, FVector(0));
	TargetLocations.Add(EActionSlot::E, FVector(0));
	TargetLocations.Add(EActionSlot::R, FVector(0));
	TargetLocations.Add(EActionSlot::LMB, FVector(0));
	TargetLocations.Add(EActionSlot::RMB, FVector(0));

	KeyInputTimestamps.Add(EActionSlot::Q, 0);
	KeyInputTimestamps.Add(EActionSlot::E, 0);
	KeyInputTimestamps.Add(EActionSlot::R, 0);
	KeyInputTimestamps.Add(EActionSlot::LMB, 0);
	KeyInputTimestamps.Add(EActionSlot::RMB, 0);
	KeyElapsedTimes = KeyInputTimestamps;
}

void AAOSCharacterBase::InitializeCharacterResources()
{
	Super::InitializeCharacterResources();

	UWorld* CurrentWorld = GetWorld();
	if (!CurrentWorld)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeCharacterResources: Failed to retrieve the World context. Unable to proceed with initialization."));
		return;
	}

	UAOSGameInstance* GameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeCharacterResources: Failed to cast to UAOSGameInstance. Make sure the GameInstance class is set correctly in the project settings."));
		return;
	}

	const FCharacterAttributesRow* ChampionsListRow = GameInstance->GetChampionListTableRow(CharacterName);
	if (!ChampionsListRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to get ChampionsListRow for ChampionName: %s"), ANSI_TO_TCHAR(__FUNCTION__), *CharacterName.ToString());
		return;
	}

	UDataTable* ResourcesTable = ChampionsListRow->CharacterResourcesTable;
	if (!ResourcesTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] CharacterResourcesTable is null for ChampionName: %s"), ANSI_TO_TCHAR(__FUNCTION__), *CharacterName.ToString());
		return;
	}

	const FCharacterGamePlayDataRow* DataRow = ResourcesTable->FindRow<FCharacterGamePlayDataRow>(FName(*FString::FromInt(1)), TEXT(""));
	if (!DataRow)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeCharacterResources: Failed to find row in DataTable for %s."), *CharacterName.ToString());
		return;
	}

	GameplayMontages = DataRow->GetGamePlayMontagesMap();
	GameplayParticles = DataRow->GetGamePlayParticlesMap();
	GameplayMeshes = DataRow->GetGamePlayMeshesMap();
	GameplayMaterials = DataRow->GetGamePlayMaterialsMap();
	GameplayClasses = DataRow->GetGamePlayClassesMap();
	GameplayTextures = DataRow->GetGamePlayTexturesMap();
}





void AAOSCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AAOSPlayerController* PlayerController = Cast<AAOSPlayerController>(GetController());
	if (!HasAuthority() && PlayerController && PlayerController == UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (PreviousForwardInputValue != ForwardInputValue || PreviousRightInputValue != RightInputValue)
		{
			UpdateInputValue_Server(ForwardInputValue, RightInputValue);
		}
	}
}

//==================== Replication Functions ====================//

void AAOSCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, ForwardInputValue, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ThisClass, RightInputValue, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ThisClass, CurrentAimPitch, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ThisClass, CurrentAimYaw, COND_SkipOwner);
}

void AAOSCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle NewTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(NewTimerHandle, this, &ThisClass::PostCharacterSpawn, 0.2f, false, 0.2f);

	// 애니메이션 인스턴스 설정
	UPlayerAnimInstance* PlayerAnimInstance = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	if (::IsValid(PlayerAnimInstance))
	{
		AnimInstance = PlayerAnimInstance;

		PlayerAnimInstance->OnEnableMoveNotifyBegin.BindUObject(this, &AAOSCharacterBase::EnableCharacterMove);
		PlayerAnimInstance->OnEnableSwitchActionNotifyBegin.BindUObject(this, &AAOSCharacterBase::EnableSwitchAction);
		PlayerAnimInstance->OnEnableGravityNotifyBegin.AddDynamic(this, &AAOSCharacterBase::EnableGravity);
		PlayerAnimInstance->OnDisableGravityNotifyBegin.AddDynamic(this, &AAOSCharacterBase::DisableGravity);

		PlayerAnimInstance->OnMontageEnded.AddDynamic(this, &AAOSCharacterBase::MontageEnded);
		PlayerAnimInstance->OnActionNotifyTriggered.AddDynamic(this, &AAOSCharacterBase::HandleActionNotifyEvent);
	}

	ArenaGameState = Cast<AArenaGameState>(UGameplayStatics::GetGameState(this));
	if (HasAuthority() && ::IsValid(ArenaGameState))
	{
		// 필요한 게임 상태 초기화 코드 추가

	}

	InitializeDataStatus();
}


void AAOSCharacterBase::PostCharacterSpawn()
{
	ArenaPlayerState = Cast<AArenaPlayerState>(GetPlayerState());

	// 서버에서 실행
	if (HasAuthority())
	{
		if (::IsValid(ArenaPlayerState))
		{
			TeamSide = (ArenaPlayerState->TeamSide == ETeamSide::Blue) ? ETeamSide::Blue : ETeamSide::Red;
			StatComponent->OnCurrentLevelChanged.AddDynamic(ArenaPlayerState, &AArenaPlayerState::OnPlayerLevelChanged);
		}
	}

	// 로컬 클라이언트에서 실행
	AAOSPlayerController* PlayerController = Cast<AAOSPlayerController>(Controller);
	if (PlayerController && PlayerController->IsLocalPlayerController() /*PlayerController && PlayerController == UGameplayStatics::GetPlayerController(GetWorld(), 0)*/)
	{
		PlayerController->InitializeItemShop();
		PlayerController->InitializeHUD(CharacterName);

		GetWorld()->GetTimerManager().SetTimer(CheckEnemyOnScreenTimer, this, &ThisClass::CheckOutOfSight, 0.1f, true);
		GetWorld()->GetTimerManager().SetTimer(TargetMaterialChangeTimer, this, &ThisClass::UpdateOverlayMaterial, 0.1f, true);

		// 입력 서브시스템에서 매핑 컨텍스트 추가
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(PlayerInputMappingContext, 0);
		}

		// 월드 내 PostProcessVolume을 찾고 캐싱
		for (TActorIterator<APostProcessVolume> It(GetWorld()); It; ++It)
		{
			if (It->bUnbound)
			{
				PostProcessVolume = *It;
				break;
			}
		}
	}
}



void AAOSCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(CheckEnemyOnScreenTimer);
	GetWorld()->GetTimerManager().ClearTimer(TargetMaterialChangeTimer);

	DeactivateHealthRegenTimer();
	DeactivateManaRegenTimer();
}

void AAOSCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		UWorld* World = GetWorld();
		if (::IsValid(World) == false)
		{
			UE_LOG(LogTemp, Error, TEXT("[AAOSCharacterBase::PostInitializeComponents] WorldContext is not valid."));
			return;
		}

		UAOSGameInstance* GameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(World));
		if (!GameInstance) return;

		const FCharacterAttributesRow* ChampionsListRow = GameInstance->GetChampionListTableRow(CharacterName);
		if (!ChampionsListRow) return;

		UDataTable* StatTable = ChampionsListRow->StatTable;
		if (!StatTable) return;

		UDataTable* ActionStatTable = ChampionsListRow->ActionStatTable;
		if (!ActionStatTable) return;

		StatComponent->InitStatComponent(StatTable);
		ActionStatComponent->InitActionStatComponent(ActionStatTable, StatComponent);

		StatComponent->OnHealthDepleted.AddDynamic(this, &AAOSCharacterBase::ActivateHealthRegenTimer);
		StatComponent->OnManaDepleted.AddDynamic(this, &AAOSCharacterBase::ActivateManaRegenTimer);
		StatComponent->OnHealthFull.AddDynamic(this, &AAOSCharacterBase::DeactivateHealthRegenTimer);
		StatComponent->OnManaFull.AddDynamic(this, &AAOSCharacterBase::DeactivateManaRegenTimer);
		StatComponent->OnOutOfCurrentHP.AddDynamic(this, &AAOSCharacterBase::OnCharacterDeath);
		StatComponent->OnCurrentLevelChanged.AddDynamic(this, &AAOSCharacterBase::SpawnLevelUpParticle);
		StatComponent->OnCurrentLevelChanged.AddDynamic(ActionStatComponent, &UActionStatComponent::ServerUpdateUpgradableStatus);
	}

	InitializeCharacterResources();
}


void AAOSCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [&]()
		{
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->SwitchInputMappingContext_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::OnCtrlKeyPressed);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->SwitchInputMappingContext_Action, ETriggerEvent::Triggered, this, &AAOSCharacterBase::OnCtrlKeyReleased);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Upgrade_Q_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::ServerUpgradeAction, EActionSlot::Q);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Upgrade_E_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::ServerUpgradeAction, EActionSlot::E);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Upgrade_R_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::ServerUpgradeAction, EActionSlot::R);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Upgrade_LMB_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::ServerUpgradeAction, EActionSlot::LMB);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Upgrade_RMB_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::ServerUpgradeAction, EActionSlot::RMB);

			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->MoveAction, ETriggerEvent::Started, this, &AAOSCharacterBase::Move_Started);

			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->UseItem_1_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::UseItemSlot, 0);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->UseItem_2_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::UseItemSlot, 1);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->UseItem_3_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::UseItemSlot, 2);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->UseItem_4_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::UseItemSlot, 3);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->UseItem_5_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::UseItemSlot, 4);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->UseItem_6_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::UseItemSlot, 5);

			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->ToggleItemShop_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::ToggleItemShop);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Recall_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::Recall);

			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Execute_Q_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::ProcessActionStarted, EActionSlot::Q);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Execute_E_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::ProcessActionStarted, EActionSlot::E);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Execute_R_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::ProcessActionStarted, EActionSlot::R);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Execute_LMB_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::ProcessActionStarted, EActionSlot::LMB);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Execute_RMB_Action, ETriggerEvent::Started, this, &AAOSCharacterBase::ProcessActionStarted, EActionSlot::RMB);

			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Execute_Q_Action, ETriggerEvent::Ongoing, this, &AAOSCharacterBase::ProcessActionOngoing, EActionSlot::Q);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Execute_E_Action, ETriggerEvent::Ongoing, this, &AAOSCharacterBase::ProcessActionOngoing, EActionSlot::E);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Execute_R_Action, ETriggerEvent::Ongoing, this, &AAOSCharacterBase::ProcessActionOngoing, EActionSlot::R);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Execute_LMB_Action, ETriggerEvent::Ongoing, this, &AAOSCharacterBase::ProcessActionOngoing, EActionSlot::LMB);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Execute_RMB_Action, ETriggerEvent::Ongoing, this, &AAOSCharacterBase::ProcessActionOngoing, EActionSlot::RMB);

			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Execute_Q_Action, ETriggerEvent::Triggered, this, &AAOSCharacterBase::ProcessActionReleased, EActionSlot::Q);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Execute_E_Action, ETriggerEvent::Triggered, this, &AAOSCharacterBase::ProcessActionReleased, EActionSlot::E);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Execute_R_Action, ETriggerEvent::Triggered, this, &AAOSCharacterBase::ProcessActionReleased, EActionSlot::R);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Execute_LMB_Action, ETriggerEvent::Triggered, this, &AAOSCharacterBase::ProcessActionReleased, EActionSlot::LMB);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->Execute_RMB_Action, ETriggerEvent::Triggered, this, &AAOSCharacterBase::ProcessActionReleased, EActionSlot::RMB);

			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->DisPlayMousenAction, ETriggerEvent::Started, this, &AAOSCharacterBase::DisplayMouseCursor, true);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->DisPlayMousenAction, ETriggerEvent::Triggered, this, &AAOSCharacterBase::DisplayMouseCursor, false);
			EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->CallAFunctionAction, ETriggerEvent::Started, this, &AAOSCharacterBase::ExecuteSomethingSpecial);
		}));
}



void AAOSCharacterBase::Move_Started(const FInputActionValue& InValue)
{
	if (EnumHasAnyFlags(CharacterState, ECharacterState::Recall))
	{
		ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::Recall);
		ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::Move);
		ServerStopAllMontages(0.5f, true);
	}
}

void AAOSCharacterBase::InitializeDataStatus()
{
	if (::IsValid(ActionStatComponent) == false)
	{
		return;
	}

	for (int32 i = static_cast<int32>(EActionSlot::Q); i <= static_cast<int32>(EActionSlot::RMB); ++i)
	{
		EActionSlot ActionSlot = static_cast<EActionSlot>(i);

		const FActiveActionState& ActiveStateSlot = ActionStatComponent->GetActiveActionState(ActionSlot);
		switch (ActiveStateSlot.ActivationType)
		{
		case EActivationType::Passive:
		case EActivationType::Active:
		case EActivationType::Toggle:
		case EActivationType::Channeling:
			DefaultDataStatus.Add(ActionSlot, EDataStatus::Ready);
			break;
		case EActivationType::Charged:
		case EActivationType::Ranged:
			DefaultDataStatus.Add(ActionSlot, EDataStatus::Position);
			break;
		case EActivationType::Targeted:
			DefaultDataStatus.Add(ActionSlot, EDataStatus::Target);
			break;
		default:
			DefaultDataStatus.Add(ActionSlot, EDataStatus::Ready);
			break;
		}
	}

	DataStatus = DefaultDataStatus;
}

void AAOSCharacterBase::SetDataStatus(EActionSlot ActionSlot, EDataStatus NewStatus)
{
	if (!DefaultDataStatus.Contains(ActionSlot))
	{
		InitializeDataStatus();
	}

	if (!DefaultDataStatus.Contains(ActionSlot))  // 여전히 없으면 경고 로그 출력
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionSlot %d not found in DefaultDataStatus even after InitializeDataStatus()"), (int32)ActionSlot);
		return;
	}

	EDataStatus DefaultStatus = DefaultDataStatus.Contains(ActionSlot) ? DefaultDataStatus[ActionSlot] : EDataStatus::Ready;
	EDataStatus& CurrentStatus = DataStatus.FindOrAdd(ActionSlot, DefaultStatus);
	CurrentStatus &= ~NewStatus;

	if (CurrentStatus == EDataStatus::Ready)
	{
		ExecuteAction(ActionSlot);
		CurrentStatus = DefaultDataStatus[ActionSlot];
	}
}



void AAOSCharacterBase::ServerNotifyActionUse_Implementation(EActionSlot ActionSlot, ETriggerEvent TriggerEvent, float KeyElapsedTime)
{
	if (!HasAuthority())
	{
		return;
	}

	ServerProcessActionUse(ActionSlot, TriggerEvent, KeyElapsedTime);
}

void AAOSCharacterBase::ServerProcessActionUse(EActionSlot ActionSlot, ETriggerEvent TriggerEvent, float KeyElapsedTime)
{
	if (!HasAuthority())
	{
		return;
	}

	if (EnumHasAnyFlags(CharacterState, ECharacterState::Death))
	{
		return;
	}

	if (::IsValid(StatComponent) == false || ::IsValid(ActionStatComponent) == false || ::IsValid(AnimInstance) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] StatComponent, ActionStatComponent, or AnimInstance is null."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (ActionStatComponent->IsActionReady(ActionSlot) == false)
	{
		ActionStatComponent->ClientNotifyAlertTextChanged("The Action is not ready yet.");
		return;
	}

	KeyElapsedTimes[ActionSlot] = KeyElapsedTime;

	const FActiveActionState& ActiveStateSlot = ActionStatComponent->GetActiveActionState(ActionSlot);
	switch (ActiveStateSlot.ActivationType)
	{
	case EActivationType::Active:
		HandleActive(ActionSlot, TriggerEvent);
		break;
	case EActivationType::Toggle:
		HandleToggle(ActionSlot, TriggerEvent);
		break;
	case EActivationType::Channeling:
		HandleChanneling(ActionSlot, TriggerEvent);
		break;
	case EActivationType::Charged:
		HandleCharged(ActionSlot, TriggerEvent);
		break;
	case EActivationType::Targeted:
		HandleTargeted(ActionSlot, TriggerEvent);
		break;
	case EActivationType::Ranged:
		HandleRangedTargeted(ActionSlot, TriggerEvent);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("[%s] Unsupported ActivationType: %d"), ANSI_TO_TCHAR(__FUNCTION__), static_cast<int32>(ActiveStateSlot.ActivationType));
		break;
	}


}

void AAOSCharacterBase::HandleActive(EActionSlot ActionSlot, ETriggerEvent TriggerEvent)
{
	if (TriggerEvent == ETriggerEvent::Started)
	{
		ExecuteAction(ActionSlot);
	}
}

void AAOSCharacterBase::HandleToggle(EActionSlot ActionSlot, ETriggerEvent TriggerEvent)
{
	if (TriggerEvent == ETriggerEvent::Started)
	{
		ExecuteAction(ActionSlot);
	}
}

void AAOSCharacterBase::HandleChanneling(EActionSlot ActionSlot, ETriggerEvent TriggerEvent)
{
	if (TriggerEvent == ETriggerEvent::Started)
	{
		ExecuteAction(ActionSlot);
	}
}

void AAOSCharacterBase::HandleCharged(EActionSlot ActionSlot, ETriggerEvent TriggerEvent)
{
	if (TriggerEvent == ETriggerEvent::Started)
	{
		ActionStarted(ActionSlot);
	}
	else if (TriggerEvent == ETriggerEvent::Triggered)
	{
		ExecuteAction(ActionSlot);
	}
}

void AAOSCharacterBase::HandleTargeted(EActionSlot ActionSlot, ETriggerEvent TriggerEvent)
{
	if (TriggerEvent == ETriggerEvent::Started)
	{
		ExecuteAction(ActionSlot);
	}
}

void AAOSCharacterBase::HandleRangedTargeted(EActionSlot ActionSlot, ETriggerEvent TriggerEvent)
{
	if (TriggerEvent == ETriggerEvent::Started)
	{
		ActionStarted(ActionSlot);
	}
	else if (TriggerEvent == ETriggerEvent::Triggered)
	{
		ExecuteAction(ActionSlot);
	}
}


// 클라이언트에서 실행되는 함수들.
void AAOSCharacterBase::ProcessActionStarted(EActionSlot ActionSlot)
{
	if (bCtrlKeyPressed)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (::IsValid(ActionStatComponent) == false)
	{
		return;
	}

	if (bCtrlKeyPressed)
	{
		return;
	}

	if (!ActionStatComponent->IsActionReady(ActionSlot))
	{
		ActionStatComponent->OnAlertTextChanged.Broadcast("The Action is not ready yet.");
		return;
	}

	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (!CameraManager)
	{
		return;
	}

	// KeyInputTimestamps 맵에 현재 시간을 추가 (없으면 추가, 있으면 덮어씌움)
	float CurrentTime = World->GetTimeSeconds();
	KeyInputTimestamps[ActionSlot] = CurrentTime;

	ActionStarted(ActionSlot);
	ServerUpdateCameraLocation(CameraManager->GetCameraLocation());

	const FActiveActionState& ActiveStateSlot = ActionStatComponent->GetActiveActionState(ActionSlot);
	if (EnumHasAnyFlags(ActiveStateSlot.ActivationType, EActivationType::Targeted))
	{
		ServerUpdateTarget(ActionSlot, CurrentTarget);
	}

	ServerNotifyActionUse(ActionSlot, ETriggerEvent::Started, 0.0f);
}

void AAOSCharacterBase::ProcessActionOngoing(EActionSlot ActionSlot)
{
	if (bCtrlKeyPressed)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (bCtrlKeyPressed)
	{
		return;
	}

	float CurrentTime = World->GetTimeSeconds();
	if (KeyInputTimestamps.Contains(ActionSlot))
	{
		float TimeSinceKeyPress = CurrentTime - KeyInputTimestamps[ActionSlot];

		// 경과 시간을 KeyElapsedTimes 맵에 저장 또는 업데이트
		KeyElapsedTimes[ActionSlot] = TimeSinceKeyPress;
	}
}

void AAOSCharacterBase::ProcessActionReleased(EActionSlot ActionSlot)
{
	if (!KeyInputTimestamps.Contains(ActionSlot) || KeyInputTimestamps[ActionSlot] <= 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (::IsValid(ActionStatComponent) == false)
	{
		return;
	}

	if (bCtrlKeyPressed)
	{
		return;
	}

	if (ActionStatComponent->IsActionReady(ActionSlot) == false)
	{
		ActionStatComponent->OnAlertTextChanged.Broadcast("The Action is not ready yet.");
		return;
	}

	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (!CameraManager)
	{
		return;
	}

	ActionReleased(ActionSlot);

	const FActiveActionState& ActiveStateSlot = ActionStatComponent->GetActiveActionState(ActionSlot);
	switch (ActiveStateSlot.ActivationType)
	{
	case EActivationType::Charged:
		ServerUpdateCameraLocation(CameraManager->GetCameraLocation());
		ServerNotifyActionUse(ActionSlot, ETriggerEvent::Triggered, KeyElapsedTimes[ActionSlot]);
		break;
	case EActivationType::Ranged:
		ServerUpdateTargetLocation(ActionSlot, TargetLocations[ActionSlot]);
		ServerUpdateCameraLocation(CameraManager->GetCameraLocation());
		ServerNotifyActionUse(ActionSlot, ETriggerEvent::Triggered, KeyElapsedTimes[ActionSlot]);
		break;
	default:
		break;
	}

	KeyInputTimestamps[ActionSlot] = 0.0f;
	KeyElapsedTimes[ActionSlot] = 0.0f;
}


//==================== Action Upgrade Functions ====================//

 /**
  * 능력을 업그레이드하는 공통 로직을 처리합니다.
  * @param ActionSlot 업그레이드할 능력의 ID입니다.
  */
  // 각 능력에 대한 업그레이드 함수
void AAOSCharacterBase::ServerUpgradeAction_Implementation(EActionSlot ActionSlot)
{
	if (::IsValid(ArenaPlayerState) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] ArenaPlayerState is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (ArenaPlayerState->GetUpgradePoints() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Not enough Action points."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (::IsValid(ActionStatComponent) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] ActionStatComponentis is not valid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	FActiveActionState ActiveActionState = ActionStatComponent->GetActiveActionState(ActionSlot);

	// 능력이 업그레이드 가능한지 확인 및 레벨 체크
	if (!ActiveActionState.bIsUpgradable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] %s Action cannot be upgraded or is already at the maximum level."), ANSI_TO_TCHAR(__FUNCTION__), *ActiveActionState.Name.ToString());
		return;
	}

	// 능력 레벨 업그레이드 및 포인트 감소
	if (ActionStatComponent->InitializeActionAtLevel(ActionSlot, ActiveActionState.CurrentLevel + 1))
	{
		ArenaPlayerState->UseUpgradePoints();
	}

	// UI 업데이트
	if (ArenaPlayerState->GetUpgradePoints() > 0)
	{
		ActionStatComponent->ServerUpdateUpgradableStatus(0, StatComponent->GetCurrentLevel());
	}
	else
	{
		ActionStatComponent->ServerToggleUpgradeStat(false);
	}

	UE_LOG(LogTemp, Log, TEXT("[%s] Action '%s' upgraded from Level %d to Level %d."),
		ANSI_TO_TCHAR(__FUNCTION__),
		*ActiveActionState.Name.ToString(),
		ActiveActionState.CurrentLevel - 1,
		ActiveActionState.CurrentLevel);
}

//==================== Input Handling Functions ====================//

void AAOSCharacterBase::OnCtrlKeyPressed()
{
	HandleCtrlKeyInput(true);
}

void AAOSCharacterBase::OnCtrlKeyReleased()
{
	HandleCtrlKeyInput(false);
}

void AAOSCharacterBase::Recall()
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	UAnimMontage* RecallMontage = GetOrLoadMontage("Recall", *FString::Printf(TEXT("/Game/Paragon%s/Characters/Heroes/%s/Animations/Recall_Montage.Recall_Montage'"), *CharacterName.ToString(), *CharacterName.ToString()));
	if (!RecallMontage)
	{
		return;
	}

	MovementComponent->StopMovementImmediately();
	ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::Recall);
	ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::Move);
	ServerPlayMontage(RecallMontage, 1.0f, NAME_None, true);
}

void AAOSCharacterBase::UseItemSlot_Implementation(int32 SlotIndex)
{
	if (::IsValid(ArenaPlayerState) == false)
	{
		return;
	}

	ArenaPlayerState->UseItem(SlotIndex);
}


void AAOSCharacterBase::ToggleItemShop()
{
	AAOSPlayerController* PlayerController = Cast<AAOSPlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	PlayerController->ToggleItemShopVisibility();
}



/**
 * Ctrl 키 입력 처리 함수
 * @param bPressed Ctrl 키가 눌렸는지 여부
 */
void AAOSCharacterBase::HandleCtrlKeyInput(bool bPressed)
{
	AAOSPlayerController* PlayerController = Cast<AAOSPlayerController>(Controller);
	if (!IsValid(PlayerController))
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (!IsValid(Subsystem))
	{
		return;
	}

	FModifyContextOptions ModifyContextOptions;
	ModifyContextOptions.bForceImmediately = true;
	ModifyContextOptions.bIgnoreAllPressedKeysUntilRelease = false;
	ModifyContextOptions.bNotifyUserSettings = false;

	if (bPressed)
	{
		Subsystem->RemoveMappingContext(PlayerInputMappingContext, ModifyContextOptions);
		Subsystem->AddMappingContext(PlayerCtrlInputMappingContext, 0, ModifyContextOptions);
		bCtrlKeyPressed = true;
	}
	else
	{
		Subsystem->AddMappingContext(PlayerInputMappingContext, 0, ModifyContextOptions);
		Subsystem->RemoveMappingContext(PlayerCtrlInputMappingContext, ModifyContextOptions);
		bCtrlKeyPressed = false;
	}
}

//==================== Widget Functions ====================//

void AAOSCharacterBase::SetWidget(UUserWidgetBase* InUserWidgetBase)
{
	StateBar = Cast<UUW_StateBar>(InUserWidgetBase);

	if (::IsValid(StateBar))
	{
		StateBar->InitializeStateBar(StatComponent);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to initialize StateBar: StateBar is invalid."), ANSI_TO_TCHAR(__FUNCTION__));
	}


	if (!HasAuthority() && IsLocallyControlled())
	{
		if (::IsValid(StateBar))
		{
			StateBar->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AAOSCharacterBase::SetWidgetVisibility(bool Visibility)
{
	Visibility ? StateBar->SetVisibility(ESlateVisibility::Visible) : StateBar->SetVisibility(ESlateVisibility::Hidden);
}

//==================== Sight Check Functions ====================//



/**
 * 상대 팀 플레이어가 화면 내에서 보이는지 확인하고 위젯 가시성을 설정합니다.
 *
 * @details
 * - 상대 팀 플레이어와의 거리를 확인하여 가시 범위 내에 있는지 확인합니다.
 * - 화면 내 좌표를 계산해 보이는지 여부를 판별합니다.
 * - Line Trace를 사용하여 시야가 막혀있는지 확인하고 위젯의 가시성을 업데이트합니다.
 *
 * 주요 처리 단계:
 * 1. 컨트롤러 및 게임 상태(ArenaGameState) 유효성 검사.
 * 2. 상대 팀 플레이어 목록 가져오기.
 * 3. 각 플레이어의 거리, 화면 내 위치, 시야 여부를 확인하여 가시성 설정.
 *
 * @note
 * 이 함수는 클라이언트 컨트롤러에서만 동작하며, 캐릭터의 위젯 가시성을 제어합니다.
 */
void AAOSCharacterBase::CheckOutOfSight()
{
	AAOSPlayerController* PlayerController = Cast<AAOSPlayerController>(GetController());
	if (!::IsValid(PlayerController))
	{
		UE_LOG(LogTemp, Error, TEXT("[AAOSCharacterBase::CheckOutOfSight] PlayerController is invalid."));
		return;
	}

	if (!HasAuthority() && PlayerController != UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		return;
	}

	if (!::IsValid(ArenaGameState))
	{
		UE_LOG(LogTemp, Error, TEXT("[AAOSCharacterBase::CheckOutOfSight] ArenaGameState is invalid."));
		return;
	}

	// 상대 팀 플레이어 목록 가져오기
	const TArray<AAOSCharacterBase*> OpponentTeamPlayers = (TeamSide == ETeamSide::Blue) ? ArenaGameState->GetPlayers(ETeamSide::Red) : ArenaGameState->GetPlayers(ETeamSide::Blue);
	if (OpponentTeamPlayers.Num() == 0)
	{
		return;
	}

	int32 ScreenWidth = 0, ScreenHeight = 0;
	PlayerController->GetViewportSize(ScreenWidth, ScreenHeight);

	// 상대 팀 플레이어 처리
	for (auto& Character : OpponentTeamPlayers)
	{
		if (!::IsValid(Character))
		{
			UE_LOG(LogTemp, Warning, TEXT("[AAOSCharacterBase::CheckOutOfSight] Character is invalid."));
			continue;
		}

		float Distance = FVector::Distance(GetActorLocation(), Character->GetActorLocation());
		if (Distance > 2000.f)
		{
			Character->SetWidgetVisibility(false);
			continue;
		}

		// 화면 내 위치 확인
		FVector2D ScreenLocation;
		if (!PlayerController->ProjectWorldLocationToScreen(Character->GetActorLocation(), ScreenLocation))
		{
			Character->SetWidgetVisibility(false);
			continue;
		}

		int32 ScreenX = static_cast<int32>(ScreenLocation.X);
		int32 ScreenY = static_cast<int32>(ScreenLocation.Y);

		if (!(ScreenX > 0 && ScreenY > 0 && ScreenX < ScreenWidth && ScreenY < ScreenHeight))
		{
			Character->SetWidgetVisibility(false);
			continue;
		}

		// Line Trace로 가시성 확인
		FHitResult HitResult;
		FCollisionQueryParams Params(NAME_None, false, this);
		bool bResult = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			GetActorLocation(),
			Character->GetActorLocation(),
			ECC_Visibility,
			Params
		);

		if (bResult && ::IsValid(HitResult.GetActor()))
		{
			if (HitResult.GetActor() == Character)
			{
				Character->SetWidgetVisibility(true);
			}
			else
			{
				Character->SetWidgetVisibility(false);
			}
		}
		else
		{
			Character->SetWidgetVisibility(false);
		}
	}
}



//==================== Particle Functions ====================//



void AAOSCharacterBase::SpawnLevelUpParticle(int32 OldLevel, int32 NewLevel)
{
	if (OldLevel == NewLevel)
	{
		return;
	}

	UParticleSystem* Particle = GetOrLoadSharedParticle("LevelUp", TEXT("/Game/ParagonMinions/FX/Particles/SharedGameplay/States/LevelUp/P_LevelUp.P_LevelUp"));
	if (Particle)
	{
		LastCharacterLocation = GetActorLocation() - FVector(0, 0, 75.f);
		SpawnEmitterAtLocation(Particle, FTransform(FRotator(1), LastCharacterLocation, FVector(1)));
	}
}



//==================== Character State Functions ====================//

void AAOSCharacterBase::UpdateCharacterState_Server_Implementation(ECharacterState InCharacterState)
{
	CharacterState = InCharacterState;
}

void AAOSCharacterBase::UpdateInputValue_Server_Implementation(const float& InForwardInputValue, const float& InRightInputValue)
{
	ForwardInputValue = InForwardInputValue;
	RightInputValue = InRightInputValue;
}

void AAOSCharacterBase::UpdateAimValue_Server_Implementation(const float& InAimPitchValue, const float& InAimYawValue)
{
	CurrentAimYaw = InAimYawValue;
	CurrentAimPitch = InAimPitchValue;
}

void AAOSCharacterBase::ServerUpdateTargetLocation_Implementation(EActionSlot ActionSlot, const FVector& InTargetLocation)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: Not authorized to update target location."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	if (!TargetLocations.Contains(ActionSlot))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid ActionSlot: %d"), ANSI_TO_TCHAR(__FUNCTION__), static_cast<int32>(ActionSlot));
		return;
	}

	TargetLocations[ActionSlot] = InTargetLocation;
	SetDataStatus(ActionSlot, EDataStatus::Position);
}

void AAOSCharacterBase::ServerUpdateTarget_Implementation(EActionSlot ActionSlot, AActor* InTarget)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: Not authorized to update target actor."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	CurrentTarget = ::IsValid(InTarget) ? InTarget : nullptr;
	SetDataStatus(ActionSlot, EDataStatus::Target);
}


void AAOSCharacterBase::ServerUpdateCameraLocation_Implementation(const FVector& InCameraLocation)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: Not authorized to update target actor."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	CurrentCameraLocation = InCameraLocation;
}

//==================== HP/MP Regeneration Functions ====================//

void AAOSCharacterBase::ActivateHealthRegenTimer()
{
	if (HasAuthority() == false)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to retrieve the World context."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	auto HealthRegenLambda = [this]()
		{
			if (::IsValid(StatComponent) == false)
			{
				UE_LOG(LogTemp, Error, TEXT("[%s] StatComponent is invalid."), ANSI_TO_TCHAR(__FUNCTION__));
				return;
			}

			float RegenerationAmount = StatComponent->GetHealthRegeneration();
			StatComponent->ModifyCurrentHP(RegenerationAmount);
		};

	World->GetTimerManager().SetTimer(HealthRegenTimer, HealthRegenLambda, 1.0f, true, 1.0f);
}

void AAOSCharacterBase::ActivateManaRegenTimer()
{
	if (HasAuthority() == false)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to retrieve the World context."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	auto ManaRegenLambda = [this]()
		{
			if (::IsValid(StatComponent) == false)
			{
				UE_LOG(LogTemp, Error, TEXT("[%s] StatComponent is invalid."), ANSI_TO_TCHAR(__FUNCTION__));
				return;
			}

			float RegenerationAmount = StatComponent->GetManaRegeneration();
			StatComponent->ModifyCurrentMP(RegenerationAmount);
		};

	World->GetTimerManager().SetTimer(ManaRegenTimer, ManaRegenLambda, 1.0f, true, 1.0f);
}

void AAOSCharacterBase::DeactivateHealthRegenTimer()
{
	if (HasAuthority() == false)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to retrieve the World context."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	World->GetTimerManager().ClearTimer(HealthRegenTimer);
}

void AAOSCharacterBase::DeactivateManaRegenTimer()
{
	if (HasAuthority() == false)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to retrieve the World context."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	World->GetTimerManager().ClearTimer(ManaRegenTimer);
}

//==================== Character Attribute Functions ====================//

void AAOSCharacterBase::DecreaseHP_Server_Implementation()
{
	StatComponent->ModifyCurrentHP(-100.f);
}

void AAOSCharacterBase::DecreaseMP_Server_Implementation()
{
	StatComponent->ModifyCurrentMP(-100.f);
}

void AAOSCharacterBase::IncreaseEXP_Server_Implementation()
{
	StatComponent->ModifyCurrentEXP(20);
}

void AAOSCharacterBase::IncreaseLevel_Server_Implementation()
{
	StatComponent->SetCurrentLevel(StatComponent->GetCurrentLevel() + 1);
}

void AAOSCharacterBase::IncreaseCriticalChance_Server_Implementation()
{
	StatComponent->ModifyAccumulatedFlatCriticalChance(5);
}

void AAOSCharacterBase::IncreaseAttackSpeed_Server_Implementation()
{
	StatComponent->ModifyAccumulatedPercentAttackSpeed(50.f);
}

void AAOSCharacterBase::ChangeTeamSide_Server_Implementation(ETeamSide InTeamSide)
{
	TeamSide = InTeamSide;

	if (::IsValid(ArenaGameState))
	{
		ArenaGameState->RemovePlayerCharacter(this);
		ArenaGameState->AddPlayerCharacter(this, TeamSide);
	}
}


/**
 * 캐릭터의 사망 처리를 수행합니다.
 *
 * @details
 * - 사망 전 이벤트를 브로드캐스트하고, 사망 진행 여부를 확인합니다.
 * - 게임 모드에 리스폰 요청을 보내고, 사망과 관련된 모든 델리게이트를 해제합니다.
 * - 체력 및 마나 재생 타이머를 비활성화하고, 캐릭터의 상태를 Death로 설정합니다.
 * - 후처리(Post Process) 효과를 활성화하며, 사망 애니메이션과 이펙트를 실행합니다.
 *
 * 주요 처리 단계:
 * 1. 사망 전 이벤트 브로드캐스트 및 리스폰 요청.
 * 2. 사망 관련 델리게이트 해제.
 * 3. 사망 후 처리(충돌 비활성화, 애니메이션 및 파티클 실행).
 *
 * @note
 * 이 함수는 캐릭터가 사망할 때 호출되며, 관련 리소스를 정리하고 사망 효과를 처리합니다.
 */
void AAOSCharacterBase::OnCharacterDeath()
{
	bool bIsDeathInProgress = true;
	if (OnPreDeathEvent.IsBound())
	{
		OnPreDeathEvent.Broadcast(bIsDeathInProgress);
	}

	if (!bIsDeathInProgress)
	{
		return;
	}

	int32 PlayerIndex = 0;
	if (::IsValid(ArenaPlayerState))
	{
		PlayerIndex = ArenaPlayerState->GetPlayerIndex();
	}

	if (AArenaGameMode* GM = Cast<AArenaGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GM->RequestRespawn(PlayerIndex);
	}

	if (StatComponent->OnOutOfCurrentHP.IsAlreadyBound(this, &ThisClass::OnCharacterDeath))
	{
		StatComponent->OnOutOfCurrentHP.RemoveDynamic(this, &ThisClass::OnCharacterDeath);
	}

	if (StatComponent->OnHealthDepleted.IsAlreadyBound(this, &AAOSCharacterBase::ActivateHealthRegenTimer))
	{
		StatComponent->OnHealthDepleted.RemoveDynamic(this, &AAOSCharacterBase::ActivateHealthRegenTimer);
	}

	if (StatComponent->OnManaDepleted.IsAlreadyBound(this, &AAOSCharacterBase::ActivateManaRegenTimer))
	{
		StatComponent->OnManaDepleted.RemoveDynamic(this, &AAOSCharacterBase::ActivateManaRegenTimer);
	}

	DeactivateHealthRegenTimer();
	DeactivateManaRegenTimer();

	EnumAddFlags(CharacterState, ECharacterState::Death);

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

	SetActorTickEnabled(false);
	SetActorHiddenInGame(true);
	ActivatePostProcessEffect_Client();

	UAnimMontage* Montage = GetOrLoadMontage("Death", TEXT(""));

	UParticleSystem* DeathParticle = GetOrLoadSharedParticle(TEXT("Death"), TEXT("/Game/ParagonMinions/FX/Particles/SharedGameplay/States/Death/FX/P_Death_Buff.P_Death_Buff"));
	if (DeathParticle) SpawnEmitterAtLocation(DeathParticle, FTransform(FRotator(0), GetActorLocation(), FVector(1)));

	if (OnPostDeathEvent.IsBound())
	{
		OnPostDeathEvent.Broadcast(this, LastHitCharacter);
	}
}




/**
 * 클라이언트에서 캐릭터가 사망하였을 때 후처리(Post Process) 효과를 활성화합니다.
 *
 * @details
 * - PostProcessVolume의 ColorSaturation과 ColorGamma 값을 조정하여 화면 색상을 변경합니다.
 * - PostProcessVolume이 이미 캐싱되어 있다면 설정을 직접 적용하고, 없는 경우 월드에서 검색하여 활성화합니다.
 *
 * 주요 처리 단계:
 * 1. PostProcessVolume이 유효한지 확인 후, ColorSaturation 및 ColorGamma 설정.
 * 2. 유효하지 않을 경우 월드 내에서 PostProcessVolume 검색.
 * 3. Unbound 속성을 가진 PostProcessVolume을 캐싱하고, 재귀적으로 효과를 활성화.
 *
 * @note
 * 이 함수는 클라이언트 전용이며, 특정 시각적 효과를 화면에 적용합니다.
 */

void AAOSCharacterBase::ActivatePostProcessEffect_Client_Implementation()
{
	if (PostProcessVolume)
	{
		// ColorSaturation과 ColorGamma 값을 조절합니다.
		PostProcessVolume->Settings.bOverride_ColorSaturation = true;
		PostProcessVolume->Settings.ColorSaturation = FVector4(0, 0, 0, 1); // 흑백으로 설정

		PostProcessVolume->Settings.bOverride_ColorGamma = true;
		PostProcessVolume->Settings.ColorGamma = FVector4(1.0f, 1.0f, 1.0f, 0.8f); // Gamma 값을 조절
	}
	else
	{
		// 월드 내 PostProcessVolume을 찾습니다.
		for (TActorIterator<APostProcessVolume> It(GetWorld()); It; ++It)
		{
			if (It->bUnbound)
			{
				PostProcessVolume = *It;
				ActivatePostProcessEffect_Client();
				break;
			}
		}
	}
}

void AAOSCharacterBase::DeActivatePostProcessEffect_Client_Implementation()
{
	if (PostProcessVolume)
	{
		// ColorSaturation과 ColorGamma 값을 조절합니다.
		PostProcessVolume->Settings.bOverride_ColorSaturation = true;
		PostProcessVolume->Settings.ColorSaturation = FVector4(1, 1, 1, 1);

		PostProcessVolume->Settings.bOverride_ColorGamma = true;
		PostProcessVolume->Settings.ColorGamma = FVector4(0.75f, 0.75f, 0.75f, 1.0f);

		PostProcessVolume->Settings.bOverride_ColorGain = true;
		PostProcessVolume->Settings.ColorGain = FVector4(0.78f, 0.78f, 0.78f, 1.0f);
	}

	while (!DamageWidgetQueue.IsEmpty())
	{
		UWidgetComponent* DamageWidgetComponent = nullptr;

		// 큐에서 위젯 컴포넌트를 꺼내 제거
		if (DamageWidgetQueue.Dequeue(DamageWidgetComponent))
		{
			if (DamageWidgetComponent)
			{
				DamageWidgetComponent->DestroyComponent();
			}
		}
	}

	DamageWidgetQueue.Empty();
}



/**
 * 캐릭터를 리스폰 상태로 전환합니다.
 *
 * @details
 * - 사망 상태 플래그를 제거하고, 충돌 및 이동 모드를 복원합니다.
 * - 숨겨진 캐릭터를 다시 보이게 하고, Tick을 활성화합니다.
 * - 사망 및 재생 관련 델리게이트를 다시 바인딩합니다.
 * - 체력 및 마나를 최대치로 설정합니다.
 * - 사망 시 적용된 후처리(Post Process) 효과를 비활성화합니다.
 *
 * @note
 * 이 함수는 캐릭터가 리스폰될 때 호출되며, 게임 플레이 재개를 준비합니다.
 */

void AAOSCharacterBase::Respawn(const float RestoreRatio)
{
	EnumRemoveFlags(CharacterState, ECharacterState::Death);

	GetMesh()->SetCollisionProfileName(FName("CharacterMesh"));
	GetCapsuleComponent()->SetCollisionProfileName(FName("AOSCharacterBase"));
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);

	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);

	if (StatComponent->OnOutOfCurrentHP.IsAlreadyBound(this, &ThisClass::OnCharacterDeath) == false)
	{
		StatComponent->OnOutOfCurrentHP.AddDynamic(this, &ThisClass::OnCharacterDeath);
	}

	if (StatComponent->OnHealthDepleted.IsAlreadyBound(this, &AAOSCharacterBase::ActivateHealthRegenTimer) == false)
	{
		StatComponent->OnHealthDepleted.AddDynamic(this, &AAOSCharacterBase::ActivateHealthRegenTimer);
	}

	if (StatComponent->OnManaDepleted.IsAlreadyBound(this, &AAOSCharacterBase::ActivateManaRegenTimer) == false)
	{
		StatComponent->OnManaDepleted.AddDynamic(this, &AAOSCharacterBase::ActivateManaRegenTimer);
	}

	const float RestoreHP = StatComponent->GetMaxHP() * RestoreRatio;
	const float RestoreMP = StatComponent->GetMaxMP() * RestoreRatio;

	StatComponent->SetCurrentHP(RestoreHP);
	StatComponent->SetCurrentMP(RestoreMP);

	DeActivatePostProcessEffect_Client();
}

//==================== Character Control Functions ====================//

void AAOSCharacterBase::MontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("MontageEnded called with null Montage"));
		return;
	}

	if (HasAuthority())
	{
		if (OnSwitchActionStateEnded.IsBound())
		{
			OnSwitchActionStateEnded.Broadcast();
		}

		if (OnRootedStateEnded.IsBound())
		{
			OnRootedStateEnded.Broadcast();
		}
	}
}

void AAOSCharacterBase::EnableCharacterMove()
{
	if (HasAuthority() && OnRootedStateEnded.IsBound())
	{
		OnRootedStateEnded.Broadcast();
	}
}

void AAOSCharacterBase::EnableSwitchAction()
{
	if (HasAuthority() && OnSwitchActionStateEnded.IsBound())
	{
		OnSwitchActionStateEnded.Broadcast();
	}
}


void AAOSCharacterBase::RestoreRootedState()
{
	OnRootedStateEnded.RemoveAll(this);

	ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::Rooted);
}

void AAOSCharacterBase::RestoreSwitchActionState()
{
	OnSwitchActionStateEnded.RemoveAll(this);

	ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::SwitchAction);
}




void AAOSCharacterBase::EnableUseControllerRotationYaw()
{
	bUseControllerRotationYaw = true;
}

void AAOSCharacterBase::EnableGravity()
{
	if (HasAuthority() == false)
	{
		return;
	}

	UCharacterMovementComponent* CharacterMovementComp = GetCharacterMovement();
	if (!CharacterMovementComp)
	{
		return;
	}

	CharacterMovementComp->GravityScale = 1.0;
}

void AAOSCharacterBase::DisableGravity()
{
	if (HasAuthority() == false)
	{
		return;
	}

	UCharacterMovementComponent* CharacterMovementComp = GetCharacterMovement();
	if (!CharacterMovementComp)
	{
		return;
	}

	CharacterMovementComp->GravityScale = 0;
}

void AAOSCharacterBase::DisableJump()
{
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->RemoveActionBinding("JumpAction", EInputEvent::IE_Pressed);
	}
}

void AAOSCharacterBase::EnableJump()
{
	if (EnhancedInputComponent)
	{
		EnhancedInputComponent->BindAction(PlayerCharacterInputConfigData->JumpAction, ETriggerEvent::Started, this, &AAOSCharacterBase::Jump);
	}
}

void AAOSCharacterBase::ClientEnableInput_Implementation()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC && PC->IsLocalController())
	{
		EnableInput(PC);
	}
}

void AAOSCharacterBase::ClientDisableInput_Implementation()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC && PC->IsLocalController())
	{
		DisableInput(PC);
	}
}

//==================== Utility Functions ====================//

const FName AAOSCharacterBase::GetAttackMontageSection(const int32& Section)
{
	return FName(*FString::Printf(TEXT("Attack%d"), Section));
}


void AAOSCharacterBase::SaveCharacterTransform()
{
	// 현재 캐릭터의 위치, 회전, 방향 벡터를 저장
	LastCharacterLocation = GetActorLocation() - FVector(0, 0, 95.f);
	LastCharacterRotation = GetActorRotation();
	LastForwardVector = GetActorForwardVector();
	LastRightVector = GetActorRightVector();
	LastUpVector = GetActorUpVector();
}


/**
 * 카메라의 전방 벡터와 추적 범위를 기반으로 충돌 지점을 계산합니다.
 * 이 함수는 카메라 위치에서 추적 범위로 정의된 지점까지 라인 트레이스를 수행합니다.
 * 만약 라인 트레이스가 물체와 충돌하면, 충돌 지점을 충돌 위치로 업데이트합니다.
 *
 * @param TraceRange 추적이 충돌을 확인해야 하는 최대 거리입니다.
 * @return 충돌 여부와 계산된 충돌 지점을 포함한 FImpactResult 구조체입니다.
 */
FHitResult AAOSCharacterBase::GetImpactPoint(const float TraceRange)
{
	FHitResult HitResult;

	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (::IsValid(CameraManager) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraManager is null."));
		return HitResult;
	}

	FVector CameraLocation = CameraManager->GetCameraLocation();
	FVector EndPoint = CameraLocation + CameraManager->GetActorForwardVector() * (TraceRange > 0 ? TraceRange : 10000.f);

	FCollisionQueryParams params(NAME_None, false, this);
	bool bResult = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		CameraLocation,
		EndPoint,
		ECollisionChannel::ECC_Visibility,
		params
	);

	HitResult.Location = bResult ? HitResult.Location : EndPoint;
	return HitResult;
}

FHitResult AAOSCharacterBase::GetSweepImpactPoint(const float TraceRange)
{
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);

	FHitResult HitResult;

	if (!IsValid(CameraManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("CameraManager is null."));
		return HitResult;
	}

	FVector CameraLocation = CameraManager->GetCameraLocation();
	FVector EndPoint = CameraLocation + CameraManager->GetActorForwardVector() * (TraceRange > 0 ? TraceRange : 1000.f);

	FCollisionQueryParams Params(NAME_None, false, this);
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(50.0f);  // 설정 가능한 반지름

	bool bResult = GetWorld()->SweepSingleByChannel(
		HitResult,
		CameraLocation,
		EndPoint,
		FQuat::Identity,
		ECollisionChannel::ECC_Visibility,
		CollisionShape,
		Params
	);


	HitResult.Location = bResult ? HitResult.Location : EndPoint;
	return HitResult;
}




/**
 * Pitch와 Yaw 각도를 기준으로 전방 벡터 방향으로 스윕 트레이스를 수행하여 충돌 지점을 찾습니다.
 *
 * @param TraceRange 최대 스윕 거리(충돌 감지 범위).
 * @return FHitResult 스윕 트레이스의 결과로, 충돌한 액터 및 위치에 대한 정보를 포함합니다.
 *
 * 이 함수는 서버에서 카메라 위치, Pitch, Yaw 값을 사용하여 전방 스윕 트레이스를 수행합니다.
 * 지정한 범위 내에서 충돌 지점을 찾지 못한 경우, 최종 위치를 결과 위치로 반환합니다.
 *
 * 이 함수를 호출하기 전에, `ServerUpdateCameraLocation` RPC 함수를 통해 클라이언트의 최신 카메라 위치를
 * 서버에 업데이트해야 합니다.
 *
 */
FHitResult AAOSCharacterBase::SweepTraceFromAimAngles(const float TraceRange)
{
	FHitResult HitResult;
	FVector StartLocation = CurrentCameraLocation;
	FRotator Rotation(CurrentAimPitch, CurrentAimYaw, 0); // Pitch와 Yaw 각도로 회전 생성
	FVector EndLocation = StartLocation + Rotation.Vector() * TraceRange;

	// 트레이스 매개변수 및 충돌 모양 설정
	FCollisionQueryParams Params(NAME_None, false, this);
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(50.0f);

	bool bResult = GetWorld()->SweepSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		ECollisionChannel::ECC_Visibility,
		CollisionShape,
		Params
	);

	HitResult.Location = bResult ? HitResult.Location : EndLocation;
	return HitResult;
}


void AAOSCharacterBase::UpdateOverlayMaterial()
{
	const FActionAttributes& ActionAttributes = ActionStatComponent->GetActionAttributes(EActionSlot::LMB);
	if (ActionAttributes.Name.IsNone())
	{
		return;
	}

	// 각 클라이언트에서 충돌 지점을 계산합니다.
	FHitResult ImpactResult = GetSweepImpactPoint(ActionAttributes.Range);
	AActor* NewTarget = ImpactResult.GetActor();

	// 이전 타겟의 오버레이 머테리얼을 초기화합니다.
	if (!ImpactResult.bBlockingHit || ::IsValid(NewTarget) == false)
	{
		if (::IsValid(CurrentTarget) == false)
		{
			CurrentTarget = nullptr;
			return;
		}

		UMeshComponent* MeshComponent = Cast<UMeshComponent>(CurrentTarget->GetComponentByClass(UMeshComponent::StaticClass()));
		if (MeshComponent)
		{
			MeshComponent->SetOverlayMaterial(OriginalMaterial);
		}

		CurrentTarget = nullptr;
		return;
	}

	// 충돌한 물체가 환경 요소라면 무시
	ECollisionChannel HitChannel = ImpactResult.GetComponent()->GetCollisionObjectType();
	if (HitChannel == ECC_WorldStatic || HitChannel == ECC_WorldDynamic)
	{
		return;
	}

	// 새로운 타겟이 기존 타겟과 동일하면 추가 변경 필요 없음
	if (CurrentTarget == NewTarget)
	{
		return;
	}

	CurrentTarget = NewTarget;
	UMeshComponent* MeshComponent = Cast<UMeshComponent>(CurrentTarget->GetComponentByClass(UMeshComponent::StaticClass()));

	if (!MeshComponent || !OverlayMaterial_Ally || !OverlayMaterial_Enemy)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Invalid MeshComponent or OverlayMaterial on target: %s"), *GetName(), *CurrentTarget->GetName());
		return;
	}

	// 원래 머테리얼 저장
	OriginalMaterial = MeshComponent->GetOverlayMaterial();

	// 적군 / 아군 판단 후 머테리얼 변경
	if (ACharacterBase* Character = Cast<ACharacterBase>(CurrentTarget))
	{
		UMaterialInterface* NewMaterial = (this->TeamSide != Character->TeamSide) ? OverlayMaterial_Enemy : OverlayMaterial_Ally;

		// 현재 머테리얼과 다를 경우만 변경
		if (MeshComponent->GetOverlayMaterial() != NewMaterial)
		{
			MeshComponent->SetOverlayMaterial(NewMaterial);
		}
	}
}


void AAOSCharacterBase::UpdateCameraPositionWithAim()
{
	if (SpringArmComponent)
	{
		FVector CharacterLocation = GetActorLocation();
		FRotator AimRotation(CurrentAimPitch, CurrentAimYaw, 0.0f);

		// SpringArm 길이를 사용하여 카메라의 위치를 계산
		FVector CameraOffset = AimRotation.Vector() * -SpringArmComponent->TargetArmLength;
		FVector CameraLocation = CharacterLocation + CameraOffset;

		CurrentCameraLocation = CameraLocation;
	}
}


void AAOSCharacterBase::ClientSetControllerRotationYaw_Implementation(bool bEnableYawRotation)
{
	if (HasAuthority())
	{
		return;
	}

	AAOSPlayerController* PlayerController = Cast<AAOSPlayerController>(GetController());
	if (PlayerController && PlayerController == UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		bUseControllerRotationYaw = bEnableYawRotation;
	}
}


void AAOSCharacterBase::DisplayMouseCursor(bool bShowCursor)
{
	if (::IsValid(Controller) == false)
	{
		return;
	}

	AAOSPlayerController* PlayerController = Cast<AAOSPlayerController>(Controller);
	if (PlayerController)
	{
		PlayerController->DisplayMouseCursor(bShowCursor);
	}
}