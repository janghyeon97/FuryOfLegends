// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/Projectile.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Characters/CharacterBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

AProjectile::AProjectile()
{
	bReplicates = true;
	SetReplicateMovement(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;

	DefaultRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRootComponent"));
	DefaultRootComponent->SetupAttachment(GetRootComponent());
	RootComponent = DefaultRootComponent;

	TrailParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TrailParticleSystem"));
	TrailParticleSystem->SetupAttachment(DefaultRootComponent);
	TrailParticleSystem->SetRelativeLocationAndRotation(FVector(0.f, 0.f, 0.f), FRotator(0.f, 0.f, 0.f));
	TrailParticleSystem->SetRelativeScale3D(FVector(0.4f, 0.4f, 0.4f));
	TrailParticleSystem->SetAutoActivate(false);
	TrailParticleSystem->bAutoDestroy = true;
	TrailParticleSystem->SetIsReplicated(true);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetAutoActivate(false);
	ProjectileMovement->InitialSpeed = 0;
	ProjectileMovement->MaxSpeed = 0;
	ProjectileMovement->bForceSubStepping = true;
	ProjectileMovement->bSweepCollision = true;
	ProjectileMovement->MaxSimulationTimeStep = 0.0166f;

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxCollision->SetupAttachment(DefaultRootComponent);
	BoxCollision->SetBoxExtent(FVector(10, 10, 10));
	BoxCollision->SetRelativeLocation(FVector(20, 0, 0));
	BoxCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TargetActor = nullptr;
	ProjectileInteractionType = EProjectileInteractionType::None;
}

void AProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBeginOverlap);
		BoxCollision->SetCollisionProfileName(FName(TEXT("Projectile")));
		BoxCollision->SetGenerateOverlapEvents(true);
	}

	ProjectileMovement->Activate();
}


void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && !::IsValid(TargetActor))
	{
		Destroy();
	}
}


void AProjectile::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == Owner || OtherActor != TargetActor)
	{
		return;
	}

	ACharacterBase* OwnerCharacter = Cast<ACharacterBase>(Owner);
	if (::IsValid(OwnerCharacter) == false)
	{
		return;
	}

	ACharacterBase* Enemy = Cast<ACharacterBase>(OtherActor);
	if (::IsValid(Enemy) == false)
	{
		return;
	}

	AController* EventInstigator = OwnerCharacter->GetController();

	OwnerCharacter->ServerApplyDamage(Enemy, OwnerCharacter, EventInstigator ? EventInstigator : nullptr, DamageInformation);
	Destroy(true, true);
}


void AProjectile::MulticastConfigureProjectile_Implementation(UParticleSystem* TrailEffect, AActor* InTarget, bool bIsHomingProjectile, float HomingAcceleration, float InitialSpeed, float MaxSpeed)
{
	if (TrailParticleSystem)
	{
		TrailParticleSystem->Template = TrailEffect;
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->bIsHomingProjectile = bIsHomingProjectile;
		ProjectileMovement->HomingTargetComponent = InTarget ? InTarget->GetRootComponent() : nullptr;
		ProjectileMovement->HomingAccelerationMagnitude = HomingAcceleration;
		ProjectileMovement->InitialSpeed = InitialSpeed;
		ProjectileMovement->MaxSpeed = MaxSpeed;
	}
}
