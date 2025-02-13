// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_StateBar.h"
#include "Components/StatComponent.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Characters/CharacterBase.h"

UUW_StateBar::UUW_StateBar(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UUW_StateBar::InitializeStateBar(UStatComponent* NewStatComponent)
{
    if (!::IsValid(NewStatComponent))
    {
        UE_LOG(LogTemp, Warning, TEXT("[UUW_StateBar::InitializeStateBar] Invalid StatComponent"));
        return;
    }

    StatComponent = NewStatComponent;

    if (StatComponent.IsValid())
    {
        StatComponent->OnMaxHPChanged.AddDynamic(this, &UUW_StateBar::OnMaxHPChanged);
        StatComponent->OnMaxMPChanged.AddDynamic(this, &UUW_StateBar::OnMaxMPChanged);
        StatComponent->OnCurrentHPChanged.AddDynamic(this, &UUW_StateBar::OnCurrentHPChanged);
        StatComponent->OnCurrentMPChanged.AddDynamic(this, &UUW_StateBar::OnCurrentMPChanged);
        StatComponent->OnCurrentLevelChanged.AddDynamic(this, &UUW_StateBar::UpdateLevelText);

        SetMaxHP(StatComponent->GetMaxHP());
        SetMaxMP(StatComponent->GetMaxMP());

        InitializeHPBarWidget(StatComponent.Get());
        InitializeMPBarWidget(StatComponent.Get());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[%s] Failed to bind StatComponent: StatComponent is invalid."), ANSI_TO_TCHAR(__FUNCTION__));
    }
}

void UUW_StateBar::InitializeHPBarWidget(UStatComponent* NewStatComponent)
{
    OnCurrentHPChanged(0, NewStatComponent->GetCurrentHP());
}

void UUW_StateBar::InitializeMPBarWidget(UStatComponent* NewStatComponent)
{
    OnCurrentMPChanged(0, NewStatComponent->GetCurrentMP());
}

void UUW_StateBar::SetMaxHP(float InMaxHP)
{
    if (InMaxHP < KINDA_SMALL_NUMBER)
    {
        MaxHP = 0.f;
        return;
    }

    MaxHP = InMaxHP;
}

void UUW_StateBar::SetMaxMP(float InMaxMP)
{
    if (InMaxMP < KINDA_SMALL_NUMBER)
    {
        MaxMP = 0;
        return;
    }

    MaxMP = InMaxMP;
}

void UUW_StateBar::OnMaxHPChanged(float InOldMaxHP, float InNewMaxHP)
{
    SetMaxHP(InNewMaxHP);
    OnCurrentHPChanged(0, InNewMaxHP);
}

void UUW_StateBar::OnMaxMPChanged(float InOldMaxMP, float InNewMaxMP)
{
    SetMaxMP(InNewMaxMP);
    OnCurrentMPChanged(0, InNewMaxMP);
}

void UUW_StateBar::OnCurrentHPChanged(float InOldHP, float InNewHP)
{
    if (::IsValid(HPBar))
    {
        if (KINDA_SMALL_NUMBER < MaxHP)
        {
            HPBar->SetPercent(InNewHP / MaxHP);
        }
        else
        {
            HPBar->SetPercent(0);
        }
    }
}

void UUW_StateBar::OnCurrentMPChanged(float InOldMP, float InNewMP)
{
    if (::IsValid(MPBar))
    {
        if (KINDA_SMALL_NUMBER < MaxMP)
        {
            MPBar->SetPercent(InNewMP / MaxMP);
        }
        else
        {
            MPBar->SetPercent(0);
        }
    }
}

void UUW_StateBar::UpdateLevelText(int32 InOldLevel, int32 InNewLevel)
{
	FString LevelString = FString::Printf(TEXT("%d"), InNewLevel);
	LevelText->SetText(FText::FromString(LevelString));
}

void UUW_StateBar::NativeConstruct()
{
	Super::NativeConstruct();

	ACharacterBase* OwningCharacter = Cast<ACharacterBase>(OwningActor);
	if (true == ::IsValid(OwningCharacter))
	{
		OwningCharacter->SetWidget(this);
	}
}
