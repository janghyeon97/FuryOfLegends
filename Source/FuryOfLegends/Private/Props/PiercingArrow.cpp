// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/PiercingArrow.h"
#include "Components/BoxComponent.h"
#include "Characters/CharacterBase.h"
#include "Characters/AOSCharacterBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Structs/SessionInfomation.h"

APiercingArrow::APiercingArrow()
{
	PrimaryActorTick.bCanEverTick = true;

	ArrowProjectileMovement->ProjectileGravityScale = 0.0f;
}

void APiercingArrow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
}

void APiercingArrow::BeginPlay()
{
	Super::BeginPlay();

	if (!Owner) return;

	OwnerCharacter = Cast<AAOSCharacterBase>(Owner);
	if (!OwnerCharacter.IsValid()) return;

	if (!HasAuthority())
	{
		UParticleSystem* TrailParticle = OwnerCharacter->GetOrLoadParticle(TEXT("DrawBack_Arrow"), TEXT("/Game/ParagonSparrow/FX/Particles/Sparrow/Abilities/DrawABead/FX/P_Sparrow_RMB.P_Sparrow_RMB"));
		UParticleSystem* HitWorldParticle = OwnerCharacter->GetOrLoadParticle(TEXT("Primary_HitWorld"), TEXT("/Game/ParagonSparrow/FX/Particles/Sparrow/Abilities/Primary/FX/P_Sparrow_Primary_Ballistic_HitWorld.P_Sparrow_Primary_Ballistic_HitWorld"));
		UParticleSystem* HitPlayerParticle = OwnerCharacter->GetOrLoadParticle(TEXT("Primary_HitPlayer"), TEXT("/Game/ParagonSparrow/FX/Particles/Sparrow/Abilities/Primary/FX/P_Sparrow_Primary_Ballistic_HitPlayer.P_Sparrow_Primary_Ballistic_HitPlayer"));

		ArrowImpactParticleSystem->OnSystemFinished.AddDynamic(this, &ThisClass::OnParticleEnded);

		if (!TrailParticle)
		{
			UE_LOG(LogTemp, Warning, TEXT("[AArrow::BeginPlay] Failed to load Particle_Arrow"));
			return;
		}

		if (!HitWorldParticle)
		{
			UE_LOG(LogTemp, Warning, TEXT("[AArrow::BeginPlay] Failed to load Particle_HitWorld"));
			return;
		}

		if (!HitPlayerParticle)
		{
			UE_LOG(LogTemp, Warning, TEXT("[AArrow::BeginPlay] Failed to load Particle_HitPlayer"));
			return;
		}

		ArrowTrailParticleSystem->Template = TrailParticle;
		HitWorldEffect = HitWorldParticle;
		HitPlayerEffect = HitPlayerParticle;
	}
}

void APiercingArrow::OnHitWorld(const FHitResult& HitResult)
{
	// 상위 클래스에서 이미 유효성 검사를 완료했으므로, 여기서 추가 검사는 필요 없음
	ECollisionChannel CollisionChannel = HitResult.GetComponent()->GetCollisionObjectType();

	StopArrow();
	MulticastPlayImpactEffects(CollisionChannel, HitResult.Location, PierceCount, ArrowProperties.MaxPierceCount);
}

void APiercingArrow::OnHitCharacter(const FHitResult& HitResult)
{
	ACharacterBase* HitCharacter = Cast<ACharacterBase>(HitResult.GetActor());
	if (::IsValid(HitCharacter) == false || OwnerCharacter.IsValid() == false)
	{
		return;
	}

	if (TeamSide == HitCharacter->TeamSide)
	{
		return;
	}

	if (PierceCount >= ArrowProperties.MaxPierceCount)
	{
		StopArrow();
	}

	ECollisionChannel CollisionChannel = HitResult.GetComponent()->GetCollisionObjectType();
	MulticastPlayImpactEffects(CollisionChannel, HitResult.Location, PierceCount, ArrowProperties.MaxPierceCount);
	ApplyDamage(HitCharacter);

	PierceCount++;
}



void APiercingArrow::MulticastPlayImpactEffects_Implementation(ECollisionChannel CollisionChannel, const FVector HitLocation, const int32 CurrentPierceCount, const int32 MaxPierceCount)
{
	if (HasAuthority())
	{
		return;
	}

	bool IsDeactivateTrailEffect = CollisionChannel == ECC_WorldStatic || CollisionChannel == ECC_WorldDynamic || CurrentPierceCount >= MaxPierceCount;
	if (ArrowTrailParticleSystem && ArrowStaticMesh && IsDeactivateTrailEffect)
	{
		bShouldDestroy = true;
		ArrowStaticMesh->SetVisibility(false);
		ArrowTrailParticleSystem->Deactivate();
	}

	UParticleSystem* SelectedParticleSystem = nullptr;
	if (CollisionChannel == ECC_WorldStatic || CollisionChannel == ECC_WorldDynamic)
	{
		SelectedParticleSystem = HitWorldEffect;
	}
	else
	{
		SelectedParticleSystem = HitPlayerEffect;
	}

	if (SelectedParticleSystem)
	{
		FRotator SpawnRotation = GetActorRotation();
		SpawnRotation.Pitch += -180.f;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedParticleSystem, FTransform(SpawnRotation, HitLocation, FVector(1)), true, EPSCPoolMethod::AutoRelease, true);
	}
}

void APiercingArrow::OnParticleEnded(UParticleSystemComponent* ParticleSystemComponent)
{
	Super::OnParticleEnded(ParticleSystemComponent);

	if (HasAuthority())
	{
		return;
	}

	if (!bShouldDestroy)
	{
		return;
	}

	if (OwnerCharacter.IsValid() == false)
	{
		return;
	}

	AController* ClientController = OwnerCharacter->GetController();
	if (ClientController)
	{
		ServerNotifyParticleFinished(ClientController);
	}
}