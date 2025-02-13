// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/GuardianAngel.h"
#include "Game/ArenaPlayerState.h"
#include "Characters/AOSCharacterBase.h"
#include "Components/StatComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Plugins/UniqueCodeGenerator.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"


AGuardianAngel::AGuardianAngel()
{
	ParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystem"));

	CooldownTime = 0.f;
}

void AGuardianAngel::Initialize()
{
    Super::Initialize();

	CooldownTime = UniqueAttributes.Contains("CooldownTime") ? UniqueAttributes["CooldownTime"] : 280.f;

    UE_LOG(LogTemp, Log, TEXT("[%s] Initialized - CooldownTime: %.2f"), *GetName(), CooldownTime);
}

void AGuardianAngel::Use(AArenaPlayerState* PlayerState)
{
	Super::Use(PlayerState);

}

void AGuardianAngel::BindToPlayer(AAOSCharacterBase* Character)
{
	Super::BindToPlayer(Character);

	if (::IsValid(Character) == false)
	{
		return;
	}

	Character->OnPreDeathEvent.AddDynamic(this, &ThisClass::ReviveCharacter);
}

void AGuardianAngel::ReviveCharacter(bool& bDeath)
{
    if (!OwnerCharacter.IsValid() || !::IsValid(OwnerCharacter->GetPlayerState<AArenaPlayerState>()))
    {
        return;
    }

    AArenaPlayerState* PlayerState = OwnerCharacter->GetPlayerState<AArenaPlayerState>();

    // 부활 처리
    OwnerCharacter->OnPreDeathEvent.RemoveDynamic(this, &ThisClass::ReviveCharacter);
    bDeath = false;

    UCapsuleComponent* CapsuleComponent = OwnerCharacter->GetCapsuleComponent();
    if (CapsuleComponent)
    {
        CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }


    OwnerCharacter->ServerModifyCharacterState(ECharacterStateOperation::Add, ECharacterState::Invulnerability);
    OwnerCharacter->SetActorHiddenInGame(true);
    OwnerCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);

    SpawnReviveEffect();

    uint32 UniqueCode = UUniqueCodeGenerator::GenerateUniqueCode(
        OwnerCharacter->ObjectType,
        static_cast<uint8>(PlayerState->GetPlayerIndex()),
        ETimerCategory::Item,
        static_cast<uint8>(ETimerType::Inventory),
        static_cast<uint8>(ItemCode)
    );


    auto Callback = [this, UniqueCode, WeakPlayerState = TWeakObjectPtr<AArenaPlayerState>(PlayerState)]()
        {
            if (OwnerCharacter.IsValid())
            {
                OwnerCharacter->OnPreDeathEvent.AddDynamic(this, &ThisClass::ReviveCharacter);
            }

            ActivationState = EItemActivationState::Expired;
        };

 
    ActivationState = EItemActivationState::Active;
    PlayerState->SetTimer(UniqueCode, Callback, CooldownTime, false, CooldownTime, true);
}



void AGuardianAngel::SpawnReviveEffect()
{
    if (!::IsValid(ReviveParticle) || !ReviveParticle->IsValidLowLevelFast())
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SpawnReviveEffect failed: ReviveParticle is invalid or not loaded correctly!"), *GetName());
        return;
    }

    if (!OwnerCharacter.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SpawnReviveEffect failed: OwnerCharacter is invalid!"), *GetName());
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] SpawnReviveEffect failed: GetWorld() returned nullptr!"), *GetName());
        return;
    }


    FVector SpawnLocation = OwnerCharacter->GetActorLocation() - FVector(0, 0, 95.f);
    Transform.SetRotation(FRotator(1).Quaternion());
    Transform.SetLocation(SpawnLocation);

    UParticleSystemComponent* PSC = OwnerCharacter->SpawnEmitterAtLocation(ReviveParticle, Transform);
    if (!PSC)
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to spawn ReviveParticle!"), *GetName());
        return;
    }

    PSC->OnSystemFinished.AddDynamic(this, &AGuardianAngel::OnParticleSystemFinished);
    PSC->Activate();
}




void AGuardianAngel::OnParticleSystemFinished(UParticleSystemComponent* PSystem)
{
    if (!PSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("OnParticleSystemFinished: ParticleSystemComponent is null!"));
        return;
    }

    if (PSystem->Template == ReviveParticle)
    {
        PSystem->OnSystemFinished.RemoveDynamic(this, &AGuardianAngel::OnParticleSystemFinished);
    }

    // ReviveEndedParticle이 설정되지 않은 경우
    if (!ReviveEndedParticle)
    {
        UE_LOG(LogTemp, Error, TEXT("OnParticleSystemFinished: ReviveEndedParticle is not set!"));
        return;
    }

    // ReviveEndedParticle로 변경
    OwnerCharacter->SpawnEmitterAtLocation(ReviveEndedParticle, Transform);
    OwnerCharacter->ServerModifyCharacterState(ECharacterStateOperation::Remove, ECharacterState::Invulnerability);

    UCapsuleComponent* CapsuleComponent = OwnerCharacter->GetCapsuleComponent();
    if (CapsuleComponent)
    {
        CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    // OwnerCharacter가 유효한 경우 리스폰
    if (OwnerCharacter.IsValid())
    {
        OwnerCharacter->Respawn(0.5f);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("OnParticleSystemFinished: OwnerCharacter is not valid!"));
    }
}



