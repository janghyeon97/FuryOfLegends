// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Item.h"
#include "HealingPotion.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API AHealingPotion : public AItem
{
	GENERATED_BODY()
	
public:
	AHealingPotion();

protected:
	virtual void Initialize() override; 
	virtual void Use(class AArenaPlayerState* PlayerState) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealingPotion", meta = (AllowPrivateAccess))
	float HealingAmount; // 총 회복량

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealingPotion", meta = (AllowPrivateAccess))
	float HealingDuration; // 회복 지속 시간

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealingPotion", meta = (AllowPrivateAccess))
	float AccumulatedHealing; // 누적 회복량

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealingPotion", meta = (AllowPrivateAccess))
	float TotalElapsedTime; // 타이머 총 경과 시간 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealingPotion", meta = (AllowPrivateAccess))
	float ElapsedTime; // 타이머 경과 시간 0.5 초 마다 반복

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealingPotion", meta = (AllowPrivateAccess))
	float RemainingTime; // 타이머 남은 시간
};
