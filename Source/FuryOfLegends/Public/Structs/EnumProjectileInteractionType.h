// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EProjectileInteractionType : uint8
{
    None,                // �⺻������ ��� �浹�� ����
    PlayerOnly,          // �÷��̾�͸� �浹
    AIOnly,              // AI�͸� �浹
    AllCharacters,       // ��� ĳ���Ϳ� �浹
    BlockableBySkill,    // ��ų�� ���� ���� �� �ִ� ����ü
    Unblockable          // � ��ų�ε� ���� �� ���� ����ü
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
