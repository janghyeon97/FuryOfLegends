// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StatComponent.h"
#include "MinionStatComponent.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UMinionStatComponent : public UStatComponent
{
	GENERATED_BODY()
	
public:
    UMinionStatComponent();

    virtual void InitStatComponent(UDataTable* InStatTable) override;

private:
    void UpdateStatsBasedOnElapsedTime(const FName& RowName);
    void UpdateStatsBasedOnElapsedTime(const float InElapsedTime);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time", meta = (AllowPrivateAccess))
    float ElapsedTime;
};
