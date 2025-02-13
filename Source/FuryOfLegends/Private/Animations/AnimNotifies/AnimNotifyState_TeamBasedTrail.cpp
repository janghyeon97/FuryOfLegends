// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/AnimNotifies/AnimNotifyState_TeamBasedTrail.h"
#include "Characters/CharacterBase.h"
#include "Kismet/GameplayStatics.h"


void UAnimNotifyState_TeamBasedTrail::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp == nullptr)
	{
		return;
	}

	ACharacterBase* OwnerCharacter = Cast<ACharacterBase>(MeshComp->GetOwner());
	if(OwnerCharacter == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("NotifyBegin: Owner is not of type ACharacterBase"));
		return;
	}

	ACharacterBase* LocalPlayerCharacter = Cast<ACharacterBase>(UGameplayStatics::GetPlayerCharacter(OwnerCharacter->GetWorld(), 0));
	if (LocalPlayerCharacter == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("NotifyBegin: LocalPlayerCharacter is null"));
		return;
	}

    // 팀에 따라 Trail 파티클 설정
    UParticleSystem* SelectedParticle = nullptr;
	SelectedParticle = OwnerCharacter->TeamSide == LocalPlayerCharacter->TeamSide ? TrailParticle_Ally : TrailParticle_Enemy;

    // 선택된 파티클이 유효하지 않은 경우 로그를 남기고 종료
    if (SelectedParticle == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("NotifyBegin: No valid particle found for the team"));
        return;
    }

    PSTemplate = SelectedParticle;
    Super::NotifyBegin(MeshComp, Animation, TotalDuration);
}

void UAnimNotifyState_TeamBasedTrail::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

}

void UAnimNotifyState_TeamBasedTrail::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation);

}
