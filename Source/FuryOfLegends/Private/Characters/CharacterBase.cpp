#include "Characters/CharacterBase.h"

// Unreal Engine 기본 헤더
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// 프로젝트 컴포넌트 관련 헤더
#include "Components/ActionStatComponent.h"
#include "Components/CharacterWidgetComponent.h"
#include "Components/StatComponent.h"
#include "Components/WidgetComponent.h"

// UI 관련 헤더
#include "Blueprint/WidgetLayoutLibrary.h"
#include "UI/DamageNumberWidget.h"

// Crowd Control 관련 헤더
#include "CrowdControls/CrowdControlEffect.h"
#include "CrowdControls/CrowdControlManager.h"

// 게임 관련 헤더
#include "Game/AOSGameInstance.h"

// 기타 유틸리티
#include "Particles/ParticleSystemComponent.h"
#include "Plugins/GameTimerManager.h"

// 구조체 관련 헤더
#include "Structs/CustomCombatData.h"
#include "Structs/CharacterResources.h"


TMap<FName, UParticleSystem*> ACharacterBase::SharedGameplayParticles;

ACharacterBase::ACharacterBase()
{
	HomingTargetSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HomingTarget"));
	HomingTargetSceneComponent->SetupAttachment(RootComponent);

	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	bNetLoadOnClient = true;

	TeamSide = ETeamSide::None;
	ObjectType = EObjectType::None;
	CharacterState = ECharacterState::None;
	CrowdControlState = ECrowdControl::None;

	LastHitCharacter = nullptr;
	DamageNumberWidgetClass = nullptr; 

	TotalAttacks = 0;
	CriticalHits = 0;
	ComboCount = 1;
	MaxComboCount = 1;

	ActiveEffects.Empty();
	CrowdControlManager = nullptr;

	CharacterName = NAME_None;
}


void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CrowdControlManager = UCrowdControlManager::Get();
		OnHitEventTriggered.AddDynamic(this, &ACharacterBase::ProcessCriticalHit);
	}
	
	StatComponent->OnMovementSpeedChanged.AddDynamic(this, &ACharacterBase::OnMovementSpeedChanged);

	EnumAddFlags(CharacterState, ECharacterState::Move);
	EnumAddFlags(CharacterState, ECharacterState::Jump);
	EnumAddFlags(CharacterState, ECharacterState::SwitchAction);

	OnMovementSpeedChanged(0, StatComponent->GetMovementSpeed());
}



void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateWidgetToLocalPlayer();
}



void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}



void ACharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, TeamSide, COND_None);
	DOREPLIFETIME_CONDITION(ThisClass, ObjectType, COND_None);
	DOREPLIFETIME_CONDITION(ThisClass, CharacterState, COND_None);
	DOREPLIFETIME_CONDITION(ThisClass, CrowdControlState, COND_None);
}

//==================== Initialization and Resource Functions ====================//

void ACharacterBase::InitializeCharacterResources()
{
	UWorld* CurrentWorld = GetWorld();
	if (!CurrentWorld)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeCharacterResources: Failed to retrieve the World context. Unable to proceed with initialization."));
		return;
	}

	UAOSGameInstance* AOSGameInstance = Cast<UAOSGameInstance>(UGameplayStatics::GetGameInstance(this));
	if (!AOSGameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("InitializeCharacterResources: Failed to cast to UAOSGameInstance. Make sure the GameInstance class is set correctly in the project settings."));
		return;
	}

	// SharedGamePlayParticlesMap가 비어 있는 경우에만 초기화
	if (SharedGameplayParticles.Num() > 0)
	{
		return;
	}

	const UDataTable* SharedGameplayTable = AOSGameInstance->GetSharedGamePlayParticlesDataTable();
	if (SharedGameplayTable == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SharedGameplayTable is null. Initialization aborted."));
		return;
	}

	for (const auto& Row : SharedGameplayTable->GetRowMap())
	{
		const FSharedGameplay* RowData = reinterpret_cast<const FSharedGameplay*>(Row.Value);
		if (!RowData) return;

		for (const auto& Attribute : RowData->SharedGameplayParticles)
		{
			if (!SharedGameplayParticles.Contains(Attribute.Key))
			{
				SharedGameplayParticles.Add(Attribute.Key, Attribute.Value);
			}
		}
	}
}

void ACharacterBase::SetWidget(class UUserWidgetBase* InUserWidgetBase)
{
}

