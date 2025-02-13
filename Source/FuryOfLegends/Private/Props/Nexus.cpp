// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/Nexus.h"
#include "Game/AOSGameInstance.h"
#include "Game/ArenaGameMode.h"
#include "Characters/CharacterBase.h"
#include "Characters/AOSCharacterBase.h"
#include "Characters/MinionBase.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StatComponent.h"
#include "Components/ActionStatComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Camera/CameraComponent.h"
#include "Controllers/AOSPlayerController.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Props/Projectile.h"
#include "Structs/CharacterResources.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "UI/TargetStatusWidget.h"


ANexus::ANexus()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;;

	DefaultRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRootComponent"));
	RootComponent = DefaultRootComponent;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SceneComponent->SetupAttachment(RootComponent);
	SceneComponent->SetRelativeLocation(FVector(0, 0, 700));
	SceneComponent->SetRelativeScale3D(FVector(5));

	SphereGrid_A = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereGrid_A"));
	SphereGrid_A->SetupAttachment(SceneComponent);
	SphereGrid_A->SetRelativeLocationAndRotation(FVector(0), FRotator(0, 0, 44));
	SphereGrid_A->SetRelativeScale3D(FVector(1));
		
	SphereGrid_B = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereGrid_B"));
	SphereGrid_B->SetupAttachment(SceneComponent);
	SphereGrid_A->SetRelativeLocationAndRotation(FVector(0), FRotator(-90, 0, 0));
	SphereGrid_B->SetRelativeScale3D(FVector(0.900024f));

	SphereHalo_A = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereHalo_A"));
	SphereHalo_A->SetupAttachment(SceneComponent);
	SphereHalo_A->SetRelativeLocation(FVector(0, 0, 0));
	SphereHalo_A->SetRelativeScale3D(FVector(1.198052f));

	Ring_A = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ring_A"));
	Ring_A->SetupAttachment(SceneComponent);
	Ring_A->SetRelativeLocationAndRotation(FVector(-5, 0, 0), FRotator(0, 0, 0));
	Ring_A->SetRelativeScale3D(FVector(0.1f, 1, 1));
	Ring_A->SetCollisionProfileName(FName(TEXT("NoCollision")));

	Refraction_A = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Refracction_A"));
	Refraction_A->SetupAttachment(Ring_A);
	Refraction_A->SetRelativeLocationAndRotation(FVector(0, 0, 0), FRotator(-90.0f, 0.0f, 0.0f));
	Refraction_A->SetRelativeScale3D(FVector(20.171476f, 2.017147f, 0));
	Refraction_A->SetCollisionProfileName(FName(TEXT("NoCollision")));

	Ring_B = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ring_B"));
	Ring_B->SetupAttachment(SceneComponent);
	Ring_B->SetRelativeLocationAndRotation(FVector(0, 5, 0), FRotator(90.0f, 0, 90.0f));
	Ring_B->SetRelativeScale3D(FVector(0.095103f, 0.830134f, 0.830134f));
	Ring_B->SetCollisionProfileName(FName(TEXT("NoCollision")));

	Refraction_B = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Refracction_B"));
	Refraction_B->SetupAttachment(Ring_B);
	Refraction_B->SetRelativeLocationAndRotation(FVector(0), FRotator(-90.0f, 0.0f, 0.0f));
	Refraction_B->SetRelativeScale3D(FVector(20.171478f, 2.017147f, 1));
	Refraction_B->SetCollisionProfileName(FName(TEXT("NoCollision")));


	NexusCenter = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NexusCenter"));
	NexusCenter->SetupAttachment(RootComponent);

	StatComponent = CreateDefaultSubobject<UStatComponent>(TEXT("StatComponent"));
	ActionStatComponent = CreateDefaultSubobject<UActionStatComponent>(TEXT("ActionStatComponent"));
	StatComponent->SetIsReplicated(true);
	ActionStatComponent->SetIsReplicated(true);

	PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
	PointLight->SetupAttachment(RootComponent);
	PointLight->SetIntensity(24000.f);
	PointLight->SetAttenuationRadius(1850.f);
	PointLight->SetLightColor(FLinearColor(1, 0.498f, 0.1725f));
	PointLight->bAffectsWorld = true;

	ProtalParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("PortalEffect"));
	ProtalParticleSystem->SetupAttachment(RootComponent);
	ProtalParticleSystem->SetAutoActivate(true);

	TargetBeamParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TargetBeam"));
	TargetBeamParticleSystem->SetupAttachment(RootComponent);
	TargetBeamParticleSystem->SetRelativeLocation(FVector(0, 0, 700));
	TargetBeamParticleSystem->SetAutoActivate(false);

	DetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DetectionBox"));
	DetectionBox->SetupAttachment(RootComponent);
	DetectionBox->SetRelativeLocation(FVector(0, 0, 700));
	DetectionBox->SetBoxExtent(FVector(1000, 1000, 1000));
	DetectionBox->SetCollisionProfileName(FName(TEXT("NoCollision")));

	UCapsuleComponent* CapsuleCollision = GetCapsuleComponent();
	if (CapsuleCollision)
	{
		CapsuleCollision->SetupAttachment(RootComponent);
		CapsuleCollision->SetRelativeLocation(FVector(0, 0, 700));
		CapsuleCollision->InitCapsuleSize(500.f, 800.f);
		CapsuleCollision->SetCollisionProfileName(FName(TEXT("NoCollision")));
	}
	
	NexusCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("NexusCamera"));
	NexusCamera->SetupAttachment(RootComponent);
	NexusCamera->SetRelativeLocationAndRotation(FVector(-2000, -2000, 400), FRotator(0.0f, 45.0f, 0.0f));
	NexusCamera->bAutoActivate = false;

	HomingTargetSceneComponent->SetRelativeLocation(FVector(0, 0, -600));

	static ConstructorHelpers::FObjectFinder<UDataTable> NEXUS_DATATABLE(TEXT("/Game/FuryOfLegends/DataTables/Game/DT_NexusDataTable.DT_NexusDataTable"));
	if (NEXUS_DATATABLE.Succeeded())
	{
		NexusDataTable = NEXUS_DATATABLE.Object;

		const FNexusDataRow* NexusDataRow = NexusDataTable->FindRow<FNexusDataRow>(FName(TEXT("1")), TEXT(""));
		if (!NexusDataRow)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] LoadResources: Failed to find NexusDataRow in NexusDataTable."), ANSI_TO_TCHAR(__FUNCTION__));
			return;
		}

		StatTable = NexusDataRow->StatTable;
		AbilityStatTable = NexusDataRow->AbilityStatTable;
		ResourcesTable = NexusDataRow->ResourcesTable;
	}

	if (!StatTable || !AbilityStatTable || !ResourcesTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] LoadResources: One or more required tables are missing."), ANSI_TO_TCHAR(__FUNCTION__));
	}

	TargetingTimerHandle = FTimerHandle();
	TargetCharacter = nullptr;
	TeamSide = ETeamSide::None;
	Count = 0;
	TargetingDelay = 0.0f;

	PrimaryRotation = FRotator(66, 0, 48);
	SecondaryRotation = FRotator(30, 8, -66);

	EnemiesInRange.Empty();
	AlliesInRange.Empty();

	CoreEmissionColor = FLinearColor(187.f / 255.f, 43.f / 255.f, 7.f / 255.f);
	CorePrimartColor = FLinearColor(209.f / 255.f, 50.f / 255.f, 35.f / 255.f);
	CoreSecondaryColor = FLinearColor(255.f / 255.f, 118.f / 255.f, 19.f / 255.f);
	RingPrimaryColor = FLinearColor(209.f / 255.f, 50.f / 255.f, 35.f / 255.f);
	RingSecondaryColor = FLinearColor(255.f / 255.f, 118.f / 255.f, 19.f / 255.f);
	CenterPrimaryColor = FLinearColor(255.f / 255.f, 0.f / 255.f, 0.f / 255.f);

	LoadGameplayAssets();
}


