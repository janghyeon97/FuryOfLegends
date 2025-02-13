#include "Components/StatComponent.h"
#include "Characters/CharacterBase.h"
#include "Game/AOSGameInstance.h"
#include "Game/ArenaPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "Structs/CharacterStatData.h"


UStatComponent::UStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = false;

	CurrentLevel = 1;
	MaxLevel = 18;
}

void UStatComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UStatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	
}

void UStatComponent::InitializeComponent()
{
	Super::InitializeComponent();

}

void UStatComponent::InitStatComponent(UDataTable* InStatTable)
{
	if (InStatTable == nullptr)
	{
		return;
	}

	StatTable = InStatTable;

	FStatTableRow* StatRow = StatTable->FindRow<FStatTableRow>(FName(*FString::FromInt(1)), TEXT(""));
	if (StatRow)
	{
		// �⺻ ���� ����
		BaseMaxHP = StatRow->MaxHP;
		BaseMaxMP = StatRow->MaxMP;
		BaseHealthRegeneration = StatRow->HealthRegeneration;
		BaseManaRegeneration = StatRow->ManaRegeneration;
		BaseAttackDamage = StatRow->AttackDamage;
		BaseDefensePower = StatRow->DefensePower;
		BaseMagicResistance = StatRow->MagicResistance;
		BaseAttackSpeed = StatRow->AttackSpeed;
		BaseCriticalChance = StatRow->CriticalChance;
		BaseMovementSpeed = StatRow->MovementSpeed;

		// ���� ���� �ʱ�ȭ (�⺻ ���Ȱ� �����ϰ� ����)
		CurrentHP = BaseMaxHP;
		CurrentMP = BaseMaxMP;

		// EXP�� Level�� �⺻ ���� ���� �ʱ�ȭ
		MaxEXP = StatRow->MaxEXP;
		CurrentEXP = 0;
		MaxLevel = 18;
		CurrentLevel = 1;

		// ������ ���氪�� �ʱ�ȭ ���������� ��� 0�̾�� �մϴ�.
		AccumulatedFlatMaxHP = 0;
		AccumulatedFlatMaxMP = 0;
		AccumulatedFlatMaxEXP = 0;
		AccumulatedFlatHealthRegeneration = 0;
		AccumulatedFlatManaRegeneration = 0;
		AccumulatedFlatAttackDamage = 0;
		AccumulatedFlatDefensePower = 0;
		AccumulatedFlatMagicResistance = 0;
		AccumulatedFlatCriticalChance = 0;
		AccumulatedFlatAttackSpeed = 0;
		AccumulatedFlatMovementSpeed = 0;

		AccumulatedPercentAttackSpeed = 0;
		AccumulatedPercentMovementSpeed = 0;

		// ��� ������ �ʱ�ȭ�� ��, ���� ������ �����մϴ�.
		RecalculateStats();
	}
}

void UStatComponent::RecalculateStats()
{
	// ������ �÷� ���氪�� �⺻ ���ȿ� ���Ͽ� ���� ������ ���
	SetMaxHP(BaseMaxHP + AccumulatedFlatMaxHP);
	SetMaxMP(BaseMaxMP + AccumulatedFlatMaxMP);
	SetHealthRegeneration(BaseHealthRegeneration + AccumulatedFlatHealthRegeneration);
	SetManaRegeneration(BaseManaRegeneration + AccumulatedFlatManaRegeneration);
	SetAttackDamage(BaseAttackDamage + AccumulatedFlatAttackDamage);
	SetAbilityPower(BaseAbilityPower + AccumulatedFlatAbilityPower);
	SetDefensePower(BaseDefensePower + AccumulatedFlatDefensePower);
	SetMagicResistance(BaseMagicResistance + AccumulatedFlatMagicResistance);
	SetAbilityHaste(BaseAbilityHaste + AccumulatedFlatAbilityHaste);
	SetCriticalChance(BaseCriticalChance + AccumulatedFlatCriticalChance);

	// �ۼ�Ʈ�� �����ϴ� ���� ���
	float NewAttackSpeed = BaseAttackSpeed * (1.0f + AccumulatedPercentAttackSpeed / 100.0f) + AccumulatedFlatAttackSpeed;
	SetAttackSpeed(NewAttackSpeed);

	float NewMovementSpeed = BaseMovementSpeed * (1.0f + AccumulatedPercentMovementSpeed / 100.0f) + AccumulatedFlatMovementSpeed;
	SetMovementSpeed(NewMovementSpeed);

	// ���� HP�� MP�� �ִ밪�� �ʰ����� �ʵ��� ����
	SetCurrentHP(MaxHP);
	SetCurrentMP(MaxMP);
}


void UStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, MaxHP);
	DOREPLIFETIME(ThisClass, CurrentHP);
	DOREPLIFETIME(ThisClass, MaxMP);
	DOREPLIFETIME(ThisClass, CurrentMP);
	DOREPLIFETIME(ThisClass, MaxEXP);
	DOREPLIFETIME(ThisClass, CurrentEXP);
	DOREPLIFETIME(ThisClass, MaxLevel);
	DOREPLIFETIME(ThisClass, CurrentLevel);
	DOREPLIFETIME(ThisClass, HealthRegeneration);
	DOREPLIFETIME(ThisClass, ManaRegeneration);
	DOREPLIFETIME(ThisClass, AttackDamage);
	DOREPLIFETIME(ThisClass, AbilityPower);
	DOREPLIFETIME(ThisClass, DefensePower);
	DOREPLIFETIME(ThisClass, MagicResistance);
	DOREPLIFETIME(ThisClass, AttackSpeed);
	DOREPLIFETIME(ThisClass, AbilityHaste);
	DOREPLIFETIME(ThisClass, CriticalChance);
	DOREPLIFETIME(ThisClass, MovementSpeed);

	// Base stats
	DOREPLIFETIME(ThisClass, BaseMaxHP);
	DOREPLIFETIME(ThisClass, BaseMaxMP);
	DOREPLIFETIME(ThisClass, BaseCurrentMP);
	DOREPLIFETIME(ThisClass, BaseHealthRegeneration);
	DOREPLIFETIME(ThisClass, BaseManaRegeneration);
	DOREPLIFETIME(ThisClass, BaseAttackDamage);
	DOREPLIFETIME(ThisClass, BaseAbilityPower);
	DOREPLIFETIME(ThisClass, BaseDefensePower);
	DOREPLIFETIME(ThisClass, BaseMagicResistance);
	DOREPLIFETIME(ThisClass, BaseAttackSpeed);
	DOREPLIFETIME(ThisClass, BaseMovementSpeed);
	DOREPLIFETIME(ThisClass, BaseAbilityHaste);
	DOREPLIFETIME(ThisClass, BaseCriticalChance);
}

#pragma region Setter

void UStatComponent::SetMaxHP(float InMaxHP)
{
	float NewMaxHP = FMath::Clamp(InMaxHP, 0.0f, 99999.f);

	OnMaxHPChanged_NetMulticast(MaxHP, NewMaxHP);
	MaxHP = NewMaxHP;
}

void UStatComponent::SetCurrentHP(float InCurrentHP)
{
	float NewCurrentHP = FMath::Clamp<float>(InCurrentHP, 0, MaxHP);
	OnCurrentHPChanged_NetMulticast(CurrentHP, NewCurrentHP);
	CurrentHP = NewCurrentHP;

	if (CurrentHP < KINDA_SMALL_NUMBER)
	{
		OnOutOfCurrentHP_NetMulticast();
		CurrentHP = 0.f;
		return;
	}

	if (CurrentHP != MaxHP && OnHealthDepleted.IsBound())
	{
		OnHealthDepleted.Broadcast();
	}
	else if(CurrentHP == MaxHP && OnHealthFull.IsBound())
	{
		OnHealthFull.Broadcast();
	}
}

void UStatComponent::SetMaxMP(float InMaxMP)
{
	float NewMaxMP = FMath::Clamp(InMaxMP, 0.0f, 99999.f); // MaxMaxMP�� �ʿ� �� ����

	OnMaxMPChanged_NetMulticast(MaxMP, NewMaxMP);
	MaxMP = NewMaxMP;
}

void UStatComponent::SetCurrentMP(float InCurrentMP)
{
	float NewCurrentMP = FMath::Clamp<float>(InCurrentMP, 0, MaxMP);
	OnCurrentMPChanged_NetMulticast(CurrentMP, NewCurrentMP);
	CurrentMP = NewCurrentMP;

	if (CurrentMP != MaxMP && OnManaDepleted.IsBound())
	{
		OnManaDepleted.Broadcast();
	}
	else if(CurrentMP == MaxMP && OnManaFull.IsBound())
	{
		OnManaFull.Broadcast();
	}
}

