// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/MinionBase.h"
#include "RangedMinion.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API ARangedMinion : public AMinionBase
{
	GENERATED_BODY()
	
public:
	ARangedMinion();

protected:
	virtual void BeginPlay() override;

	virtual void MontageEnded(UAnimMontage* Montage, bool bInterrupted) override;
	virtual void LMB_Executed() override;
	virtual void LMB_CheckHit() override;
};
