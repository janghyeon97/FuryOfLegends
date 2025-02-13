// Fill out your copyright notice in the Description page of Project Settings.

#include "Props/ArrowBase.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Characters/AOSCharacterBase.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Game/ArenaGameState.h"

// --------------------------------------------------
// 1. ������ �� �ʱ�ȭ ���� �Լ�
// --------------------------------------------------

// Constructor
AArrowBase::AArrowBase()
{
	bReplicates = true;
	SetReplicateMovement(true);
	PrimaryActorTick.bCanEverTick = true;

	// Components setup
	DefaultRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRootComponent"));
	DefaultRootComponent->SetupAttachment(GetRootComponent());

	ArrowStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMeshComponent"));
	ArrowStaticMesh->SetupAttachment(DefaultRootComponent);
	ArrowStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArrowStaticMesh->SetRelativeLocationAndRotation(FVector(70.f, 0.f, 0.f), FRotator(90.f, 0.f, 0.f));

	ArrowProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ArrowProjectileMovement->SetAutoActivate(false);
	ArrowProjectileMovement->InitialSpeed = 0;
	ArrowProjectileMovement->MaxSpeed = 0;
	ArrowProjectileMovement->bForceSubStepping = true;
	ArrowProjectileMovement->bSweepCollision = true;
	ArrowProjectileMovement->MaxSimulationTimeStep = 0.0166f;
	ArrowProjectileMovement->bIsHomingProjectile = false;

	ArrowImpactParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ImpactParticleSystem"));
	ArrowImpactParticleSystem->SetupAttachment(ArrowStaticMesh);
	ArrowImpactParticleSystem->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -20.f), FRotator(90.f, 0.f, 0.f));
	ArrowImpactParticleSystem->SetAutoActivate(false);
	ArrowImpactParticleSystem->bAutoDestroy = true;
	ArrowImpactParticleSystem->SetIsReplicated(true);

	ArrowTrailParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TrailParticleSystem"));
	ArrowTrailParticleSystem->SetupAttachment(ArrowStaticMesh);
	ArrowTrailParticleSystem->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -20.f), FRotator(-90.f, 0.f, 0.f));
	ArrowTrailParticleSystem->SetAutoActivate(false);
	ArrowTrailParticleSystem->bAutoDestroy = true;
	ArrowTrailParticleSystem->SetIsReplicated(true);

	//----------------------------------------------------------------------------------------------

	ClientParticleFinishedCount = 0;
	PierceCount = 0;
	OwnerLocation = FVector(0);
	IgnoredActors = TArray<AActor*>();
	CompletedClients = TArray<AController*>();
	bShouldDestroy = false;

	ArrowProperties = FArrowProperties();
	DamageInformation = FDamageInformation();
}

// Replication setup
void AArrowBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ArrowProperties);
}

// Initialization function
void AArrowBase::InitializeArrow(const FArrowProperties& InArrowProperties, const FDamageInformation& InDamageInformation)
{
	ArrowProperties = InArrowProperties;
	DamageInformation = InDamageInformation;
	IgnoredActors.Empty();

	ArrowProjectileMovement->InitialSpeed = ArrowProperties.InitialSpeed;
	ArrowProjectileMovement->MaxSpeed = ArrowProperties.MaxSpeed;

	if (ArrowProperties.bIsHoming && ArrowProperties.TargetActor.IsValid())
	{
		ArrowProjectileMovement->bIsHomingProjectile = true;
		ArrowProjectileMovement->HomingTargetComponent = ArrowProperties.TargetActor->GetRootComponent();
		ArrowProjectileMovement->HomingAccelerationMagnitude = ArrowProperties.HomingAcceleration;
	}

	MulticastConfigureProjectile(ArrowProperties);
}

void AArrowBase::MulticastConfigureProjectile_Implementation(const FArrowProperties& InArrowProperties)
{
	if (HasAuthority())
	{
		return;
	}

	ArrowProjectileMovement->InitialSpeed = ArrowProperties.InitialSpeed;
	ArrowProjectileMovement->MaxSpeed = ArrowProperties.MaxSpeed;

	if (ArrowProperties.bIsHoming && ArrowProperties.TargetActor.IsValid())
	{
		ArrowProjectileMovement->bIsHomingProjectile = true;
		ArrowProjectileMovement->HomingTargetComponent = ArrowProperties.TargetActor->GetRootComponent();
		ArrowProjectileMovement->HomingAccelerationMagnitude = ArrowProperties.HomingAcceleration;
	}
}

