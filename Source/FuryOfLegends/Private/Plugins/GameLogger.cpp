// Fill out your copyright notice in the Description page of Project Settings.


#include "Plugins/GameLogger.h"
#include "GameFramework/Character.h"
#include <initializer_list>
#include <cstdarg>

GameLogger::GameLogger()
{
}

GameLogger::~GameLogger()
{
}


void GameLogger::Log(const FString& FunctionName, const ACharacter* Character, const TCHAR* Format, ...)
{
    // 가변 인자 목록 처리
    va_list Args;
    va_start(Args, Format);

    // 버퍼 크기 설정
    constexpr int32 BufferSize = 1024;
    TCHAR Buffer[BufferSize];

    // TCHAR 기반의 vsnprintf: FCString::GetVarArgs 사용
    FCString::GetVarArgs(Buffer, BufferSize, Format, Args);

    va_end(Args);

    FString FormattedMessage(Buffer);

    // 로그 메시지를 출력
    if (Character)
    {
        UE_LOG(LogTemp, Log, TEXT("[%s] [Character: %s] - %s"),
            *FunctionName, *Character->GetName(), *FormattedMessage);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("[%s] [Character: None] - %s"),
            *FunctionName, *FormattedMessage);
    }
}