void ANexus::LoadGameplayAssets()
{
	if (::IsValid(ResourcesTable) == false)
	{
		return;
	}

	const FCharacterGamePlayDataRow* DataRow = ResourcesTable->FindRow<FCharacterGamePlayDataRow>(FName(*FString::FromInt(1)), TEXT(""));
	if (!DataRow)
	{
		return;
	}

	// 비어 있는 경우에만 초기화
	if (GameplayMontages.Num() == 0)
	{
		GameplayMontages = DataRow->GetGamePlayMontagesMap();
	}

	if (GameplayParticles.Num() == 0)
	{
		GameplayParticles = DataRow->GetGamePlayParticlesMap();
	}

	if (GameplayMeshes.Num() == 0)
	{
		GameplayMeshes = DataRow->GetGamePlayMeshesMap();
	}

	if (GameplayMaterials.Num() == 0)
	{
		GameplayMaterials = DataRow->GetGamePlayMaterialsMap();
	}

	if (GameplayClasses.Num() == 0)
	{
		GameplayClasses = DataRow->GetGamePlayClassesMap();
	}
}

void ANexus::SetMeshesFromResource()
{
	UStaticMesh* SphereMesh = LoadOrGetResourceWithStaticLoad<UStaticMesh>(GameplayMeshes, FName("Sphere"), TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh)
	{
		SphereGrid_A->SetStaticMesh(SphereMesh);
		SphereGrid_B->SetStaticMesh(SphereMesh);
		SphereHalo_A->SetStaticMesh(SphereMesh);
	}

	UStaticMesh* SmallRingMesh = LoadOrGetResourceWithStaticLoad<UStaticMesh>(GameplayMeshes, FName("SmallRing"), TEXT("/Game/SuperGrid/Meshes/Utility_SmallRing.Utility_SmallRing"));
	if (SmallRingMesh)
	{
		Ring_A->SetStaticMesh(SmallRingMesh);
	}

	UStaticMesh* CylinderMesh = LoadOrGetResourceWithStaticLoad<UStaticMesh>(GameplayMeshes, FName("Cylinder"), TEXT("/Game/SuperGrid/Meshes/Utility_CylinderTop.Utility_CylinderTop"));
	if (CylinderMesh)
	{
		Refraction_A->SetStaticMesh(CylinderMesh);
	}

	UStaticMesh* NexusCenterMesh = LoadOrGetResourceWithStaticLoad<UStaticMesh>(GameplayMeshes, FName("NexusCenter"), TEXT("/Game/ParagonProps/Monolith/Dusk/Meshes/SternInhibitorCenter.SternInhibitorCenter"));
	if (NexusCenterMesh)
	{
		NexusCenter->SetStaticMesh(NexusCenterMesh);
	}
}