UAnimMontage* ACharacterBase::GetOrLoadMontage(const FName& Key, const TCHAR* Path)
{
	return GetOrLoadResource<UAnimMontage>(GameplayMontages, Key, Path);
}

UParticleSystem* ACharacterBase::GetOrLoadParticle(const FName& Key, const TCHAR* Path)
{
	return GetOrLoadResource<UParticleSystem>(GameplayParticles, Key, Path);
}

UStaticMesh* ACharacterBase::GetOrLoadMesh(const FName& Key, const TCHAR* Path)
{
	return GetOrLoadResource<UStaticMesh>(GameplayMeshes, Key, Path);
}

UMaterialInstance* ACharacterBase::GetOrLoadMaterial(const FName& Key, const TCHAR* Path)
{
	return GetOrLoadResource<UMaterialInstance>(GameplayMaterials, Key, Path);
}

UClass* ACharacterBase::GetOrLoadClass(const FName& Key, const TCHAR* Path)
{
	return GetOrLoadResource<UClass>(GameplayClasses, Key, Path);
}

UTexture* ACharacterBase::GetOrLoadTexture(const FName& Key, const TCHAR* Path)
{
	return GetOrLoadResource<UTexture>(GameplayTextures, Key, Path);
}

UParticleSystem* ACharacterBase::GetOrLoadSharedParticle(const FName& Key, const TCHAR* Path)
{
	return GetOrLoadResource<UParticleSystem>(SharedGameplayParticles, Key, Path);
}

template<typename T>
T* ACharacterBase::GetOrLoadResource(TMap<FName, T*>& ResourceMap, const FName& Key, const TCHAR* Path)
{
	// 이미 캐시에 리소스가 존재하는 경우
	if (ResourceMap.Contains(Key))
	{
		return ResourceMap[Key];
	}

	// StaticLoadObject를 사용하여 리소스 로드
	T* Resource = Cast<T>(StaticLoadObject(T::StaticClass(), nullptr, Path));
	if (Resource)
	{
		ResourceMap.Add(Key, Resource);  // 맵에 리소스 추가
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GetOrLoadResource: Failed to load %s resource at path %s"), *Key.ToString(), Path);
	}

	return Resource;
}


UStatComponent* ACharacterBase::GetStatComponent() const
{
	return StatComponent;
}

UActionStatComponent* ACharacterBase::GetActionStatComponent() const
{
	return ActionStatComponent;
}

void ACharacterBase::RotateWidgetToLocalPlayer()
{
	// 로컬 플레이어의 카메라 가져오기
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController || !WidgetComponent)
	{
		return;
	}

	// 카메라의 위치와 방향
	FVector CameraLocation;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	// 위젯이 로컬 플레이어를 향하게 회전
	FVector MinionLocation = WidgetComponent->GetComponentLocation();
	FRotator LookAtRotation = (CameraLocation - MinionLocation).Rotation();
	WidgetComponent->SetWorldRotation(LookAtRotation);
}


float ACharacterBase::GetUniqueAttribute(EActionSlot SlotID, const FName& Key, float DefaultValue) const
{
	if (ActionStatComponent)
	{
		return ActionStatComponent->GetUniqueValue(SlotID, Key, DefaultValue);
	}
	return DefaultValue;
}

float ACharacterBase::AdjustAnimPlayRate(const float AnimLength)
{
	if (!::IsValid(StatComponent))
	{
		UE_LOG(LogTemp, Error, TEXT("AdjustAnimPlayRate: StatComponent is null."));
		return 1.0f;
	}

	float CurrentAttackSpeed = StatComponent->GetAttackSpeed();
	float AttackIntervalTime = 1.0f / CurrentAttackSpeed;

	float PlayRate = (AttackIntervalTime < AnimLength) ? (AnimLength / AttackIntervalTime) : 1.0f;

	const float MinPlayRate = 0.5f;
	const float MaxPlayRate = 2.0f;
	PlayRate = FMath::Clamp(PlayRate, MinPlayRate, MaxPlayRate);

	return PlayRate;
}

//==================== Damage-related Functions ====================//