void AArrowBase::BeginPlay()
{
	Super::BeginPlay();

	if (!Owner) return;

	OwnerCharacter = Cast<ACharacterBase>(Owner);
	if (!OwnerCharacter.IsValid()) return;

	// ĳ������ �ʱ� ��ġ ����
	OwnerLocation = OwnerCharacter->GetActorLocation();
	TeamSide = OwnerCharacter->TeamSide;

	if (!HasAuthority())
	{
		UStaticMesh* StaticMesh = OwnerCharacter->GetOrLoadMesh(TEXT("Arrow"), TEXT("/Game/ParagonSparrow/FX/Meshes/Heroes/Sparrow/Abilities/SM_Sparrow_Arrow.SM_Sparrow_Arrow"));

		TArray<FName> MaterialSlotNames = OwnerCharacter->GetMesh()->GetMaterialSlotNames();
		TArray<UMaterialInterface*> MaterialInterfaces = OwnerCharacter->GetMesh()->GetMaterials();

		// "Arrow"��� �̸��� ���Ե� ������ ã��
		for (int32 SlotIndex = 0; SlotIndex < MaterialSlotNames.Num(); SlotIndex++)
		{
			FName SlotName = MaterialSlotNames[SlotIndex];
			FString SlotNameString = SlotName.ToString();

			if (SlotNameString.EndsWith(TEXT("_Arrow")))
			{
				UMaterialInterface* ArrowMaterial = MaterialInterfaces.IsValidIndex(SlotIndex) ? MaterialInterfaces[SlotIndex] : nullptr;
				if (ArrowMaterial)
				{
					ArrowStaticMesh->SetMaterial(0, ArrowMaterial);
				}
			}
		}

		if (::IsValid(StaticMesh))
		{
			ArrowStaticMesh->SetStaticMesh(StaticMesh);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[AArrowBase::BeginPlay] Failed to set the static mesh for Arrow."));
		}


		if (ArrowTrailParticleSystem)
		{
			ArrowTrailParticleSystem->Activate();
		}
	}

	if (ArrowProjectileMovement)
	{
		ArrowProjectileMovement->Activate();
	}
}



// --------------------------------------------------
// 2. Tick �� ������Ʈ ���� �Լ�
// --------------------------------------------------

// Tick function for projectile movement and collision
void AArrowBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() == false)
	{
		return;
	}

	// �ִ� ��Ÿ� üũ
	FVector ArrowLocation = GetActorLocation();
	if (ArrowProperties.MaxRange != 0)
	{
		float Distance = FVector::Dist2D(OwnerLocation, ArrowLocation);
		if (Distance >= ArrowProperties.MaxRange)
		{
			if (IsPendingKillPending())
			{
				UE_LOG(LogTemp, Warning, TEXT("Arrow is already being destroyed."));
				return;
			}

			Destroy(true); // ��ü �ı�
			return;

		}

		if (ArrowProperties.bIsHoming && ArrowProperties.TargetActor.IsValid())
		{
			FVector TargetLocation = ArrowProperties.TargetActor->GetActorLocation();
			if (FVector::Dist(ArrowLocation, TargetLocation) <= 50.f)
			{
				if (IsPendingKillPending())
				{
					UE_LOG(LogTemp, Warning, TEXT("Arrow is already being destroyed."));
					return;
				}

				Destroy(true); // ��ü �ı�
				return;
			}
		}
	}


	// �浹 �˻�
	if (bShouldSweep && GetVelocity().SizeSquared() > 0.0f)
	{
		FVector ArrowForward = GetActorForwardVector();
		FVector StartLocation = ArrowLocation + (ArrowForward * 140.f);
		FVector EndLocation = StartLocation + (GetVelocity() * DeltaTime * 10);

		FHitResult HitResult;
		FCollisionQueryParams CollisionParams(NAME_None, false, this);
		CollisionParams.AddIgnoredActors(IgnoredActors);

		if (GetWorld()->SweepSingleByChannel(HitResult, StartLocation, EndLocation, FQuat::Identity, ArrowProperties.Detection, FCollisionShape::MakeSphere(ArrowProperties.CollisionRadius), CollisionParams))
		{
			FVector AdjustedLocation = HitResult.Location - (ArrowForward * 80.f);
			SetActorLocation(AdjustedLocation, true);
			OnArrowHit(HitResult);
			IgnoredActors.Add(HitResult.GetActor());
		}
	}
}




// --------------------------------------------------
// 3. �浹 �� ��Ʈ ó�� �Լ�
// --------------------------------------------------

void AArrowBase::OnArrowHit(const FHitResult& HitResult)
{
	if (!HasAuthority()) return;

	AActor* HitActor = HitResult.GetActor();
	if (!HitActor || HitActor == GetOwner()) return;

	UPrimitiveComponent* HitComponent = HitResult.GetComponent();
	if (!HitComponent) return;

	ECollisionChannel CollisionChannel = HitComponent->GetCollisionObjectType();

	// �浹�� ������Ʈ�� Ÿ�Կ� ���� �ٸ� �Լ��� ȣ��
	if (CollisionChannel == ECC_WorldStatic || CollisionChannel == ECC_WorldDynamic)
	{
		OnHitWorld(HitResult);
	}
	else
	{
		OnHitCharacter(HitResult);
	}
}

void AArrowBase::OnHitWorld(const FHitResult& HitResult)
{
	// �� �浹 �� ó��
}

void AArrowBase::OnHitCharacter(const FHitResult& HitResult)
{
	// ĳ���� �浹 �� ó��
}




