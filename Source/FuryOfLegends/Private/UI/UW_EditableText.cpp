// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_EditableText.h"
#include "Components/EditableText.h"

void UUW_EditableText::NativeConstruct()
{

}

FString UUW_EditableText::GetText() const
{
	return EditText->GetText().ToString();
}