void ACharacterBase::ServerApplyDamage_Implementation(ACharacterBase* Enemy, AActor* DamageCauser, AController* EventInstigator, FDamageInformation DamageInformation)
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (::IsValid(Enemy) == false)
	{
		return;
	}

	if (DamageInformation.ActionSlot == EActionSlot::LMB)
	{
		if (EnumHasAnyFlags(CrowdControlState, ECrowdControl::Blind))
		{
			// To do
			return;
		}
	}

	bool bIsDamageReceived = Enemy->ReceiveDamage(DamageCauser, EventInstigator, DamageInformation);
	if (!bIsDamageReceived)
	{
		return;
	}
	
	if (DamageInformation.CrowdControls.Num() > 0)
	{
		for (auto& CrowdControl : DamageInformation.CrowdControls)
		{
			ServerApplyCrowdControl(Enemy, DamageCauser, EventInstigator, CrowdControl);
		}
	}
}


bool ACharacterBase::ReceiveDamage(AActor* DamageCauser, AController* EventInstigator, FDamageInformation DamageInformation)
{
	if (EnumHasAnyFlags(CharacterState, ECharacterState::Invulnerability))
	{
		return false;
	}

	bool bResult = true;
	if (OnReceiveDamageEnteredEvent.IsBound())
	{
		OnReceiveDamageEnteredEvent.Broadcast(bResult);
	}

	if (!bResult)
	{
		return false;
	}

	float FinalDamageAmount = 0;
	if (OnPreReceiveDamageEvent.IsBound())
	{
		OnPreReceiveDamageEvent.Broadcast(this, DamageCauser, EventInstigator, DamageInformation);
	}

	if (EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::Physical) || EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::Critical))
	{
		if (EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::Critical))
		{
			float ReducedCriticalDamage = DamageInformation.PhysicalDamage * (100 / (100 + StatComponent->GetDefensePower()));
			FinalDamageAmount += ReducedCriticalDamage;
			DamageInformation.PhysicalDamage = ReducedCriticalDamage;
		}
		else
		{
			float ReducedPhysicalDamage = DamageInformation.PhysicalDamage * (100 / (100 + StatComponent->GetDefensePower()));
			FinalDamageAmount += ReducedPhysicalDamage;
			DamageInformation.PhysicalDamage = ReducedPhysicalDamage;
		}
	}

	if (EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::Magic))
	{
		float ReducedMagicDamage = DamageInformation.MagicDamage * (100 / (100 + StatComponent->GetMagicResistance()));
		FinalDamageAmount += ReducedMagicDamage;
		DamageInformation.MagicDamage = ReducedMagicDamage;
	}

	if (EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::TrueDamage))
	{
		FinalDamageAmount += DamageInformation.TrueDamage;
	}


	if (ACharacterBase* EnemyCharacter = Cast<ACharacterBase>(DamageCauser))
	{
		if (EnumHasAnyFlags(DamageInformation.AttackTrigger, EAttackTrigger::OnHit) && EnemyCharacter->OnHitEventTriggered.IsBound())
		{
			EnemyCharacter->OnHitEventTriggered.Broadcast(DamageInformation);
		}

		if (EnumHasAnyFlags(DamageInformation.AttackTrigger, EAttackTrigger::OnAttack) && EnemyCharacter->OnAttackEventTriggered.IsBound())
		{
			EnemyCharacter->OnAttackEventTriggered.Broadcast(DamageInformation);
		}

		if (EnumHasAnyFlags(DamageInformation.AttackTrigger, EAttackTrigger::AbilityEffects) && EnemyCharacter->OnAbilityEffectsEventTriggered.IsBound())
		{
			EnemyCharacter->OnAbilityEffectsEventTriggered.Broadcast(DamageInformation);
		}
	}

	StatComponent->ModifyCurrentHP(-FinalDamageAmount);
	ClientSpawnDamageWidget(this, FinalDamageAmount, DamageInformation);

	if (OnPostReceiveDamageEvent.IsBound())
	{
		OnPostReceiveDamageEvent.Broadcast(this, DamageCauser, EventInstigator, DamageInformation);
	}

	ACharacterBase* DamageCauserActor = Cast<ACharacterBase>(DamageCauser);
	if (::IsValid(DamageCauserActor))
	{
		DamageCauserActor->ClientSpawnDamageWidget(this, FinalDamageAmount, DamageInformation);
		ProcessDamageCauser(DamageCauser);
	}

	return true;
}