void ANexus::SetAssetsFromResource()
{
	/*UMaterialInstance* HoloA = GetOrLoadMaterial(FName("HoloA"), TEXT("/Game/SuperGrid/Materials/M_Tut_HoloA.M_Tut_HoloA"));
	if (HoloA)
	{
		SphereHalo_A->SetMaterial(0, HoloA);
	}

	UMaterialInstance* HoloB = GetOrLoadMaterial(FName("HoloB"), TEXT("/Game/SuperGrid/Materials/M_Tut_HoloB.M_Tut_HoloB"));
	if (HoloB)
	{
		SphereHalo_B->SetMaterial(0, HoloB);
	}

	UMaterialInstance* LightConeRed = GetOrLoadMaterial(FName("LightConeRed"), TEXT("/Game/SuperGrid/Materials/M_LightCone_Red.M_LightCone_Red"));
	if (LightConeRed)
	{
		SphereHalo_C->SetMaterial(0, LightConeRed);
	}

	UMaterialInstance* HoloD = GetOrLoadMaterial(FName("HoloD"), TEXT("/Game/SuperGrid/Materials/M_Tut_HoloD.M_Tut_HoloD"));
	if (HoloD)
	{
		Ring_A->SetMaterial(0, HoloD);
	}

	UMaterialInstance* Refraction = GetOrLoadMaterial(FName("Refraction"), TEXT("/Game/SuperGrid/Materials/M_Utility_Refract_Inst.M_Utility_Refract_Inst"));
	if (Refraction)
	{
		Refraction_A->SetMaterial(0, Refraction);
	}

	UMaterialInstance* Channel4 = GetOrLoadMaterial(FName("Channel4"), TEXT("/Game/SuperGrid/Materials/M_Utility_Channel_R_4.M_Utility_Channel_R_4"));
	if (Channel4)
	{
		NexusCenter->SetMaterial(0, Channel4);
	}*/

	UParticleSystem* PortalEffect = GetOrLoadParticle(FName("Portal"), TEXT("/Game/SuperGrid/Particles/P_Portal.P_Portal"));
	if (PortalEffect)
	{
		ProtalParticleSystem->Template = PortalEffect;
	}

	UParticleSystem* BeamEffect = GetOrLoadParticle(FName("TargetBeam"), TEXT("/Game/ParagonProps/FX/Particles/Core/P_SingleTargetCore_TargetBeam.P_SingleTargetCore_TargetBeam"));
	if (PortalEffect)
	{
		TargetBeamParticleSystem->Template = BeamEffect;
	}
}


void ANexus::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Ring_A)
	{
		//Ring_A->AddLocalRotation(RotationSpeed * DeltaTime);
		ApplyDynamicRotation(DeltaTime);
	}

	if (::IsValid(TargetCharacter))
	{
		FVector TargetLocation = TargetCharacter->GetActorLocation();
		TargetBeamParticleSystem->SetVectorParameter(FName("BeamEnd"), TargetLocation);
	}
}

void ANexus::ApplyDynamicRotation(float DeltaTime)
{
	Ring_A->AddLocalRotation(PrimaryRotation * DeltaTime);
	Ring_B->AddLocalRotation(SecondaryRotation * DeltaTime);
}


void ANexus::PostInitializeComponents()	
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		if (!StatComponent)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] StatComponent is null."), ANSI_TO_TCHAR(__FUNCTION__));
			return;
		}

		if (!ActionStatComponent)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] ActionStatComponent is null."), ANSI_TO_TCHAR(__FUNCTION__));
			return;
		}

		if (StatTable)
		{
			StatComponent->InitStatComponent(StatTable);

			StatComponent->OnOutOfCurrentHP.AddDynamic(this, &ThisClass::OnCharacterDeath);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] StatTable is null."), ANSI_TO_TCHAR(__FUNCTION__));
		}

		if (AbilityStatTable)
		{
			ActionStatComponent->InitActionStatComponent(AbilityStatTable, StatComponent);
			ActionStatComponent->InitializeActionAtLevel(EActionSlot::LMB, 1);

			ActionStatComponent->OnCooldownTimeChanged.AddDynamic(this, &ANexus::OnCooldownTimeChanged);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] AbilityStatTable is null."), ANSI_TO_TCHAR(__FUNCTION__));
		}
	}
}

void ANexus::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, TargetCharacter, COND_None);
}

