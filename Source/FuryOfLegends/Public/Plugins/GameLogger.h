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

    // 로그 메시지 출력 함수 (클래스 이름, 함수 이름, Character 이름을 포함)
	static void Log(const FString& FunctionName, const ACharacter* Character, const TCHAR* Format, ...);
};