void ACharacterBase::ProcessCriticalHit(FDamageInformation& DamageInformation)
{
	float CriticalChance = static_cast<float>(StatComponent->GetCriticalChance()) / 100;
	if (CriticalChance <= 0)
	{
		return;
	}

	float ExpectedCriticalHits = TotalAttacks * CriticalChance;
	float ActualCriticalHits = static_cast<float>(CriticalHits);

	float Adjustment = (ExpectedCriticalHits - ActualCriticalHits) / (TotalAttacks + 1);
	float AdjustedCriticalChance = CriticalChance + Adjustment;

	bool bIsCriticalHit = (FMath::FRand() <= AdjustedCriticalChance);

	if (bIsCriticalHit)
	{
		CriticalHits++;

		if (::IsValid(StatComponent))
		{
			float AttackDamage = StatComponent->GetAttackDamage();
			DamageInformation.AddDamage(EDamageType::Critical, AttackDamage * 0.7);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AAOSCharacterBase::ApplyCriticalHitDamage - StatComponent is not valid."));
		}
	}
	TotalAttacks++;
}


void ACharacterBase::ProcessDamageCauser(AActor* DamageCauser)
{
	ACharacterBase* EnemyCharacter = Cast<ACharacterBase>(DamageCauser);
	if (::IsValid(EnemyCharacter) == false)
	{
		return;
	}

	LastHitCharacter = DamageCauser;

	// 데미지를 준 캐릭터가 플레이어인지 확인
	if (EnumHasAnyFlags(EnemyCharacter->ObjectType, EObjectType::Player) == false)
	{
		return;
	}

	float CurrentTime = GetWorld()->GetTimeSeconds();

	// 이미 기록이 있는지 확인하여 값이 있으면 업데이트, 없으면 추가
	if (float* DamageTime = PlayerHitTimestamps.Find(EnemyCharacter))
	{
		*DamageTime = CurrentTime;
	}
	else
	{
		PlayerHitTimestamps.Add(EnemyCharacter, CurrentTime);
	}
}


//==================== Crowd Control Functions ====================//

void ACharacterBase::ServerApplyCrowdControl_Implementation(ACharacterBase* Enemy, AActor* DamageCauser, AController* EventInstigator, FCrowdControlInformation CrowdControlInfo)
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (::IsValid(Enemy) == false)
	{
		return;
	}

	bool bCrowdControlApplied = Enemy->ReceiveCrowdControl(DamageCauser, EventInstigator, CrowdControlInfo);

	if (OnPostApplyCrowdControlEvent.IsBound())
	{
		OnPostApplyCrowdControlEvent.Broadcast(bCrowdControlApplied, Enemy, DamageCauser, EventInstigator, CrowdControlInfo);
	}
}

bool ACharacterBase::ReceiveCrowdControl(AActor* DamageCauser, AController* EventInstigator, FCrowdControlInformation CrowdControlInfo)
{
	bool bResult = true;
	if (OnReceiveCrowdControlEnteredEvent.IsBound())
	{
		OnReceiveCrowdControlEnteredEvent.Broadcast(bResult);
	}

	if (!bResult)
	{
		return false;
	}

	ECrowdControl Type = CrowdControlInfo.Type;

	// CrowdControlManager가 유효한지 확인
	if (!::IsValid(CrowdControlManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("CrowdControlManager is null. Cannot apply crowd control effect of type: %d"), static_cast<int32>(Type));
		return false;
	}

	// CrowdControlManager에서 효과 클래스 가져오기
	TSubclassOf<UCrowdControlEffect> EffectClass = CrowdControlManager->GetEffectClass(Type);
	if (!::IsValid(EffectClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("No effect class found for crowd control type: %d"), static_cast<int32>(Type));
		return false;
	}

	if (OnPreReceiveCrowdControlEvent.IsBound())
	{
		OnPreReceiveCrowdControlEvent.Broadcast(this, DamageCauser, EventInstigator, CrowdControlInfo);
	}

	// 이미 효과가 활성화된 경우 처리
	if (UCrowdControlEffect* ExistingEffect = ActiveEffects.FindRef(Type))
	{
		if (ExistingEffect)
		{
			ExistingEffect->ApplyEffect(this, CrowdControlInfo.Duration, CrowdControlInfo.Percent);
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Existing effect for type %d is invalid, removing from ActiveEffects"), static_cast<int32>(Type));
			ActiveEffects.Remove(Type);
		}
	}

	// 객체 풀에서 효과 객체 가져오기
	UCrowdControlEffect* NewEffect = CrowdControlManager->GetEffect(Type);
	if (!::IsValid(NewEffect))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to retrieve new crowd control effect object from pool for type %d"), static_cast<int32>(Type));
		return false;
	}

	NewEffect->ApplyEffect(this, CrowdControlInfo.Duration, CrowdControlInfo.Percent);
	ActiveEffects.Add(Type, NewEffect);

	if (OnPostReceiveCrowdControlEvent.IsBound())
	{
		OnPostReceiveCrowdControlEvent.Broadcast(this, DamageCauser, EventInstigator, CrowdControlInfo);
	}

	return true;
}