// --------------------------------------------------
// 4. ������ ó�� �Լ�
// --------------------------------------------------

void AArrowBase::ApplyDamage(AActor* OtherActor)
{
	ACharacterBase* Character = Cast<ACharacterBase>(OtherActor);
	if (::IsValid(Character))
	{
		if (EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::Physical))
		{
			DamageInformation.PhysicalDamage *= (1.0f - (PierceCount * ArrowProperties.DamageReductionPerPierce) / 100.0f);
		}
		if (EnumHasAnyFlags(DamageInformation.DamageType, EDamageType::Magic))
		{
			DamageInformation.MagicDamage *= (1.0f - (PierceCount * ArrowProperties.DamageReductionPerPierce) / 100.0f);
		}

		OwnerCharacter->ServerApplyDamage(Character, OwnerCharacter.Get(), OwnerCharacter->GetController(), DamageInformation);
	}
}




// --------------------------------------------------
// 5. ȭ�� ���� �� �ΰ� ��� ���� �Լ�
// --------------------------------------------------

void AArrowBase::MulticastLaunchArrow_Implementation()
{
	if (ArrowProjectileMovement)
	{
		ArrowProjectileMovement->Activate();
	}
}

void AArrowBase::StopArrow()
{
	if (ArrowProjectileMovement)
	{
		ArrowProjectileMovement->StopMovementImmediately();
		ArrowProjectileMovement->ProjectileGravityScale = 0.0f;
	}
}


void AArrowBase::AttachToClosestBone(AActor* InTargetActor)
{
	if (!InTargetActor) return;

	ACharacterBase* TargetCharacter = Cast<ACharacterBase>(InTargetActor);
	if (!TargetCharacter) return;

	USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(TargetCharacter->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
	if (!SkeletalMeshComponent) return;

	FVector CurrentLocation = GetActorLocation();
	FName ClosestBoneName;
	float ClosestDistanceSq = FLT_MAX;
	FVector ClosestBoneLocation;

	// ��� Bone�� ��ġ�� Ȯ���Ͽ� ���� ����� Bone�� ã��
	for (int32 i = 0; i < SkeletalMeshComponent->GetNumBones(); i++)
	{
		FName BoneName = SkeletalMeshComponent->GetBoneName(i);
		FVector BoneLocation = SkeletalMeshComponent->GetBoneLocation(BoneName);

		float DistanceSq = FVector::DistSquared(BoneLocation, CurrentLocation);

		// ���� ����� Bone�� ������Ʈ
		if (DistanceSq < ClosestDistanceSq)
		{
			ClosestDistanceSq = DistanceSq;
			ClosestBoneName = BoneName;
			ClosestBoneLocation = BoneLocation;
		}
	}

	if (ClosestBoneName.IsNone()) return;

	// ȭ���� ���� ����� Bone�� ����
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, true);
	SetActorLocation(ClosestBoneLocation, false, nullptr, ETeleportType::None);
	AttachToComponent(SkeletalMeshComponent, AttachmentRules, ClosestBoneName);
	//UE_LOG(LogTemp, Warning, TEXT("[Server] %s Attached to %s's Mesh at Bone: %s"), *GetName(), *TargetActor->GetName(), *ClosestBoneName.ToString());
}




// --------------------------------------------------
// 6. �ΰ����� ���
// --------------------------------------------------

TArray<AActor*> AArrowBase::DetectActorsInExplosionRadius()
{
	TArray<AActor*> AffectedActors;
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape ExplosionSphere = FCollisionShape::MakeSphere(ArrowProperties.ExplosionRadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->OverlapMultiByChannel(OverlapResults, GetActorLocation(), FQuat::Identity, ECC_GameTraceChannel5, ExplosionSphere, QueryParams))
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* OverlappedActor = Result.GetActor();
			if (OverlappedActor && OverlappedActor != GetOwner())
			{
				ACharacterBase* Character = Cast<ACharacterBase>(OverlappedActor);
				if (Character && OwnerCharacter.IsValid() && OwnerCharacter->TeamSide != Character->TeamSide)
				{
					AffectedActors.Add(OverlappedActor);
				}
			}
		}
	}

	return AffectedActors;
}



// --------------------------------------------------
// 7. ���� ���� �Լ�
// --------------------------------------------------

void AArrowBase::ServerNotifyParticleFinished_Implementation(AController* ClientController)
{
	if (!HasAuthority() || !ClientController)
	{
		return;
	}

	// ���� ����� Ŭ���̾�Ʈ ���� ������
	const int32 TotalClients = GetWorld()->GetGameState()->PlayerArray.Num();

	// �ߺ� ��ȣ ����: �̹� ó���� Ŭ���̾�Ʈ���� Ȯ��
	if (!CompletedClients.Contains(ClientController))
	{
		ClientParticleFinishedCount++;
		CompletedClients.Add(ClientController);
	}

	// ��� Ŭ���̾�Ʈ�� ��ƼŬ ����� �Ϸ��� ��� ȭ���� �ı�
	if (ClientParticleFinishedCount >= TotalClients)
	{
		Destroy();
	}
}
