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
    // ���� ���� ��� ó��
    va_list Args;
    va_start(Args, Format);

    // ���� ũ�� ����
    constexpr int32 BufferSize = 1024;
    TCHAR Buffer[BufferSize];

    // TCHAR ����� vsnprintf: FCString::GetVarArgs ���
    FCString::GetVarArgs(Buffer, BufferSize, Format, Args);

    va_end(Args);

    FString FormattedMessage(Buffer);

    // �α� �޽����� ���
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