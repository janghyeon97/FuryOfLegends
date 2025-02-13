// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UserWidgetBase.h"
#include "UW_StateBar.generated.h"

/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UUW_StateBar : public UUserWidgetBase
{
	GENERATED_BODY()

public:
    UUW_StateBar(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;

    void SetMaxMP(float InMaxHP);

    void SetMaxHP(float InMaxHP);

    void InitializeStateBar(class UStatComponent* NewStatComponent);

    void InitializeMPBarWidget(class UStatComponent* NewStatComponent);

    void InitializeHPBarWidget(class UStatComponent* NewStatComponent);

    UFUNCTION()
    void OnMaxHPChanged(float InOldMaxHP, float InNewMaxHP);

    UFUNCTION()
    void OnCurrentHPChanged(float InOldHP, float InNewHP);

    UFUNCTION()
    void OnMaxMPChanged(float InOldMaxHP, float InNewMaxHP);

    UFUNCTION()
    void OnCurrentMPChanged(float InOldHP, float InNewHP);

    UFUNCTION()
    virtual void UpdateLevelText(int32 InOldLevel, int32 InNewLevel);

protected:
    UPROPERTY()
    TWeakObjectPtr<class UStatComponent> StatComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UW_StateBar", Meta = (BindWidget))
    TObjectPtr<class UTextBlock> LevelText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UW_StateBar", Meta = (BindWidget))
    TObjectPtr<class UTextBlock> PlayerNameText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UW_StateBar", Meta = (BindWidget))
    TObjectPtr<class UProgressBar> HPBar;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UW_StateBar", Meta = (BindWidget))
    TObjectPtr<class UProgressBar> MPBar;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UW_StateBar")
    float MaxHP;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UW_StateBar")
    float MaxMP;
};