void ANexus::BeginPlay()
{
	Super::BeginPlay();

	if (ActionStatComponent)
	{
		const FActionAttributes& Stats = ActionStatComponent->GetActionAttributes(EActionSlot::LMB);
		DetectionBox->SetBoxExtent(FVector(Stats.Range));
	}

	UCapsuleComponent* CapsuleCollision = GetCapsuleComponent();
	if (CapsuleCollision)
	{
		CapsuleCollision->SetCollisionProfileName(FName("Object"));
		CapsuleCollision->SetGenerateOverlapEvents(true);
	}

	if (HasAuthority())
	{
		DetectionBox->SetCollisionProfileName(FName("Trigger"));
		DetectionBox->SetGenerateOverlapEvents(true);
		DetectionBox->OnComponentBeginOverlap.AddDynamic(this, &ANexus::OnCharacterEnterRange);
		DetectionBox->OnComponentEndOverlap.AddDynamic(this, &ANexus::OnCharacterExitRange);

		const FNexusDataRow* NexusDataRow = NexusDataTable->FindRow<FNexusDataRow>(FName(TEXT("1")), TEXT(""));
		if (!NexusDataRow)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] LoadResources: Failed to find NexusDataRow in NexusDataTable."), ANSI_TO_TCHAR(__FUNCTION__));
			return;
		}

		TargetingDelay = NexusDataRow->TargetingDelay;
	}

	SetAssetsFromResource();
}







/**
 * ANexus::OnCharacterEnterRange - 사거리 내에 캐릭터가 진입했을 때 호출되는 함수입니다.
 * - 캐릭터가 아군인지 적군인지 확인하고, 각각 AlliesInRange 또는 EnemiesInRange 목록에 추가합니다.
 * - 아군이 플레이어일 경우, 공격받았을 때 타워가 반응하도록 OnPostReceiveDamageEvent 델리게이트를 바인딩합니다.
 * - 첫 번째 적군이 범위에 진입했을 때 LMB 공격을 시작하여 타겟팅 효과를 활성화합니다.
 *
 *  주요 기능:
 *  1. 진입한 캐릭터 유효성 및 소유자 여부 확인
 *  2. 아군 또는 적군에 따라 범위 내 목록에 추가
 *  3. 첫 번째 적 캐릭터가 진입 시 타워가 공격을 시작하도록 설정
 *
 *  @param OverlappedComponent 범위를 담당하는 충돌 컴포넌트
 *  @param OtherActor 진입한 캐릭터
 *  @param OtherComp 진입한 캐릭터의 충돌 컴포넌트
 *  @param OtherBodyIndex 충돌 바디 인덱스
 *  @param bFromSweep 진입이 스윕에 의한 것인지 여부
 *  @param SweepResult 스윕 결과 히트 정보
 */

void ANexus::OnCharacterEnterRange(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == Owner)
	{
		return;
	}

	//UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Character Entered: %s"), *OtherActor->GetName()), true, true, FLinearColor::Green, 2.0f);

	ACharacterBase* Character = Cast<ACharacterBase>(OtherActor);
	if (::IsValid(Character) == false)
	{
		return;
	}

	if (Character->TeamSide == TeamSide)
	{
		AlliesInRange.Add(Character);

		if (EnumHasAnyFlags(Character->ObjectType, EObjectType::Player))
		{
			Character->OnPostReceiveDamageEvent.AddDynamic(this, &ANexus::OnAllyAttacked);
		}
		return;
	}
	else
	{
		EnemiesInRange.Add(Character);
	}

	BindNexusInfoToHUD(Character);

	if (EnemiesInRange.Num() == 1)
	{
		TargetCharacter = Character;
		LMB_Started();
	}
}



/**
 * ANexus::OnCharacterExitRange - 사거리에서 캐릭터가 벗어났을 때 호출되는 함수입니다.
 * - 벗어난 캐릭터가 아군인지 적군인지 확인하여 AlliesInRange 또는 EnemiesInRange 목록에서 제거합니다.
 * - 아군 플레이어인 경우 데미지 이벤트 바인딩을 해제하고, 타겟 캐릭터가 범위를 벗어났을 경우 새로운 타겟을 설정합니다.
 * - 타겟 캐릭터가 사거리 밖으로 나가면 타겟팅 효과를 비활성화하고 타이머를 정지합니다.
 *
 *  주요 기능:
 *  1. 진입한 캐릭터 유효성 확인 및 팀 사이드에 따른 처리
 *  2. 타겟 캐릭터가 범위를 벗어났을 때 타겟팅 효과 비활성화 및 타이머 정지
 *  3. 새로운 타겟을 선택하여 타워의 공격을 이어나감
 *
 *  @param OverlappedComp 범위 콜리전 컴포넌트
 *  @param OtherActor 범위를 벗어난 캐릭터
 *  @param OtherComp 캐릭터의 충돌 컴포넌트
 *  @param OtherBodyIndex 충돌 바디 인덱스
 */

void ANexus::OnCharacterExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == Owner)
	{
		return;
	}

	ACharacterBase* Character = Cast<ACharacterBase>(OtherActor);
	if (!::IsValid(Character))
	{
		return;
	}

	//UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Character Exited: %s"), *Character->GetName()), true, true, FLinearColor::Red, 2.0f);

	// 범위를 벗어난 캐릭터 제거
	AlliesInRange.Remove(Character);
	EnemiesInRange.Remove(Character);

	// 플레이어의 데미지 이벤트 제거
	if (EnumHasAnyFlags(Character->ObjectType, EObjectType::Player))
	{
		if (Character->TeamSide == TeamSide)
		{
			Character->OnPostReceiveDamageEvent.RemoveDynamic(this, &ANexus::OnAllyAttacked);
		}

		RmoveBindNexusInfoToHUD(Character);
	}

	if (TargetCharacter == OtherActor)
	{
		// 타겟 캐릭터가 범위를 벗어났을 경우 처리
		EnumRemoveFlags(CharacterState, ECharacterState::LMB);
		MulticastActivateTargetEffect(false);
		TargetCharacter = nullptr;

		// 타이머가 활성화되어 있으면 정지
		if (GetWorld()->GetTimerManager().IsTimerActive(TargetingTimerHandle))
		{
			GetWorld()->GetTimerManager().ClearTimer(TargetingTimerHandle);
		}
	}

	// 새로운 타겟 캐릭터를 선택
	if (!TargetCharacter && EnemiesInRange.Num() > 0)
	{
		TargetCharacter = SelectPriorityTarget();
		LMB_Started();
	}
}




