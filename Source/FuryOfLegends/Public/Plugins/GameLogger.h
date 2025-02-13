// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


class ACharacter;

/**
 * 
 */
class FURYOFLEGENDS_API GameLogger
{
public:
	GameLogger();
	~GameLogger();

    // �α� �޽��� ��� �Լ� (Ŭ���� �̸�, �Լ� �̸�, Character �̸��� ����)
	static void Log(const FString& FunctionName, const ACharacter* Character, const TCHAR* Format, ...);
};
