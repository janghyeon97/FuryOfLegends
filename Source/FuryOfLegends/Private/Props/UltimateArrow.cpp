// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/UltimateArrow.h"
#include "Characters/CharacterBase.h"
#include "Characters/AOSCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Structs/CharacterData.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"

AUltimateArrow::AUltimateArrow()
{
	PrimaryActorTick.bCanEverTick = true;

	ArrowProjectileMovement->ProjectileGravityScale = 0.0f;
}

void AUltimateArrow::BeginPlay()
{
	Super::BeginPlay();

	if (!Owner) return;

	OwnerCharacter = Cast<AAOSCharacterBase>(Owner);
	if (!OwnerCharacter.IsValid()) return;

	if (!HasAuthority())
	{
		UParticleSystem* TrailParticle = OwnerCharacter->GetOrLoadParticle(TEXT("Ultimate_Arrow"), TEXT("/Game/ParagonSparrow/FX/Particles/Sparrow/Abilities/Ultimate/FX/P_Arrow_Ultimate.P_Arrow_Ultimate"));
		UParticleSystem* HitWorldParticle = OwnerCharacter->GetOrLoadParticle(TEXT("Ultimate_HitWorld"), TEXT("/Game/ParagonSparrow/FX/Particles/Sparrow/Abilities/Ultimate/FX/P_Sparrow_UltHitWorld.P_Sparrow_UltHitWorld"));
		UParticleSystem* HitPlayerParticle = OwnerCharacter->GetOrLoadParticle(TEXT("Ultimate_HitPlayer"), TEXT("/Game/Paragon/ParagonSparrow/FX/Particles/Sparrow/Abilities/Ultimate/FX/P_Sparrow_UltHit.P_Sparrow_UltHit"));

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

void AUltimateArrow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AUltimateArrow::OnHitWorld(const FHitResult& HitResult)
{
	// 상위 클래스에서 이미 유효성 검사를 완료했으므로, 여기서 추가 검사는 필요 없음
	ECollisionChannel CollisionChannel = HitResult.GetComponent()->GetCollisionObjectType();

	StopArrow();
	MulticastPlayImpactEffects(CollisionChannel, HitResult.Location);
}

void AUltimateArrow::OnHitCharacter(const FHitResult& HitResult)
{
	// 상위 클래스에서 이미 유효성 검사를 완료했으므로, 여기서 추가 검사는 필요 없음
	ECollisionChannel CollisionChannel = HitResult.GetComponent()->GetCollisionObjectType();

	ACharacterBase* HitCharacter = Cast<ACharacterBase>(HitResult.GetActor());
	if (TeamSide == HitCharacter->TeamSide)
	{
		return;
	}

	StopArrow();
	MulticastPlayImpactEffects(CollisionChannel, HitResult.Location);

	FCollisionQueryParams params;
	params.TraceTag = FName("Name_None");
	params.bTraceComplex = false;
	params.AddIgnoredActor(this);

	TArray<FOverlapResult> OutHits;
	bool bResult = GetWorld()->OverlapMultiByChannel(
		OutHits,
		GetActorLocation(),
		FQuat::Identity,
		ArrowProperties.Detection,
		FCollisionShape::MakeSphere(ArrowProperties.ExplosionRadius),
		params
	);

	for (const auto& OutHit : OutHits)
	{
		ACharacterBase* OverlapCharacter = Cast<ACharacterBase>(OutHit.GetActor());
		if (::IsValid(OverlapCharacter) == false)
		{
			continue;
		}

		if (OverlapCharacter->TeamSide == TeamSide)
		{
			continue;
		}

		UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Overlapped Actor: %s"), *OverlapCharacter->GetName()), true, true, FLinearColor::Red, 2.0f, NAME_None);
		ApplyDamage(OverlapCharacter);
	}
}


void AUltimateArrow::MulticastPlayImpactEffects_Implementation(ECollisionChannel CollisionChannel, const FVector HitLocation)
{
	if (HasAuthority())
	{
		return;
	}

	bShouldDestroy = true;
	ArrowStaticMesh->SetVisibility(false);
	ArrowTrailParticleSystem->Deactivate();

	UParticleSystem* SelectedParticleSystem = nullptr;
	if (CollisionChannel == ECC_GameTraceChannel1 || CollisionChannel == ECC_GameTraceChannel2)
	{
		SelectedParticleSystem = HitPlayerEffect;
	}
	else
	{
		SelectedParticleSystem = HitWorldEffect;
	}

	if (SelectedParticleSystem)
	{
		FRotator SpawnRotation = GetActorRotation();
		SpawnRotation.Pitch += -180.f;
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedParticleSystem, FTransform(SpawnRotation, HitLocation, FVector(1)), true, EPSCPoolMethod::AutoRelease, true);
	}
}


void AUltimateArrow::OnParticleEnded(UParticleSystemComponent* ParticleSystemComponent)
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