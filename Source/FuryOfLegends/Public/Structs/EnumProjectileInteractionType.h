// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EProjectileInteractionType : uint8
{
    None,                // 기본적으로 모든 충돌을 무시
    PlayerOnly,          // 플레이어와만 충돌
    AIOnly,              // AI와만 충돌
    AllCharacters,       // 모든 캐릭터와 충돌
    BlockableBySkill,    // 스킬에 의해 막힐 수 있는 투사체
    Unblockable          // 어떤 스킬로도 막을 수 없는 투사체
};



/**
 * 
 */
class FURYOFLEGENDS_API EnumProjectileInteractionType
{
public:
	EnumProjectileInteractionType();
	~EnumProjectileInteractionType();
};
