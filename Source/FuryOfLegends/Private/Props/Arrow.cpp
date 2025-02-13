// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/Arrow.h"
#include "Components/BoxComponent.h"
#include "Characters/AOSCharacterBase.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Structs/SessionInfomation.h"

AArrow::AArrow()
{
	PrimaryActorTick.bCanEverTick = true;

	HitWorldEffect = nullptr;
	HitPlayerEffect = nullptr;
}

void AArrow::BeginPlay()
{
	if (!Owner) return;

	OwnerCharacter = Cast<AAOSCharacterBase>(Owner);
	if (!OwnerCharacter.IsValid()) return;

	// 클라이언트 측에서만 파티클 로드 및 설정
	if (!HasAuthority())
	{
		UParticleSystem* TrailParticle = OwnerCharacter->GetOrLoadParticle(TEXT("Primary_Attack"), TEXT("/Game/ParagonSparrow/FX/Particles/Sparrow/Abilities/Primary/FX/P_Sparrow_PrimaryAttack.P_Sparrow_PrimaryAttack"));
		UParticleSystem* HitWorldParticle = OwnerCharacter->GetOrLoadParticle(TEXT("Primary_HitWorld"), TEXT("/Game/ParagonSparrow/FX/Particles/Sparrow/Abilities/Primary/FX/P_Sparrow_Primary_Ballistic_HitWorld.P_Sparrow_Primary_Ballistic_HitWorld"));
		UParticleSystem* HitPlayerParticle = OwnerCharacter->GetOrLoadParticle(TEXT("Primary_HitPlayer"), TEXT("/Game/ParagonSparrow/FX/Particles/Sparrow/Abilities/Primary/FX/P_Sparrow_Primary_Ballistic_HitPlayer.P_Sparrow_Primary_Ballistic_HitPlayer"));

		if (!TrailParticle || !HitWorldParticle || !HitPlayerParticle)
		{
			UE_LOG(LogTemp, Warning, TEXT("[AArrow::BeginPlay] Failed to load one or more particles."));
			return;
		}

		ArrowTrailParticleSystem->Template = TrailParticle;
		HitWorldEffect = HitWorldParticle;
		HitPlayerEffect = HitPlayerParticle;

		// 파티클 종료 이벤트 바인딩
		ArrowImpactParticleSystem->OnSystemFinished.AddDynamic(this, &ThisClass::OnParticleEnded);
	}

	Super::BeginPlay();
}


void AArrow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AArrow::OnHitWorld(const FHitResult& HitResult)
{
	// 상위 클래스에서 이미 유효성 검사를 완료했으므로, 여기서 추가 검사는 필요 없음
	ECollisionChannel CollisionChannel = HitResult.GetComponent()->GetCollisionObjectType();

	StopArrow();
	MulticastPlayImpactEffects(CollisionChannel, HitResult.Location);
}

void AArrow::OnHitCharacter(const FHitResult& HitResult)
{
	AActor* HitActor = HitResult.GetActor();
	if (::IsValid(HitActor) == false)
	{
		return;
	}

	AActor* TargetActor = ArrowProperties.TargetActor.Get();
	if (::IsValid(TargetActor) == false)
	{
		return;
	}

	if (HitActor != TargetActor)
	{
		return;
	}

	ACharacterBase* HitCharacter = Cast<ACharacterBase>(HitActor);
	if (::IsValid(HitCharacter) == false)
	{
		return;
	}

	bShouldSweep = false;
	ECollisionChannel CollisionChannel = HitResult.GetComponent()->GetCollisionObjectType();

	StopArrow();
	AttachToClosestBone(HitActor);
	MulticastPlayImpactEffects(CollisionChannel, HitResult.Location);

	if (TeamSide != HitCharacter->TeamSide)
	{
		ApplyDamage(HitCharacter);
		return;
	}
}


void AArrow::MulticastPlayImpactEffects_Implementation(ECollisionChannel CollisionChannel, const FVector HitLocation)
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

void AArrow::OnParticleEnded(UParticleSystemComponent* ParticleSystemComponent)
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