//==================== Particle Functions ====================//

UParticleSystemComponent* ACharacterBase::SpawnEmitterAtLocation(UParticleSystem* Particle, FTransform Transform, bool bAutoDestory, EPSCPoolMethod PoolingMethod, bool bAutoActivate)
{
	// 서버에서만 실행되도록 체크
	if (HasAuthority() == false)
	{
		return nullptr;
	}

	// 파티클 생성
	UParticleSystemComponent* ParticleSystemComponent = UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(), Particle, Transform.GetLocation(), Transform.GetRotation().Rotator(), Transform.GetScale3D(), bAutoDestory, PoolingMethod, bAutoActivate);

	// 클라이언트들에게 파티클 생성 요청
	MulticastSpawnEmitterAtLocation(Particle, Transform);

	return ParticleSystemComponent;
}

void ACharacterBase::MulticastSpawnEmitterAtLocation_Implementation(UParticleSystem* Particle, FTransform Transform, bool bAutoDestory, EPSCPoolMethod PoolingMethod, bool bAutoActivate)
{
	// 서버에서는 실행하지 않음
	if (HasAuthority())
	{
		return;
	}

	// 파티클 유효성 체크
	if (::IsValid(Particle) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("MulticastSpawnEmitterAtLocation: Invalid ParticleSystem."));
		return;
	}

	// 클라이언트에서 파티클 생성
	UParticleSystemComponent* ParticleSystemComponent = UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(), Particle, Transform.GetLocation(), Transform.GetRotation().Rotator(), Transform.GetScale3D(), bAutoDestory, PoolingMethod, bAutoActivate);
}

void ACharacterBase::ServerSpawnEmitterAttached_Implementation(UParticleSystem* Particle, USceneComponent* AttachToComponent, FTransform Transform, EAttachLocation::Type LocationType)
{
	if (HasAuthority())
	{
		MulticastSpawnEmitterAttached(Particle, AttachToComponent, Transform, LocationType);
	}
}

void ACharacterBase::MulticastSpawnEmitterAttached_Implementation(UParticleSystem* Particle, USceneComponent* AttachToComponent, FTransform Transform, EAttachLocation::Type LocationType)
{
	if (!HasAuthority())
	{
		UGameplayStatics::SpawnEmitterAttached(
			Particle, AttachToComponent, "Name_None", Transform.GetLocation(), Transform.GetRotation().Rotator(), Transform.GetScale3D(), LocationType, true, EPSCPoolMethod::None, true
		);
	}
}

//==================== Mesh Functions ====================//

void ACharacterBase::ServerSpawnMeshAttached_Implementation(UStaticMesh* MeshToSpawn, USceneComponent* AttachToComponent, float Duration)
{
	if (HasAuthority())
	{
		MulticastSpawnMeshAttached(MeshToSpawn, AttachToComponent, Duration);
	}
}

