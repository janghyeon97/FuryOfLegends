// Fill out your copyright notice in the Description page of Project Settings.


#include "Props/FreezeSegment.h"
#include "Characters/CharacterBase.h"
#include "Characters/AOSCharacterBase.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"

AFreezeSegment::AFreezeSegment()
{
	bReplicates = true;

	PrimaryActorTick.bCanEverTick = false;

	FreezeSegment = nullptr;

	Radius = 440.f;
	NumParicles = 28;
	Lifetime = 4;
	Rate = 0.01;
	Scale = 0.8f;
}

void AFreezeSegment::BeginPlay()
{
	Super::BeginPlay();
	
	if (!Owner) return;

	OwnerCharacter = Cast<AAOSCharacterBase>(Owner);
	if (!OwnerCharacter.IsValid()) return;

	Angle = 360.f / static_cast<float>(NumParicles);

	LastActorLocation = GetActorLocation();
	LastActorForwardVector = GetActorForwardVector();
	LastActorUpVector = GetActorUpVector();

	if (HasAuthority())
	{
		HandleServer();
	}
	else
	{
		HandleClient();
	}
}

void AFreezeSegment::HandleServer()
{
	for (int32 i = 0; i < NumParicles; ++i)
	{
		UBoxComponent* BoxComponent = NewObject<UBoxComponent>(this);
		if (!BoxComponent) continue;

		BoxComponent->SetupAttachment(RootComponent);
		BoxComponent->SetBoxExtent(FVector(50 * Scale));
		BoxComponent->bHiddenInGame = false;

		// 같은 AFreezeSegment 내의 다른 BoxComponent와 충돌하지 않도록 설정
		BoxComponent->IgnoreActorWhenMoving(this, true);

		BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore); // 모든 채널을 기본적으로 무시

		BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AFreezeSegment::OnBeginOverlap);
		BoxComponent->RegisterComponent();

		CollisionBoxes.Add(BoxComponent);
	}

	// 한 번에 모든 파티클 위치 계산 및 클라이언트에 전송
	TArray<FVector> ParticleLocations;
	TArray<FRotator> ParticleRotations;
	CalculateParticleLocations(ParticleLocations, ParticleRotations);
	MulticastPlayParticles(ParticleLocations, ParticleRotations, NumParicles, Scale, Rate, Lifetime);

	Iterator = 0;

	GetWorldTimerManager().SetTimer(ParticleTimer, [this, ParticleLocations, ParticleRotations]()
		{
			if (CollisionBoxes.IsValidIndex(Iterator))
			{
				CollisionBoxes[Iterator]->SetWorldLocationAndRotation(ParticleLocations[Iterator], ParticleRotations[Iterator]);
				CollisionBoxes[Iterator]->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				CollisionBoxes[Iterator]->SetCollisionObjectType(Detection);
				CollisionBoxes[Iterator]->SetCollisionResponseToAllChannels(ECR_Ignore);
				CollisionBoxes[Iterator]->SetGenerateOverlapEvents(true);

				//DetectPlayer
				if (Detection == ECC_GameTraceChannel4)
				{
					CollisionBoxes[Iterator]->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
				}
				//DetectCharacter
				else if (Detection == ECC_GameTraceChannel5)
				{
					CollisionBoxes[Iterator]->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
					CollisionBoxes[Iterator]->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);
				}
				//DetectObject
				else if (Detection == ECC_GameTraceChannel7)
				{
					CollisionBoxes[Iterator]->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
					CollisionBoxes[Iterator]->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Overlap);
					CollisionBoxes[Iterator]->SetCollisionResponseToChannel(ECC_GameTraceChannel6, ECR_Overlap);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid index for CollisionBoxes: %d"), Iterator);
			}

			Iterator++;

			if (Iterator >= NumParicles)
			{
				GetWorldTimerManager().ClearTimer(ParticleTimer);
			}
		},
		Rate,
		true,
		-1.0f
	);
	GetWorldTimerManager().SetTimer(ParticleEndTimer, [this]() {OnParticleEnded(); }, 1.0f, false, NumParicles * Rate + Lifetime);
}