/**
 * ANexus::LMB_Started - 타워의 기본 공격을 시작하는 함수입니다.
 * - 서버에서만 실행되며, 타워가 적 캐릭터를 타겟으로 설정하고 공격을 시작합니다.
 * - 유효한 타겟 캐릭터와 능력 상태를 확인한 후 타겟팅 효과를 활성화하고 공격 타이머를 시작합니다.
 *
 * 주요 기능:
 * 1. 서버 권한과 공격 가능 상태 확인
 * 2. 유효한 타겟이 존재하고 능력이 준비되었는지 확인
 * 3. 타겟팅 효과 활성화 및 공격 타이머 설정
 *
 * @return void
 */
void ANexus::LMB_Started()
{
	if (!HasAuthority()) return;

	// 기본 공격이 이미 활성화된 상태인지 확인
	if (EnumHasAnyFlags(CharacterState, ECharacterState::LMB))
	{
		return;
	}

	// 타겟 캐릭터의 유효성 확인
	if (::IsValid(TargetCharacter) == false)
	{
		return;
	}

	// 능력 통계 컴포넌트의 유효성 확인
	if (!::IsValid(ActionStatComponent))
	{
		return;
	}

	// 능력이 준비되지 않은 경우 반환
	if (!ActionStatComponent->IsActionReady(EActionSlot::LMB))
	{
		return;
	}

	// 기본 공격 상태 플래그 활성화 및 타겟팅 효과 활성화
	EnumAddFlags(CharacterState, ECharacterState::LMB);
	MulticastActivateTargetEffect(true);

	// 공격 딜레이 이후 타겟팅 효과 비활성화 및 공격 실행
	GetWorld()->GetTimerManager().SetTimer(TargetingTimerHandle, [this]()
		{
			MulticastActivateTargetEffect(false);
			LMB_Executed();
		},
		1.0f,
		false,
		TargetingDelay
	);
}



/**
 * ANexus::LMB_Executed - 이 함수는 LMB(Primary Attack) 능력 발동 시 호출되며, 타겟을 향해 발사체를 생성하고 발사합니다.
 * - 서버에서만 실행되며, 타겟 유효성 및 발사체, 효과 리소스를 확인합니다.
 * - 플레이어와 미니언에 대한 데미지 산출 방식을 구분하여 최종 데미지를 계산합니다.
 * - 발사체의 속성 및 이동 방식(추적형)을 설정하고 서버 및 클라이언트 모두에서 발사체를 동기화합니다.
 *
 *  주요 기능:
 *  1. 타겟과 능력 컴포넌트 유효성 확인
 *  2. 타겟의 타입에 따라 최종 데미지 계산
 *  3. 발사체의 위치와 방향 설정 및 속성 초기화
 *  4. 발사체의 추적 속성 및 클라이언트 동기화
 *
 *  반환: 없음
 */