void ACharacterBase::MulticastSpawnMeshAttached_Implementation(UStaticMesh* MeshToSpawn, USceneComponent* AttachToComponent, float Duration)
{
	if (HasAuthority())
	{
		return;
	}

	if (::IsValid(MeshToSpawn) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Mesh is not valid"), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	UStaticMeshComponent* NewMeshComponent = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass());
	if (::IsValid(NewMeshComponent) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Failed to create new mesh component."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, false);

	NewMeshComponent->CreationMethod = EComponentCreationMethod::Instance;
	NewMeshComponent->AttachToComponent(AttachToComponent != nullptr ? AttachToComponent : GetRootComponent(), AttachmentRules);
	NewMeshComponent->SetStaticMesh(MeshToSpawn);
	NewMeshComponent->SetCollisionProfileName("CharacterMesh");
	NewMeshComponent->RegisterComponent();

	FTimerHandle NewTimerHandle;
	GetWorldTimerManager().SetTimer(NewTimerHandle,[MeshComponent = NewMeshComponent]()
		{
			if (MeshComponent)
			{
				MeshComponent->DestroyComponent();
			}
		},
		Duration,
		false
	);
}

void ACharacterBase::ServerSpawnActorAtLocation_Implementation(UClass* SpawnActor, FTransform SpawnTransform)
{
	if (HasAuthority())
	{
		if (!SpawnActor)
		{
			return;
		}

		GetWorld()->SpawnActor<AActor>(SpawnActor, SpawnTransform);
	}
}

//==================== Montage-related functions ====================//

void ACharacterBase::PlayMontage(const FString& MontageName, float PlayRate, FName StartSectionName, const TCHAR* Path)
{
	UAnimMontage* Montage = GetOrLoadMontage(FName(*MontageName), Path);
	if (!Montage)
	{
		return;
	}

	PlayAnimMontage(Montage, PlayRate, StartSectionName);
	ServerPlayMontage(Montage, PlayRate, StartSectionName);
}

void ACharacterBase::ServerStopMontage_Implementation(float BlendOut, UAnimMontage* Montage, bool bApplyToLocalPlayer)
{
	if (HasAuthority() == false)
	{
		return;
	}

	MulticastStopMontage(BlendOut, Montage, bApplyToLocalPlayer);
}


void ACharacterBase::MulticastStopMontage_Implementation(float BlendOut, UAnimMontage* Montage, bool bApplyToLocalPlayer)
{
	if (!HasAuthority() && GetController() == UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (bApplyToLocalPlayer)
		{
			AnimInstance->Montage_Stop(BlendOut, Montage);
		}
		return;
	}

	AnimInstance->Montage_Stop(BlendOut, Montage);
}



void ACharacterBase::ServerPlayMontage_Implementation(UAnimMontage* Montage, float PlayRate, FName SectionName, bool bApplyToLocalPlayer)
{
	if (HasAuthority() == false)
	{
		return;
	}

	PlayAnimMontage(Montage, PlayRate, SectionName);
	MulticastPlayMontage(Montage, PlayRate, SectionName, bApplyToLocalPlayer);
}

void ACharacterBase::MulticastPlayMontage_Implementation(UAnimMontage* Montage, float PlayRate, FName SectionName, bool bApplyToLocalPlayer)
{
	if (!Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("MulticastPlayMontage_Implementation: Montage is null."));
		return;
	}

	if (HasAuthority())
	{
		return;
	}

	if (!HasAuthority() && GetController() == UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (bApplyToLocalPlayer)
		{
			PlayAnimMontage(Montage, PlayRate, SectionName);
		}
		return;
	}

	PlayAnimMontage(Montage, PlayRate, SectionName);
}


void ACharacterBase::ServerStopAllMontages_Implementation(float BlendOut, bool bApplyToLocalPlayer)
{
	if (HasAuthority() == false)
	{
		return;
	}

	MulticastStopAllMontages(BlendOut, bApplyToLocalPlayer);
}

void ACharacterBase::MulticastStopAllMontages_Implementation(float BlendOut, bool bApplyToLocalPlayer)
{
	if (!::IsValid(AnimInstance))
	{
		return;
	}

	if ((!HasAuthority() && GetController() == UGameplayStatics::GetPlayerController(GetWorld(), 0)) && !bApplyToLocalPlayer)
	{
		return;
	}

	AnimInstance->StopAllMontages(BlendOut);
}

void ACharacterBase::ServerMontageJumpToSection_Implementation(const UAnimMontage* Montage, FName SectionName, bool bApplyToLocalPlayer)
{
	MulticastMontageJumpToSection(Montage, SectionName, bApplyToLocalPlayer);
}

void ACharacterBase::MulticastMontageJumpToSection_Implementation(const UAnimMontage* Montage, FName SectionName, bool bApplyToLocalPlayer)
{
	if ((!HasAuthority() && GetController() == UGameplayStatics::GetPlayerController(this, 0)) && !bApplyToLocalPlayer)
	{
		return;
	}

	if (::IsValid(AnimInstance) == false)
	{
		return;
	}

	AnimInstance->Montage_JumpToSection(SectionName, Montage);
}


void ACharacterBase::MulticastPauseMontage_Implementation()
{
	/*if ((!HasAuthority() && GetOwner() == UGameplayStatics::GetPlayerController(this, 0)) || IsLocallyControlled())
	{
		return;
	}*/

	if (::IsValid(AnimInstance) == false)
	{
		return;
	}

	AnimInstance->Montage_Pause(AnimInstance->GetCurrentActiveMontage());
}

void ACharacterBase::MulticastResumeMontage_Implementation()
{
	if (::IsValid(AnimInstance) == false)
	{
		return;
	}

	// 애니메이션 재개
	if (AnimInstance && !AnimInstance->Montage_IsPlaying(AnimInstance->GetCurrentActiveMontage()))
	{
		AnimInstance->Montage_Resume(AnimInstance->GetCurrentActiveMontage());
	}
}

// -----------------------------------------------------------

void ACharacterBase::ClientSpawnDamageWidget_Implementation(AActor* Target, const float DamageAmount, const FDamageInformation& DamageInformation)
{
	if (DamageNumberWidgetClass == nullptr)
	{
		return;
	}

	UWidgetComponent* DamageWidgetComponent = NewObject<UWidgetComponent>(Target);
	if (::IsValid(DamageWidgetComponent) == false)
	{
		return;
	}

	DamageWidgetComponent->SetupAttachment(Target->GetRootComponent());

	// 랜덤 위치 오프셋 생성 (머리 위에 랜덤한 위치)
	FVector RandomOffset(FMath::FRandRange(-30.0f, 30.0f), FMath::FRandRange(-30.0f, 30.0f), FMath::FRandRange(40, 80.0f));
	DamageWidgetComponent->SetRelativeLocation(RandomOffset);
	DamageWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);    // 화면 공간에 표시
	DamageWidgetComponent->SetDrawAtDesiredSize(true);              // 크기를 자동으로 조정
	DamageWidgetComponent->RegisterComponent();                     // 위젯 활성화
	DamageWidgetComponent->SetInitialLayerZOrder(0);                // 위젯이 모든 것보다 앞에 배치됨
	DamageWidgetComponent->SetWidgetClass(DamageNumberWidgetClass); // 위젯 클래스 설정

	UDamageNumberWidget* DamageWidget = Cast<UDamageNumberWidget>(DamageWidgetComponent->GetUserWidgetObject());
	if (DamageWidget)
	{
		// 데미지 타입에 따른 텍스트 색상 및 크기 설정
		FLinearColor TextColor = FLinearColor::White;
		float TextScale = 1.0f;

		// 물리 피해
		if (EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::Physical))
		{
			TextColor = FLinearColor(255.0f / 255.0f, 22.0f / 255.0f, 15.0f / 255.0f, 255.0f / 255.0f);
			TextScale = 0.3f;
		}
		// 마법 피해
		else if (EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::Magic))
		{
			TextColor = FLinearColor(26.0f / 255.0f, 29.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);
			TextScale = 0.8f;
		}
		// 치명타
		else if (EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::Critical))
		{
			TextColor = FLinearColor::Red;
			TextScale = 1.0f;
		}
		// 고정 피해
		else if (EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::TrueDamage))
		{
			TextColor = FLinearColor(145.0f / 255.0f, 145.0f / 255.0f, 145.0f / 255.0f, 255.0f / 255.0f);
			TextScale = 1.0f;
		}

		// 데미지 설정
		DamageWidget->SetDamageAmount(DamageAmount, TextColor, TextScale);
		DamageWidgetQueue.Enqueue(DamageWidgetComponent);  // 위젯 큐에 추가

		// 애니메이션이 끝났을 때 제거하기 위해 콜백 설정
		if (UWidgetAnimation* Anim = DamageWidget->GetFadeOutAnimation())
		{
			FWidgetAnimationDynamicEvent EndEvent;
			EndEvent.BindDynamic(this, &ACharacterBase::OnDamageWidgetAnimationFinished);
			DamageWidget->BindToAnimationFinished(Anim, EndEvent);

			// 애니메이션 재생
			DamageWidget->PlayAnimation(Anim);
		}
	}
}