void AFreezeSegment::HandleClient()
{
	FreezeSegment = OwnerCharacter->GetOrLoadParticle(TEXT("FreezeSegment"), TEXT("/Game/ParagonAurora/FX/Particles/Abilities/Freeze/FX/P_Aurora_Freeze_Segment.P_Aurora_Freeze_Segment"));
	if (FreezeSegment == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AFreezeSegment::BeginPlay] Failed to set the particle FreezeSegment."));
		return;
	}
}


void AFreezeSegment::CalculateParticleLocations(TArray<FVector>& OutLocations, TArray<FRotator>& OutRotations)
{
	for (int32 i = 0; i < NumParicles; ++i)
	{
		const float AngleRad = FMath::DegreesToRadians(i * Angle);
		FVector SpawnLocation = LastActorLocation + LastActorForwardVector.RotateAngleAxis(i * Angle, LastActorUpVector) * Radius;
		FRotator SpawnRotation = LastActorForwardVector.Rotation();
		SpawnRotation.Yaw += i * Angle + 90;

		OutLocations.Add(SpawnLocation);
		OutRotations.Add(SpawnRotation);
	}
}

// Multicast RPC를 통해 클라이언트에서 파티클 재생
void AFreezeSegment::MulticastPlayParticles_Implementation(const TArray<FVector>& SpawnLocations, const TArray<FRotator>& SpawnRotations, const int32 InNumParicles, const float ParticleScale, const float InRate, const float InLifetime)
{
	if (HasAuthority()) return; // 서버에서는 파티클 재생 안 함

	if (FreezeSegment == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AFreezeSegment::MulticastPlayParticles] Particle template not set."));
		return;
	}

	// 위치 및 회전값이 유효한지 확인
	if (SpawnLocations.Num() != SpawnRotations.Num() || SpawnLocations.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AFreezeSegment::MulticastPlayParticles] Invalid SpawnLocations or SpawnRotations."));
		return;
	}

	Iterator = 0;
	Rate = InRate;
	Lifetime = InLifetime;
	Scale = ParticleScale;
	NumParicles = InNumParicles;

	GetWorldTimerManager().SetTimer(
		ParticleTimer,
		[this, SpawnLocations, SpawnRotations]()
		{
			if (!FreezeSegment)
			{
				GetWorldTimerManager().ClearTimer(ParticleTimer);
				return;
			}

			UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FreezeSegment, SpawnLocations[Iterator], SpawnRotations[Iterator], FVector(Scale), true, EPSCPoolMethod::AutoRelease, false);
			if (PSC)
			{
				PSC->SetFloatParameter(FName("AbilityDuration"), (NumParicles - Iterator - 1) * Rate + Lifetime);
				PSC->SetFloatParameter(FName("DurationRange"), (NumParicles - Iterator - 1) * Rate + Lifetime);
				PSC->SetRelativeScale3D(FVector(Scale));
				PSC->Activate();
			}

			Iterator++;

			if (Iterator >= NumParicles)
			{
				GetWorldTimerManager().ClearTimer(ParticleTimer);
			}

		},
		Rate,
		true,
		0.0f
	);
}

void AFreezeSegment::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetOwner())
	{
		return;
	}

	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("AFreezeSegment OnBeginOverlap: %s"), *OtherActor->GetName()), true, true, FLinearColor::Red, 2.0f);

	if (ProcessedActors.Contains(OtherActor))
	{
		return;
	}

	ECollisionChannel CollisionChannel = OtherComp->GetCollisionObjectType();
	if (CollisionChannel == ECC_WorldDynamic || CollisionChannel == ECC_WorldStatic)
	{
		return;
	}

	ACharacterBase* Character = Cast<ACharacterBase>(OtherActor);
	if (!Character)
	{
		return;
	}

	if (OwnerCharacter->TeamSide == Character->TeamSide)
	{
		return;
	}

	ProcessedActors.Add(OtherActor);
	ApplyDamage(OtherActor);
}

void AFreezeSegment::ApplyDamage(AActor* OtherActor)
{
	ACharacterBase* Character = Cast<ACharacterBase>(OtherActor);
	if (::IsValid(Character))
	{
		OwnerCharacter->ServerApplyDamage(Character, OwnerCharacter.Get(), OwnerCharacter->GetController(), DamageInformation);
	}
}


void AFreezeSegment::OnParticleEnded()
{
	Destroy();
}