void ANexus::LMB_Executed()
{
	if (HasAuthority() == false) return;

	// 타겟 캐릭터가 유효하지 않으면 타겟팅 효과를 비활성화하고 종료
	if (::IsValid(TargetCharacter) == false)
	{
		MulticastActivateTargetEffect(false);
		EnumRemoveFlags(CharacterState, ECharacterState::LMB);
		return;
	}

	// 능력 상태 컴포넌트가 유효하지 않으면 타겟팅 효과를 비활성화하고 종료
	if (::IsValid(ActionStatComponent) == false)
	{
		MulticastActivateTargetEffect(false);
		EnumRemoveFlags(CharacterState, ECharacterState::LMB);
		return;
	}

	// 투사체 클래스와 이펙트를 로드
	UClass* ProjectileClass = GetOrLoadResource<UClass>(GameplayClasses, TEXT("PrimaryProjectile"), TEXT(""));
	UParticleSystem* TrailEffect = GetOrLoadResource<UParticleSystem>(GameplayParticles, TEXT("CoreProjectile"), TEXT(""));

	// 투사체 클래스와 이펙트가 유효하지 않으면 경고 로그를 출력하고 종료
	if (!ProjectileClass || !TrailEffect)
	{
		MulticastActivateTargetEffect(false);
		EnumRemoveFlags(CharacterState, ECharacterState::LMB);
		UE_LOG(LogTemp, Error, TEXT("[%s] ProjectileClass or TrailEffect is nullptr."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	// 투사체의 생성 위치 및 회전 설정
	FVector SpawnLocation = GetActorLocation() + FVector(0, 0, 700);
	FRotator SpawnRotation = UKismetMathLibrary::MakeRotFromX(TargetCharacter->GetActorLocation() - SpawnLocation);
	FTransform Transform(SpawnRotation, SpawnLocation, FVector(1));

	// 능력 사용 및 쿨다운 적용
	ActionStatComponent->HandleActionExecution(EActionSlot::LMB, GetWorld()->GetTimeSeconds());
	ActionStatComponent->ActivateActionCooldown(EActionSlot::LMB);

	// 데미지 계산을 위한 능력 통계 및 캐릭터 공격력 가져오기
	const FActionAttributes& ActionAttributes = ActionStatComponent->GetActionAttributes(EActionSlot::LMB);
	const float CharacterAD = StatComponent->GetAttackDamage();
	const float BaseAttackDamage = ActionAttributes.AttackDamage;
	const float PhysicalScaling = ActionAttributes.PhysicalScaling;

	float FinalDamage = 0.0f;

	// 대상이 Minion일 때 타입에 따른 데미지 계산
	if (TargetCharacter->ObjectType == EObjectType::Minion)
	{
		AMinionBase* Minion = Cast<AMinionBase>(TargetCharacter);
		UStatComponent* TargetStatComponent = TargetCharacter->GetStatComponent();

		if (Minion && TargetStatComponent)
		{
			float MaxHP = TargetStatComponent->GetMaxHP();

			if (Minion->GetCharacterName().IsEqual(TEXT("Super")))
			{
				FinalDamage = MaxHP * 0.14f;
			}
			else if (Minion->GetCharacterName().IsEqual(TEXT("Melee")))
			{
				FinalDamage = MaxHP * 0.45f;
			}
			else if (Minion->GetCharacterName().IsEqual(TEXT("Ranged")))
			{
				FinalDamage = MaxHP * 0.7f;
			}
		}
	}
	// 대상이 Player일 때 물리 데미지 계산
	else if (TargetCharacter->ObjectType == EObjectType::Player)
	{
		FinalDamage = BaseAttackDamage + CharacterAD * PhysicalScaling;
	}
	// 그 외 대상에 대해 물리 데미지 계산
	else
	{
		FinalDamage = BaseAttackDamage + CharacterAD * PhysicalScaling;
	}

	// 데미지 정보를 생성
	FDamageInformation DamageInformation;
	DamageInformation.SetActionSlot(EActionSlot::LMB);
	DamageInformation.AddDamage(EDamageType::Physical, FinalDamage);


	AProjectile* Projectile = Cast<AProjectile>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), ProjectileClass, Transform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, this));
	if (Projectile != nullptr)
	{
		const float HomingAcceleration = GetUniqueAttribute(EActionSlot::LMB, TEXT("HomingAcceleration"), 0.0f);
		const float InitialSpeed = GetUniqueAttribute(EActionSlot::LMB, TEXT("InitialSpeed"), 2000.f);
		const float MaxSpeed = GetUniqueAttribute(EActionSlot::LMB, TEXT("MaxSpeed"), 0.0f);

		// 투사체의 타겟 및 이동 속성 설정
		Projectile->TargetActor = TargetCharacter;
		Projectile->TrailParticleSystem->Template = TrailEffect;
		Projectile->DamageInformation = DamageInformation;

		Projectile->TrailParticleSystem->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
		Projectile->BoxCollision->SetRelativeLocation(FVector(40, 0, 0));

		Projectile->ProjectileMovement->bIsHomingProjectile = true;
		Projectile->ProjectileMovement->ProjectileGravityScale = 0.0f;
		Projectile->ProjectileMovement->HomingTargetComponent = TargetCharacter->GetRootComponent();
		Projectile->ProjectileMovement->HomingAccelerationMagnitude = HomingAcceleration;
		Projectile->ProjectileMovement->InitialSpeed = InitialSpeed;
		Projectile->ProjectileMovement->MaxSpeed = MaxSpeed;

		Projectile->ProjectileInteractionType = EProjectileInteractionType::Unblockable;

		// 투사체를 스폰하고 클라이언트와 동기화
		UGameplayStatics::FinishSpawningActor(Projectile, Transform);
		Projectile->MulticastConfigureProjectile(TrailEffect, TargetCharacter, true, HomingAcceleration, InitialSpeed, MaxSpeed);
	}
}


void ANexus::LMB_CheckHit()
{

}


void ANexus::OnCooldownTimeChanged(EActionSlot SlotID, float CurrentCooldownTime, float MaxCooldownTime)
{
	if (EnumHasAnyFlags(SlotID, EActionSlot::LMB) && CurrentCooldownTime <= 0.0f)
	{
		EnumRemoveFlags(CharacterState, ECharacterState::LMB);

		if (EnemiesInRange.Find(TargetCharacter) != INDEX_NONE)
		{
			Count++;
			LMB_Started();
		}
		else
		{
			Count = 0;
			MulticastSetTargetCharacter(SelectPriorityTarget());
			LMB_Started();
		}
	}
}