void ACharacterBase::OnDamageWidgetAnimationFinished()
{
	UWidgetComponent* DamageWidgetComponent = nullptr;

	// 큐에서 위젯 컴포넌트를 꺼내 제거
	if (DamageWidgetQueue.Dequeue(DamageWidgetComponent))
	{
		if (::IsValid(DamageWidgetComponent) || !DamageWidgetComponent->IsPendingKillEnabled())
		{
			DamageWidgetComponent->DestroyComponent();
		}
	}
}


void ACharacterBase::ClientSetControlRotation_Implementation(FRotator NewRotation)
{
	if (::IsValid(Controller))
	{
		Controller->SetControlRotation(NewRotation);
		SetActorRotation(NewRotation);
	}
}


//==================== State Functions ====================//

void ACharacterBase::ServerModifyCharacterState_Implementation(ECharacterStateOperation Operation, ECharacterState StateFlag)
{
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Called without authority."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	// Move, Jump, SwitchAction 은 기본 활성화 상태
	const bool bIsDefaultActive = EnumHasAnyFlags(StateFlag, ECharacterState::Move) ||
		EnumHasAnyFlags(StateFlag, ECharacterState::Jump) ||
		EnumHasAnyFlags(StateFlag, ECharacterState::SwitchAction);

	int32& ReferenceCount = CharacterStateReferenceCount.FindOrAdd(StateFlag, 0);

	// 기본 활성 상태라면 Remove 시 증가, Add 시 감소  
	if (bIsDefaultActive)
	{
		ReferenceCount += (Operation == ECharacterStateOperation::Add) ? -1 : 1;
	}
	else
	{
		ReferenceCount += (Operation == ECharacterStateOperation::Add) ? 1 : -1;
	}

	// 참조 카운트가 음수가 되지 않도록 보장
	ReferenceCount = FMath::Max(ReferenceCount, 0);

	const bool bShouldDeactivate = (bIsDefaultActive ? ReferenceCount == 1 : ReferenceCount == 0);
	const bool bShouldActivate = (bIsDefaultActive ? ReferenceCount == 0 : ReferenceCount == 1);

	FString StateFlagName = StaticEnum<ECharacterState>()->GetNameStringByValue(static_cast<int64>(StateFlag));

	if (bShouldDeactivate)
	{
		EnumRemoveFlags(CharacterState, StateFlag); // 상태 비활성화
	}
	else if (bShouldActivate)
	{
		EnumAddFlags(CharacterState, StateFlag); // 상태 활성화
	}
}