void UStatComponent::SetMaxEXP(float InMaxEXP)
{
	float NewMaxEXP = FMath::Clamp<int32>(InMaxEXP, 0, 99999);
	OnMaxEXPChanged_NetMulticast(MaxEXP, NewMaxEXP);
	MaxEXP = NewMaxEXP;
}

void UStatComponent::SetCurrentEXP(float InCurrentEXP)
{
	if (CurrentLevel < MaxLevel)
	{
		if (InCurrentEXP >= MaxEXP)
		{
			// ������ ����
			float OverflowEXP = InCurrentEXP - MaxEXP;
			SetCurrentLevel(FMath::Clamp<int32>(GetCurrentLevel() + 1, 1, MaxLevel));
			SetCurrentEXP(OverflowEXP); // ���� ����ġ�� ����
		}
		else
		{
			CurrentEXP = FMath::Clamp<float>(InCurrentEXP, 0.f, MaxEXP);
			OnCurrentEXPChanged_NetMulticast(CurrentEXP, InCurrentEXP);
		}
	}
	else
	{
		// �ִ� ���� ���� ��, ����ġ�� �߰��� ������� ����
		CurrentEXP = MaxEXP;
		OnCurrentEXPChanged_NetMulticast(CurrentEXP, InCurrentEXP);
	}
}