/**
 * ANexus::OnAllyAttacked - 아군이 공격받았을 때 호출되는 함수입니다.
 * - 공격받은 아군과 공격한 적이 범위 내에 존재하는지 확인 후, 조건이 맞으면 해당 적을 새로운 타겟으로 설정합니다.
 * - 타겟팅 타이머가 비활성화 상태일 경우, 새로운 타겟에 대한 공격을 시작합니다.
 *
 * 주요 기능:
 * 1. 공격받은 아군 및 공격자 유효성 검사 및 캐스팅
 * 2. 아군과 적이 범위 내에 있는지 확인
 * 3. 조건에 맞으면 타겟팅을 갱신하고 공격 시작
 *
 * @param DamageReceiver 공격을 받은 캐릭터 (아군)
 * @param DamageCauser 공격을 가한 캐릭터 (적군)
 * @param InstigatorActor 공격을 유발한 컨트롤러
 * @param DamageInformation 데미지 정보
 */
void ANexus::OnAllyAttacked(AActor* DamageReceiver, AActor* DamageCauser, AController* InstigatorActor, FDamageInformation& DamageInformation)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ACharacterBase* Ally = Cast<ACharacterBase>(DamageReceiver);
	if (!Ally)
	{
		return;
	}

	ACharacterBase* Enemy = Cast<ACharacterBase>(DamageCauser);
	if (!Enemy)
	{
		return;
	}

	// 아군 및 적군이 사거리 내에 존재하는지 확인
	if (AlliesInRange.Find(Ally) == INDEX_NONE || EnemiesInRange.Find(Enemy) == INDEX_NONE)
	{
		return;
	}

	// 아군과 적군이 모두 플레이어일 경우 타겟팅 설정
	if (Ally->ObjectType == EObjectType::Player && Enemy->ObjectType == EObjectType::Player)
	{
		// 타겟팅 타이머 활성화 상태 확인
		bool IsTimerActive = World->GetTimerManager().IsTimerActive(TargetingTimerHandle);
		if (IsTimerActive)
		{
			World->GetTimerManager().ClearTimer(TargetingTimerHandle);
		}

		// 새로운 타겟 설정
		MulticastSetTargetCharacter(Enemy);
		LMB_Started();
	}
}


void ANexus::OnTargetEliminated(AActor* Eliminator)
{

}

void ANexus::MulticastSetTargetCharacter_Implementation(ACharacterBase* NewTarget)
{
	if (::IsValid(NewTarget))
	{
		TargetCharacter = NewTarget;
	}
	else
	{
		TargetCharacter = nullptr;
	}
}



void ANexus::MulticastActivateTargetEffect_Implementation(bool bActivation)
{
	if (GetNetMode() != NM_Client)
	{
		return;
	}

	if (!TargetBeamParticleSystem)
	{
		return;
	}

	if (bActivation)
	{
		TargetBeamParticleSystem->Activate();
	}
	else
	{
		TargetBeamParticleSystem->Deactivate();
	}
}


/**
 * ANexus::SelectPriorityTarget - 사거리 내에서 우선순위에 따라 타겟을 선택하는 함수입니다.
 * - Super 미니언 > Melee 미니언 > Ranged 미니언 > 플레이어 순으로 우선순위를 두고 타겟을 설정합니다.
 * - 각 우선순위 카테고리 내에서 랜덤으로 타겟을 선택하여 타워의 공격을 목표로 삼습니다.
 *
 * 주요 기능:
 * 1. EnemiesInRange 배열을 순회하여 적을 분류 (Super, Melee, Ranged Minions 및 Players)
 * 2. 우선순위에 따라 첫 번째로 등장한 적군을 선택
 * 3. 우선순위별 카테고리 내에서 랜덤으로 적군 선택
 *
 * @return 선택된 ACharacterBase* 타겟
 */
ACharacterBase* ANexus::SelectPriorityTarget()
{
	ACharacterBase* SelectedTarget = nullptr;
	TArray<ACharacterBase*> SuperMinions;
	TArray<ACharacterBase*> MeleeMinions;
	TArray<ACharacterBase*> RangedMinions;
	TArray<ACharacterBase*> Players;

	// 사거리 내 적을 순회하여 각 카테고리에 추가
	for (ACharacterBase* Enemy : EnemiesInRange)
	{
		if (!::IsValid(Enemy))
		{
			continue;
		}

		// 미니언 타입 분류: Super, Melee, Ranged
		if (EnumHasAnyFlags(Enemy->ObjectType, EObjectType::Minion))
		{
			AMinionBase* Minion = Cast<AMinionBase>(Enemy);
			if (Minion)
			{
				if (Minion->GetCharacterName().IsEqual(TEXT("Super")))
				{
					SuperMinions.Add(Minion);
				}
				else if (Minion->GetCharacterName().IsEqual(TEXT("Melee")))
				{
					MeleeMinions.Add(Minion);
				}
				else if (Minion->GetCharacterName().IsEqual(TEXT("Ranged")))
				{
					RangedMinions.Add(Minion);
				}
			}
		}
		// 플레이어 캐릭터 분류
		else if (EnumHasAnyFlags(Enemy->ObjectType, EObjectType::Player))
		{
			Players.Add(Enemy);
		}
	}

	// 우선순위에 따라 타겟 설정
	if (SuperMinions.Num() > 0)
	{
		SelectedTarget = SuperMinions[FMath::RandRange(0, SuperMinions.Num() - 1)];
	}
	else if (MeleeMinions.Num() > 0)
	{
		SelectedTarget = MeleeMinions[FMath::RandRange(0, MeleeMinions.Num() - 1)];
	}
	else if (RangedMinions.Num() > 0)
	{
		SelectedTarget = RangedMinions[FMath::RandRange(0, RangedMinions.Num() - 1)];
	}
	else if (Players.Num() > 0)
	{
		SelectedTarget = Players[FMath::RandRange(0, Players.Num() - 1)];
	}

	return SelectedTarget;
}



