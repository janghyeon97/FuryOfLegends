// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Structs/ActionData.h"
#include "CustomCombatData.generated.h"


UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EBaseCrowdControl : uint32
{
	None                    = 0x00,                // 아무 효과 없음
	MovementRestricted      = 0x01 << 0,           // 캐릭터의 이동을 막음
	InputDisabled           = 0x01 << 1,           // 플레이어 입력을 막음
	ActionUseRestricted_Q   = 0x01 << 2,           // Q 스킬 사용 제한
	ActionUseRestricted_E   = 0x01 << 3,           // E 스킬 사용 제한
	ActionUseRestricted_R   = 0x01 << 4,           // R 스킬 사용 제한
	ActionUseRestricted_LMB = 0x01 << 5,           // LMB 스킬 사용 제한
	ActionUseRestricted_RMB = 0x01 << 6,           // RMB 스킬 사용 제한
	VisionRestricted        = 0x01 << 7,           // 시야 제한
	RotationRestricted      = 0x01 << 8            // 방향 전환 제한
};
ENUM_CLASS_FLAGS(EBaseCrowdControl);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ECrowdControl : uint32
{
	None			= 0x00,
	Slow			= 0x01 << 0,    // 이동속도 둔화
	Cripple			= 0x01 << 1,    // 공격속도 둔화
	Silence			= 0x01 << 2,    // 침묵
	Blind			= 0x01 << 3,    // 실명
	BlockedSight	= 0x01 << 4,    // 시야 차단
	Snare			= 0x01 << 5,    // 속박
	Stun			= 0x01 << 6,    // 기절
	Taunt			= 0x01 << 7     // 도발
};
ENUM_CLASS_FLAGS(ECrowdControl);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EAttackTrigger : uint32
{
	None			= 0x00 UMETA(Hidden),
	OnHit			= 0x01 << 0,  // 기본 공격이 명중하면 발동
	OnAttack		= 0x01 << 1,  // 공격 시 발동 (OnHit보다 넓은 범위)
	AbilityEffects	= 0x01 << 2   // 스킬이 명중하면 발동
};
ENUM_CLASS_FLAGS(EAttackTrigger);


UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EAbilityEffect : uint32
{
	None            = 0x00 UMETA(Hidden),
	Stun            = 0x01 << 0 UMETA(DisplayName = "Stun"),
	Slow            = 0x01 << 1 UMETA(DisplayName = "Slow"),
	Burn            = 0x01 << 2 UMETA(DisplayName = "Burn"),
	Lifesteal       = 0x01 << 3 UMETA(DisplayName = "Lifesteal"),
	Cripple         = 0x01 << 4 UMETA(DisplayName = "Cripple"),
	Silence         = 0x01 << 5 UMETA(DisplayName = "Silence"),
	Blind           = 0x01 << 6 UMETA(DisplayName = "Blind"),
	BlockedSight    = 0x01 << 7 UMETA(DisplayName = "BlockedSight"),
	Snare           = 0x01 << 8 UMETA(DisplayName = "Snare"),
	Taunt           = 0x01 << 9 UMETA(DisplayName = "Taunt")
};
ENUM_CLASS_FLAGS(EAbilityEffect);

UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EDamageType : uint8
{
	None        = 0x00 UMETA(Hidden),
	Physical    = 0x01 << 0 UMETA(DisplayName = "Physical"),
	Magic       = 0x01 << 1 UMETA(DisplayName = "Magic"),
	TrueDamage  = 0x01 << 2 UMETA(DisplayName = "TrueDamage"),
	Critical    = 0x01 << 3 UMETA(DisplayName = "Critical")
};
ENUM_CLASS_FLAGS(EDamageType);

USTRUCT(BlueprintType)
struct FCrowdControlInformation
{
	GENERATED_BODY()

public:
	FCrowdControlInformation() : Type(ECrowdControl::None), Duration(0), Percent(0) {}
	FCrowdControlInformation(ECrowdControl InCrowdControl, float InDuration, float InPercent = 1.0f)
		: Type(InCrowdControl)
		, Duration(InDuration)
		, Percent(InPercent)
	{}


public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECrowdControl Type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-100.0", ClampMax = "100.0", uIMin = "-100.0", uIMax = "100.0"))
	float Percent;
};

USTRUCT(BlueprintType)
struct FDamageInformation
{
	GENERATED_BODY()

public:
	FDamageInformation()
		: ActionSlot(EActionSlot::None)
		, PhysicalDamage(0.0f)
		, MagicDamage(0.0f)
		, TrueDamage(0.0f)
		, DamageType(EDamageType::None)
		, AttackTrigger(EAttackTrigger::None)
	{
		CrowdControls.Empty();
	}

	// Action Slot 설정 함수
	void SetActionSlot(EActionSlot InActionSlot)
	{
		ActionSlot = InActionSlot;
	}

	// 물리 피해 추가 함수
	void AddPhysicalDamage(float Amount)
	{
		if (Amount > 0.0f)
		{
			PhysicalDamage += Amount;
			EnumAddFlags(DamageType, EDamageType::Physical);
		}
	}

	// 마법 피해 추가 함수
	void AddMagicDamage(float Amount)
	{
		if (Amount > 0.0f)
		{
			MagicDamage += Amount;
			EnumAddFlags(DamageType, EDamageType::Magic);
		}
	}

	// 고정 피해 추가 함수
	void AddTrueDamage(float Amount)
	{
		if (Amount > 0.0f)
		{
			TrueDamage += Amount;
			EnumAddFlags(DamageType, EDamageType::TrueDamage);
		}
	}

	// 피해량을 DamageType에 따라 추가하는 함수 (기존 함수 유지)
	void AddDamage(EDamageType InDamageType, float DamageAmount)
	{
		if (DamageAmount <= 0.0f)
		{
			return;
		}

		switch (InDamageType)
		{
		case EDamageType::Physical:
			AddPhysicalDamage(DamageAmount);
			break;
		case EDamageType::Magic:
			AddMagicDamage(DamageAmount);
			break;
		case EDamageType::TrueDamage:
			AddTrueDamage(DamageAmount);
			break;
		case EDamageType::Critical:
			EnumAddFlags(DamageType, EDamageType::Critical);
			AddPhysicalDamage(DamageAmount); // 치명타는 물리 피해로 처리
			break;
		default:
			break;
		}
	}

	// 공격 트리거 추가 함수 
	void AddTrigger(EAttackTrigger InTrigger)
	{
		EnumAddFlags(AttackTrigger, InTrigger);
	}

	// 군중 제어 효과 추가 함수
	void AddCrowdControl(const FCrowdControlInformation& CCInfo)
	{
		CrowdControls.Add(CCInfo);
	}

	void ClearDamage()
	{
		PhysicalDamage = 0.f;
		MagicDamage = 0.f;
		TrueDamage = 0.f;

		DamageType = EDamageType::None;
	}

	// 군중 제어 효과 전체 초기화 함수
	void ClearCrowdControl()
	{
		CrowdControls.Empty();
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	EActionSlot ActionSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	float PhysicalDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	float MagicDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	float TrueDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	EDamageType DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	EAttackTrigger AttackTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	TArray<FCrowdControlInformation> CrowdControls;
};



/**
 * 
 */
UCLASS()
class FURYOFLEGENDS_API UCustomCombatData : public UObject
{
	GENERATED_BODY()
	
};