void UStatComponent::SetCurrentLevel(int32 InCurrentLevel)
{
	if (!StatTable)
	{
		UE_LOG(LogTemp, Error, TEXT("SetCurrentLevel - StatTable is not valid."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Set CurrentLevel :: %d -> %d"), CurrentLevel, InCurrentLevel);

	int32 NewCurrentLevel = FMath::Clamp<int32>(InCurrentLevel, 1, MaxLevel);
	OnCurrentLevelChanged_NetMulticast(CurrentLevel, NewCurrentLevel);

	// �������� ���� ���� ������Ʈ
	FStatTableRow* NewLevelStatRow = StatTable->FindRow<FStatTableRow>(FName(*FString::FromInt(NewCurrentLevel)), TEXT(""));
	if (NewLevelStatRow)
	{
		const float OldMaxHP = BaseMaxHP;
		const float OldMaxMP = BaseMaxMP;
		

		// ���� ������ ���ο� ������ ������ ������Ʈ
		BaseMaxHP = NewLevelStatRow->MaxHP;
		BaseMaxMP = NewLevelStatRow->MaxMP;
		BaseHealthRegeneration = NewLevelStatRow->HealthRegeneration;
		BaseManaRegeneration = NewLevelStatRow->ManaRegeneration;
		BaseAttackDamage = NewLevelStatRow->AttackDamage;
		BaseDefensePower = NewLevelStatRow->DefensePower;
		BaseMagicResistance = NewLevelStatRow->MagicResistance;
		BaseAttackSpeed = NewLevelStatRow->AttackSpeed;
		BaseMovementSpeed = NewLevelStatRow->MovementSpeed;

		// ������Ʈ�� ���� ������ �����Ͽ� ���� ���� ���
		SetMaxHP(BaseMaxHP + AccumulatedFlatMaxHP);
		SetMaxMP(BaseMaxMP + AccumulatedFlatMaxMP);
		SetHealthRegeneration(BaseHealthRegeneration + AccumulatedFlatHealthRegeneration);
		SetManaRegeneration(BaseManaRegeneration + AccumulatedFlatManaRegeneration);
		SetAttackDamage(BaseAttackDamage + AccumulatedFlatAttackDamage);
		SetDefensePower(BaseDefensePower + AccumulatedFlatDefensePower);
		SetMagicResistance(BaseMagicResistance + AccumulatedFlatMagicResistance);
		SetAbilityHaste(BaseAbilityHaste + AccumulatedFlatAbilityHaste);
		SetCriticalChance(BaseCriticalChance + AccumulatedFlatCriticalChance);

		// �ۼ�Ʈ�� �����ϴ� ���� ��� (���� �ӵ��� �̵� �ӵ��� �ۼ�Ʈ ����)
		SetAttackSpeed(BaseAttackSpeed * (1.0f + AccumulatedPercentAttackSpeed / 100.0f) + AccumulatedFlatAttackSpeed);
		SetMovementSpeed(BaseMovementSpeed * (1.0f + AccumulatedPercentMovementSpeed / 100.0f) + AccumulatedFlatMovementSpeed);

		// ���� HP�� MP�� �� �ִ밪�� �°� ������Ʈ
		SetCurrentHP(CurrentHP + (BaseMaxHP- OldMaxHP));
		SetCurrentMP(CurrentMP + (BaseMaxMP - OldMaxMP));

		// ����ġ�� ���� ���� ������Ʈ
		SetMaxEXP(NewLevelStatRow->MaxEXP);
		SetCurrentEXP(CurrentEXP);
	}

	CurrentLevel = NewCurrentLevel;
}

void UStatComponent::SetHealthRegeneration(float InHealthRegeneration)
{
	float NewHealthRegeneration = FMath::Clamp<float>(InHealthRegeneration, 0, 9999.f);
	OnHealthRegenerationChanged_NetMulticast(HealthRegeneration, NewHealthRegeneration);
	HealthRegeneration = NewHealthRegeneration;
}

void UStatComponent::SetManaRegeneration(float InManaRegeneration)
{
	float NewManaRegeneration = FMath::Clamp<float>(InManaRegeneration, 0, 9999.f);
	OnManaRegenerationChanged_NetMulticast(ManaRegeneration, NewManaRegeneration);
	ManaRegeneration = NewManaRegeneration;
}

void UStatComponent::SetAttackDamage(float InAttackDamage)
{
	float NewAttackDamage = FMath::Clamp<float>(InAttackDamage, 0, 99999.f);
	OnAttackDamageChanged_NetMulticast(AttackDamage, NewAttackDamage);
	AttackDamage = NewAttackDamage;
}

void UStatComponent::SetAbilityPower(float InAbilityPower)
{
	float NewAbilityPower = FMath::Clamp<float>(InAbilityPower, 0, 99999.f);
	OnAbilityPowerChanged_NetMulticast(AbilityPower, NewAbilityPower);
	AbilityPower = NewAbilityPower;
}

void UStatComponent::SetDefensePower(float InDefensePower)
{
	float NewDefensePower = FMath::Clamp<float>(InDefensePower, 0, 9999.f);
	OnDefensePowerChanged_NetMulticast(DefensePower, NewDefensePower);
	DefensePower = NewDefensePower;
}

void UStatComponent::SetMagicResistance(float InMagicResistance)
{
	float NewMagicResistance = FMath::Clamp<float>(InMagicResistance, 0, 9999.f);
	OnMagicResistanceChanged_NetMulticast(MagicResistance, NewMagicResistance);
	MagicResistance = NewMagicResistance;
}

void UStatComponent::SetAbilityHaste(int32 InAbilityHaste)
{
	int32 NewAbilityHaste = FMath::Clamp<int32>(InAbilityHaste, 0, 300.f);
	OnAbilityHasteChanged_NetMulticast(AbilityHaste, NewAbilityHaste);
	AbilityHaste = NewAbilityHaste;
}

void UStatComponent::SetAttackSpeed(float InAttackSpeed)
{
	float NewAttackSpeed = FMath::Clamp<float>(InAttackSpeed, 0, 2.5f);
	OnAttackSpeedChanged_NetMulticast(AttackSpeed, NewAttackSpeed);
	AttackSpeed = NewAttackSpeed;
}

void UStatComponent::SetCriticalChance(int32 InCriticalChance)
{
	int32 NewCriticalChance = FMath::Clamp<int32>(InCriticalChance, 0, 100);
	OnCriticalChanceChanged_NetMulticast(CriticalChance, NewCriticalChance);
	CriticalChance = NewCriticalChance;
}

void UStatComponent::SetMovementSpeed(float InMovementSpeed)
{
	float NewMovementSpeed = FMath::Clamp<float>(InMovementSpeed, 0, 9999.f);
	OnMovementSpeedChanged_NetMulticast(MovementSpeed, NewMovementSpeed);
	MovementSpeed = NewMovementSpeed;
}


#pragma endregion

#pragma region Modifiers

void UStatComponent::ModifyCurrentHP(float Delta)
{
	SetCurrentHP(CurrentHP + Delta);
}

void UStatComponent::ModifyCurrentMP(float Delta)
{
	SetCurrentMP(CurrentMP + Delta);
}

void UStatComponent::ModifyCurrentEXP(float Delta)
{
	SetCurrentEXP(CurrentEXP + Delta);
}

void UStatComponent::ModifyAccumulatedPercentAttackSpeed(float Delta)
{
	AccumulatedPercentAttackSpeed += Delta;
	float NewAttackSpeed = BaseAttackSpeed * (1.0f + AccumulatedPercentAttackSpeed / 100.0f) + AccumulatedFlatAttackSpeed;
	SetAttackSpeed(NewAttackSpeed);
}

void UStatComponent::ModifyAccumulatedFlatAttackSpeed(float Delta)
{
	AccumulatedFlatAttackSpeed += Delta;
	float NewAttackSpeed = BaseAttackSpeed * (1.0f + AccumulatedPercentAttackSpeed / 100.0f) + AccumulatedFlatAttackSpeed;
	SetAttackSpeed(NewAttackSpeed);
}

/**
 * @brief �̵� �ӵ��� ���� �ۼ�Ʈ ��ȭ�� �����Ͽ� �̵� �ӵ��� �����մϴ�.
 *
 * �� �Լ��� �־��� Delta ���� ������ �ۼ�Ʈ ��ȭ�� �߰��ϰ�, �̸� �������
 * ĳ������ ���� �̵� �ӵ��� ����մϴ�. �ۼ�Ʈ ��ȭ�� �⺻ �̵� �ӵ��� ��������,
 * �߰����� ���� �̵� �ӵ�(AccumulatedFlatMovementSpeed)�� ����˴ϴ�.
 *
 * AccumulatedPercentMovementSpeed:
 * - �� ������ ĳ������ �̵� �ӵ��� ������ �ۼ�Ʈ ��ȭ�� �����մϴ�.
 * - ���� ���, -10%�� ��ȭ ȿ���� +20%�� �ӵ� ���� ȿ���� ����Ǹ�, 10%�� ���� ����˴ϴ�.
 * - �⺻ �̵� �ӵ�(BaseMovementSpeed)�� ���Ͽ� ���� �̵� �ӵ��� ����ϴ� �� ���˴ϴ�.
 *
 * @param Delta �̵� �ӵ��� ������ �ۼ�Ʈ ��ȭ�� (������ ��� ����, ����� ��� ����)
 */
void UStatComponent::ModifyAccumulatedPercentMovementSpeed(float Delta)
{
	AccumulatedPercentMovementSpeed += Delta;
	float NewMovementSpeed = BaseMovementSpeed * (1.0f + AccumulatedPercentMovementSpeed / 100.0f) + AccumulatedFlatMovementSpeed;
	SetMovementSpeed(NewMovementSpeed);
}

/**
 * @brief �̵� �ӵ��� ���� ���� �� ��ȭ�� �����Ͽ� �̵� �ӵ��� �����մϴ�.
 *
 * �� �Լ��� �־��� Delta ���� ������ ���� �̵� �ӵ��� �߰��ϰ�, �̸� �������
 * ĳ������ ���� �̵� �ӵ��� ����մϴ�. ���� �̵� �ӵ��� �⺻ �̵� �ӵ��� ��������,
 * �ۼ�Ʈ ��ȭ(AccumulatedPercentMovementSpeed)�� ����˴ϴ�.
 *
 * AccumulatedPercentMovementSpeed:
 * - �� ������ ĳ������ �̵� �ӵ��� ������ �ۼ�Ʈ ��ȭ�� �����մϴ�.
 * - �� �Լ������� ���������� �������� ������, ���� �̵� �ӵ� ��꿡 ���˴ϴ�.
 *
 * @param Delta �̵� �ӵ��� ������ ���� �� ��ȭ�� (������ ��� ����, ����� ��� ����)
 */
void UStatComponent::ModifyAccumulatedFlatMovementSpeed(float Delta)
{
	AccumulatedFlatMovementSpeed += Delta;
	float NewMovementSpeed = BaseMovementSpeed * (1.0f + AccumulatedPercentMovementSpeed / 100.0f) + AccumulatedFlatMovementSpeed;
	SetMovementSpeed(NewMovementSpeed);
}



void UStatComponent::ModifyAccumulatedFlatMaxHP(float Delta)
{
	AccumulatedFlatMaxHP += Delta;
	float NewMaxHP = BaseMaxHP + AccumulatedFlatMaxHP;
	SetMaxHP(NewMaxHP);
}

void UStatComponent::ModifyAccumulatedFlatMaxMP(float Delta)
{
	AccumulatedFlatMaxMP += Delta;
	float NewMaxMP = BaseMaxMP + AccumulatedFlatMaxMP;
	SetMaxMP(NewMaxMP);
}

void UStatComponent::ModifyAccumulatedFlatHealthRegeneration(float Delta)
{
	AccumulatedFlatHealthRegeneration += Delta;
	float NewHealthRegeneration = BaseHealthRegeneration + AccumulatedFlatHealthRegeneration;
	SetHealthRegeneration(NewHealthRegeneration);
}

void UStatComponent::ModifyAccumulatedFlatManaRegeneration(float Delta)
{
	AccumulatedFlatManaRegeneration += Delta;
	float NewManaRegeneration = BaseManaRegeneration + AccumulatedFlatManaRegeneration;
	SetManaRegeneration(NewManaRegeneration);
}

void UStatComponent::ModifyAccumulatedFlatAttackDamage(float Delta)
{
	AccumulatedFlatAttackDamage += Delta;
	float NewAttackDamage = BaseAttackDamage + AccumulatedFlatAttackDamage;
	SetAttackDamage(NewAttackDamage);
}

void UStatComponent::ModifyAccumulatedFlatAbilityPower(float Delta)
{
	AccumulatedFlatAbilityPower += Delta;
	float NewAbilityPower = BaseAbilityPower + AccumulatedFlatAbilityPower;
	SetAbilityPower(NewAbilityPower);
}

void UStatComponent::ModifyAccumulatedFlatDefensePower(float Delta)
{
	AccumulatedFlatDefensePower += Delta;
	float NewDefensePower = BaseDefensePower + AccumulatedFlatDefensePower;
	SetDefensePower(NewDefensePower);
}

void UStatComponent::ModifyAccumulatedFlatMagicResistance(float Delta)
{
	AccumulatedFlatMagicResistance += Delta;
	float NewMagicResistance = BaseMagicResistance + AccumulatedFlatMagicResistance;
	SetMagicResistance(NewMagicResistance);
}

void UStatComponent::ModifyAccumulatedFlatAbilityHaste(int32 Delta)
{
	AccumulatedFlatAbilityHaste += Delta;
	int32 NewAbilityHaste = BaseAbilityHaste + AccumulatedFlatAbilityHaste;
	SetAbilityHaste(NewAbilityHaste);
}

void UStatComponent::ModifyAccumulatedFlatCriticalChance(int32 Delta)
{
	AccumulatedFlatCriticalChance += Delta;
	int32 NewCriticalChance = BaseCriticalChance + AccumulatedFlatCriticalChance;
	SetCriticalChance(NewCriticalChance);
}

#pragma endregion


void UStatComponent::OnOutOfCurrentHP_NetMulticast_Implementation()
{
	if (OnOutOfCurrentHP.IsBound())
	{
		OnOutOfCurrentHP.Broadcast();
	}
}

void UStatComponent::OnMaxHPChanged_NetMulticast_Implementation(float InOldMaxHP, float InNewMaxHP)
{
	if (OnMaxHPChanged.IsBound())
	{
		OnMaxHPChanged.Broadcast(InOldMaxHP, InNewMaxHP);
	}
}

void UStatComponent::OnCurrentHPChanged_NetMulticast_Implementation(float InOldCurrentHP, float InNewCurrentHP)
{
	if (OnCurrentHPChanged.IsBound())
	{
		OnCurrentHPChanged.Broadcast(InOldCurrentHP, InNewCurrentHP);
	}
}

void UStatComponent::OnMaxMPChanged_NetMulticast_Implementation(float InOldMaxMP, float InNewMaxMP)
{
	if (OnMaxMPChanged.IsBound())
	{
		OnMaxMPChanged.Broadcast(InOldMaxMP, InNewMaxMP);
	}
}

void UStatComponent::OnCurrentMPChanged_NetMulticast_Implementation(float InOldCurrentMP, float InNewCurrentMP)
{
	if (OnCurrentMPChanged.IsBound())
	{
		OnCurrentMPChanged.Broadcast(InOldCurrentMP, InNewCurrentMP);
	}
}

void UStatComponent::OnMaxEXPChanged_NetMulticast_Implementation(float InOldMaxEXP, float InNewMaxEXP)
{
	if (OnMaxEXPChanged.IsBound())
	{
		OnMaxEXPChanged.Broadcast(InOldMaxEXP, InNewMaxEXP);
	}
}

void UStatComponent::OnCurrentEXPChanged_NetMulticast_Implementation(float InOldCurrentEXP, float InNewCurrentEXP)
{
	if (OnCurrentEXPChanged.IsBound())
	{
		OnCurrentEXPChanged.Broadcast(InOldCurrentEXP, InNewCurrentEXP);
	}
}

void UStatComponent::OnCurrentLevelChanged_NetMulticast_Implementation(int32 InOldCurrentLevel, int32 InNewCurrentLevel)
{
	if (OnCurrentLevelChanged.IsBound())
	{
		OnCurrentLevelChanged.Broadcast(InOldCurrentLevel, InNewCurrentLevel);
	}
}

void UStatComponent::OnHealthRegenerationChanged_NetMulticast_Implementation(float InOldHealthRegeneration, float InNewHealthRegeneration)
{
	if (OnHPRegenChanged.IsBound())
	{
		OnHPRegenChanged.Broadcast(InOldHealthRegeneration, InNewHealthRegeneration);
	}
}

void UStatComponent::OnManaRegenerationChanged_NetMulticast_Implementation(float InOldManaRegeneration, float InNewManaRegeneration)
{
	if (OnMPRegenChanged.IsBound())
	{
		OnMPRegenChanged.Broadcast(InOldManaRegeneration, InNewManaRegeneration);
	}
}

void UStatComponent::OnAttackDamageChanged_NetMulticast_Implementation(float InOldAttackDamage, float InNewAttackDamage)
{
	if (OnAttackDamageChanged.IsBound())
	{
		OnAttackDamageChanged.Broadcast(InOldAttackDamage, InNewAttackDamage);
	}
}

void UStatComponent::OnAbilityPowerChanged_NetMulticast_Implementation(float InOldAbilityPower, float InNewAbilityPower)
{
	if (OnAbilityPowerChanged.IsBound())
	{
		OnAbilityPowerChanged.Broadcast(InOldAbilityPower, InNewAbilityPower);
	}
}

void UStatComponent::OnDefensePowerChanged_NetMulticast_Implementation(float InOldDefensePower, float InNewDefensePower)
{
	if (OnDefensePowerChanged.IsBound())
	{
		OnDefensePowerChanged.Broadcast(InOldDefensePower, InNewDefensePower);
	}
}

void UStatComponent::OnMagicResistanceChanged_NetMulticast_Implementation(float InOldMagicResistance, float InNewMagicResistance)
{
	if (OnMagicResistanceChanged.IsBound())
	{
		OnMagicResistanceChanged.Broadcast(InOldMagicResistance, InNewMagicResistance);
	}
}

void UStatComponent::OnAttackSpeedChanged_NetMulticast_Implementation(float InOldAttackSpeed, float InNewAttackSpeed)
{
	if (OnAttackSpeedChanged.IsBound())
	{
		OnAttackSpeedChanged.Broadcast(InOldAttackSpeed, InNewAttackSpeed);
	}
}

void UStatComponent::OnAbilityHasteChanged_NetMulticast_Implementation(int32 InOldAbilityHaste, int32 InNewAbilityHaste)
{
	if (OnAbilityHasteChanged.IsBound())
	{
		OnAbilityHasteChanged.Broadcast(InOldAbilityHaste, InNewAbilityHaste);
	}
}

void UStatComponent::OnCriticalChanceChanged_NetMulticast_Implementation(int32 InOldCriticalChance, int32 InNewCriticalChance)
{
	if (OnCriticalChanceChanged.IsBound())
	{
		OnCriticalChanceChanged.Broadcast(InOldCriticalChance, InNewCriticalChance);
	}
}

void UStatComponent::OnMovementSpeedChanged_NetMulticast_Implementation(float InOldMovementSpeed, float InNewMovementSpeed)
{
	if (OnMovementSpeedChanged.IsBound())
	{
		OnMovementSpeedChanged.Broadcast(InOldMovementSpeed, InNewMovementSpeed);
	}
}

void UStatComponent::OnRep_CharacterStatReplicated()
{
	if (OnCharacterStatReplicated.IsBound())
	{
		OnCharacterStatReplicated.Broadcast();
	}
}