void ANexus::OnCharacterDeath()
{
	if (HasAuthority() == false)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	AArenaGameMode* GameMode = Cast<AArenaGameMode>(UGameplayStatics::GetGameMode(World));
	if (!GameMode)
	{
		return;
	}

	if (StatComponent->OnOutOfCurrentHP.IsAlreadyBound(this, &ThisClass::OnCharacterDeath))
	{
		StatComponent->OnOutOfCurrentHP.RemoveDynamic(this, &ThisClass::OnCharacterDeath);
	}

	EnumAddFlags(CharacterState, ECharacterState::Death);

	UCapsuleComponent* CapsuleCollision = GetCapsuleComponent();
	if (CapsuleCollision)
	{
		CapsuleCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	DetectionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetActorTickEnabled(false);
	if (World->GetTimerManager().IsTimerActive(TargetingTimerHandle))
	{
		World->GetTimerManager().ClearTimer(TargetingTimerHandle);
	}

	TargetBeamParticleSystem->SetActive(false);
	GameMode->NotifyNexusDestroyed(this);
}

void ANexus::MulticastOnCharacterDeath_Implementation()
{
}



void ANexus::BindNexusInfoToHUD(ACharacterBase* Player)
{
	if (::IsValid(Player) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: Player is invalid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	AAOSPlayerController* PlayerController = Cast<AAOSPlayerController>(Player->GetController());
	if (!PlayerController)
	{
		return;
	}

	PlayerController->ClientBindTargetStatusWidget(StatComponent);
}

void ANexus::RmoveBindNexusInfoToHUD(ACharacterBase* Player)
{
	if (::IsValid(Player) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Failed: Player is invalid."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	AAOSPlayerController* PlayerController = Cast<AAOSPlayerController>(Player->GetController());
	if (!PlayerController)
	{
		return;
	}

	PlayerController->ClientRemoveTargetStatusWidget();
}


void ANexus::ActivateCamera()
{
	if (::IsValid(NexusCamera) == false)
	{
		return;
	}

	NexusCamera->Activate();
}

template<typename T>
inline T* ANexus::LoadOrGetResourceWithStaticLoad(TMap<FName, T*>& ResourceMap, const FName& Key, const TCHAR* Path)
{
	// 이미 캐시에 리소스가 존재하는 경우
	if (ResourceMap.Contains(Key))
	{
		return ResourceMap[Key];
	}

	// StaticLoadObject를 사용하여 리소스 로드
	static ConstructorHelpers::FObjectFinder<T> RESOURCE(Path);
	if (RESOURCE.Succeeded())
	{
		ResourceMap.Add(Key, RESOURCE.Object);  // 맵에 리소스 추가
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GetOrLoadResource: Failed to load %s resource at path %s"), *Key.ToString(), Path);
	}

	return RESOURCE.Object;
}




void ANexus::SetMaterialColor(UPrimitiveComponent* MeshComponent, int32 MaterialIndex, FName ParameterName, const FLinearColor& Color)
{
	if (!MeshComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] MeshComponent is null."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	UMaterialInterface* MaterialInterface = MeshComponent->GetMaterial(MaterialIndex);
	if (!MaterialInterface)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] No material found at index %d on mesh component."), ANSI_TO_TCHAR(__FUNCTION__), MaterialIndex);
		return;
	}

	UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(MaterialIndex);
	if (DynamicMaterial)
	{
		DynamicMaterial->SetVectorParameterValue(ParameterName, Color);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to create dynamic material instance for mesh component."), ANSI_TO_TCHAR(__FUNCTION__));
	}
}


#if WITH_EDITOR
void ANexus::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 변경 사항에 따라 처리하는 코드
	if (PropertyChangedEvent.Property)
	{
		// Core의 PrimaryColor 및 SecondaryColor 설정
		SetMaterialColor(SphereGrid_A, 0, PrimaryColorParam, CorePrimartColor);
		SetMaterialColor(SphereGrid_B, 0, PrimaryColorParam, CorePrimartColor);
		SetMaterialColor(SphereHalo_A, 0, CenterColorParam, CorePrimartColor);

		SetMaterialColor(SphereGrid_A, 0, SecondaryColorParam, CoreSecondaryColor);
		SetMaterialColor(SphereGrid_B, 0, SecondaryColorParam, CoreSecondaryColor);

		// Ring 색상 설정
		SetMaterialColor(Ring_A, 0, PrimaryColorParam, RingPrimaryColor);
		SetMaterialColor(Ring_A, 0, SecondaryColorParam, RingSecondaryColor);

		SetMaterialColor(Ring_B, 0, PrimaryColorParam, RingPrimaryColor);
		SetMaterialColor(Ring_B, 0, SecondaryColorParam, RingSecondaryColor);

		// Center 색상 설정
		SetMaterialColor(NexusCenter, 0, CenterColorParam, CenterPrimaryColor);

		PointLight->SetLightColor(CoreEmissionColor);
	}
}
#endif