void ACharacterBase::OnRep_CharacterStateChanged()
{
}

void ACharacterBase::OnRep_CrowdControlStateChanged()
{

}


//==================== Delegate Functions ====================//


void ACharacterBase::OnMovementSpeedChanged(float InOldMS, float InNewMS)
{
	GetCharacterMovement()->MaxWalkSpeed = InNewMS;
}

void ACharacterBase::OnPreCalculateDamage(float& AttackDamage, float& AbilityPower)
{
}

void ACharacterBase::OnPreDamageReceived(float FinalDamage)
{
}

//==================== Ability Execution Functions ====================//


void ACharacterBase::ActionStarted(EActionSlot SlotID)
{
	if (!::IsValid(this))
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid 'this' pointer. The object is not valid or has been destroyed."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	switch (SlotID)
	{
	case EActionSlot::Q:
		Q_Started();
		break;
	case EActionSlot::E:
		E_Started();
		break;
	case EActionSlot::R:
		R_Started();
		break;
	case EActionSlot::LMB:
		LMB_Started();
		break;
	case EActionSlot::RMB:
		RMB_Started();
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid SlotID: %d. Unable to execute action."), ANSI_TO_TCHAR(__FUNCTION__), static_cast<int32>(SlotID));
		return;
	}
}



void ACharacterBase::ActionReleased(EActionSlot SlotID)
{
	if (!::IsValid(this))
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid 'this' pointer. The object is not valid or has been destroyed."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	switch (SlotID)
	{
	case EActionSlot::Q:
		Q_Released();
		break;
	case EActionSlot::E:
		E_Released();
		break;
	case EActionSlot::R:
		R_Released();
		break;
	case EActionSlot::LMB:
		LMB_Released();
		break;
	case EActionSlot::RMB:
		RMB_Released();
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid SlotID: %d. Unable to execute action."), ANSI_TO_TCHAR(__FUNCTION__), static_cast<int32>(SlotID));
		return;
	}
}

void ACharacterBase::ExecuteAction(EActionSlot SlotID)
{
	if (::IsValid(this) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid 'this' pointer. The object is not valid or has been destroyed."), ANSI_TO_TCHAR(__FUNCTION__));
		return;
	}

	switch (SlotID)
	{
	case EActionSlot::Q:
		Q_Executed();
		break;
	case EActionSlot::E:
		E_Executed();
		break;
	case EActionSlot::R:
		R_Executed();
		break;
	case EActionSlot::LMB:
		LMB_Executed();
		break;
	case EActionSlot::RMB:
		RMB_Executed();
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("[%s] Invalid SlotID: %d. Unable to execute action."), ANSI_TO_TCHAR(__FUNCTION__), static_cast<int32>(SlotID));
		return;
	}
}

