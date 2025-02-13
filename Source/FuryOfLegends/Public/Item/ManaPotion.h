// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Item.h"
#include "ManaPotion.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API AManaPotion : public AItem
{
	GENERATED_BODY()
	
public:
	AManaPotion();

protected:
	virtual void Initialize() override;
	virtual void Use(class AArenaPlayerState* PlayerState) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealingPotion", meta = (AllowPrivateAccess))
	float HealingAmount; // �� ȸ����

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealingPotion", meta = (AllowPrivateAccess))
	float HealingDuration; // ȸ�� ���� �ð�

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealingPotion", meta = (AllowPrivateAccess))
	float AccumulatedHealing; // ���� ȸ����

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealingPotion", meta = (AllowPrivateAccess))
	float TotalElapsedTime; // Ÿ�̸� �� ��� �ð� 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealingPotion", meta = (AllowPrivateAccess))
	float ElapsedTime; // Ÿ�̸� ��� �ð� 0.5 �� ���� �ݺ�

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealingPotion", meta = (AllowPrivateAccess))
	float RemainingTime; // Ÿ�̸� ���� �ð�
};
